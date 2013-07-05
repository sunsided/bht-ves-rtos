/*****************************************************************************
*                                                                            
* Beispiel für einen minimalen Multithreading-Betrieb (MT-Betrieb) 
* auf einem Prozessor der 8051-Familie.
*                                                                            
* Markus Mayer   (Matr-Nr. XXXXXX)			                                 
* Patrick Kaiser (Matr-Nr. YYYYYY)			                                 
*
* Implementierung von user- und kernel space-Funktionen für die
* Registrierung und Verarbeitung von Semaphoren.
*                                                                            
*****************************************************************************/

#include <assert.h>
#include "system.h"
#include "timer.h"
#include "threads.h"
#include "systemcall.h"
#include "semaphores.h"

/**
* Liste der Semaphore
*/
static sem_list_item_t semaphore_list[MAX_SEMAPHORE_COUNT];

/**
* Anzahl der Semaphore
*/
static uint8_t semaphore_count = 0;

/**
* Initialisiert die Semaphorliste
*/
void os_initialize_semaphore_list(void)
{
	static sem_id_t sem_idx;
	
	semaphore_count = 0;
	for (sem_idx = 0; sem_idx < MAX_SEMAPHORE_COUNT; ++sem_idx)
	{
		semaphore_list[sem_idx].value = 0;
		semaphore_list[sem_idx].list_head = NIL;
	}
}

/**
* Initialisiert einen Semaphor
*
* @param semaphore	Der zu initialisierende Semaphor
* @param sem_size		Die initiale Größe des Semaphors
* @returns 		  		0 im Erfolgsfall, ansonsten Fehlercode.
*/
sem_error_t os_semaphore_init(semaphore_t* semaphore, const sem_size_t sem_size)
{
	static system_call_t							*sc;
	static syscall_init_semaphore_t	*calldata;
	static system_call_result_t			*sr;
	
	if (0 == semaphore) return SEM_INVALID_SEMAPHORE;
	
	// Debugging: Auf ungültigen Wert initialisieren
	semaphore->semaphore_id = NIL;
	
	// System call beginnen
	sc = os_begin_system_call(SEMAPHORE_INIT);
	assert(SEMAPHORE_INIT == sc->type);
	
	// Wenn keine Semaphore mehr registrierbar sind,
	// System call abbrechen (atomaren Bereich verlassen)
	if (MAX_SEMAPHORE_COUNT == semaphore_count)
	{
		os_cancel_execute_system_call();
		return SEM_TOO_MANY_SEMAPHORES;
	}

	// System call vorbereiten und absenden
	calldata = &sc->call_data.semaphore_init;
	calldata->initial_size = sem_size;
	
	os_execute_system_call();

	// Semphor-ID beziehen
	sr = os_get_system_call_result();
	semaphore->semaphore_id = sr->result_data.semaphore.semaphore_id;
	os_clear_system_call_result();

	return SEM_SUCCESS;
}

/**
* Gibt den Semaphor frei.
*
* @param semaphore	Der Semaphor
*/
sem_error_t os_semaphore_post(const semaphore_t* semaphore)
{
	static system_call_t								*sc;
	static syscall_modify_semaphore_t	*calldata;
	static sem_id_t										id;
	
	if (0 == semaphore)
	{
		return SEM_INVALID_SEMAPHORE;
	}
	id = semaphore->semaphore_id;
	
	// System call initiieren
	sc = os_begin_system_call(SEMAPHORE_POST);
	assert(SEMAPHORE_POST == sc->type);
	
	// Wenn keine Semaphore registriert sind, stimmt
	// etwas richtig nicht.
	// System call abbrechen (atomaren Bereich verlassen)
	if (0 == semaphore_count)
	{
		os_cancel_execute_system_call();
		return SEM_INVALID_STATE;
	}
	
	// Notwendigkeit des system calls überprüfen:
	// Wenn kein blockierender Thread, erhöhen.
	if (NIL == semaphore_list[id].list_head)
	{
		// Semaphor erhöhen und Operation abbrechen
		if (semaphore_list[id].value < 0xFF)
		{
			++semaphore_list[id].value;
			assert(0x0 < semaphore_list[id].value);
		}
		
		os_cancel_execute_system_call();
		return SEM_SUCCESS;
	}
	
	// Affentest
	assert(NIL != semaphore_list[id].list_head);
	
	// Mindestens ein Thread wartet auf den Semaphor.
	// system call ausführen
	calldata = &sc->call_data.semaphore_modification;
	calldata->semaphore_id = id;
	os_execute_system_call();

	// NOTE: An dieser Stelle können alle static-Variablen überschrieben worden sein.
	
	/*
	// sanity check
	sr = os_get_system_call_result();
	assert(semaphore->semaphore_id == sr->result_data.semaphore.semaphore_id);
	*/
	os_clear_system_call_result();
	
	return SEM_SUCCESS;
}

/**
* Blockiert auf den Semaphor.
*
* @param semaphore	Der Semaphor
*/
sem_error_t os_semaphore_wait(const semaphore_t* semaphore)
{
	static system_call_t							*sc;
	static syscall_modify_semaphore_t	*calldata;
	static sem_id_t										id;
	
	if (0x0 == semaphore)
	{
		return SEM_INVALID_SEMAPHORE;
	}
	id = semaphore->semaphore_id;
	
	// System call initiieren
	sc = os_begin_system_call(SEMAPHORE_WAIT);
	assert(SEMAPHORE_WAIT == sc->type);
	
	// Wenn keine Semaphore registriert sind, stimmt
	// etwas richtig nicht.
	// System call abbrechen (atomaren Bereich verlassen)
	if (0 == semaphore_count)
	{
		os_cancel_execute_system_call();
		return SEM_INVALID_STATE;
	}
	
	// Notwendigkeit des system calls überprüfen:
	// Wenn Semaphor-Wert größer als null, ist kein
	// anderer Thread blockiert - ein call ist unnötig.
	if (semaphore_list[id].value > 0)
	{
		// Semaphor verringern und Operation abbrechen
		--semaphore_list[id].value;
		assert(0xFF > semaphore_list[id].value);
		
		os_cancel_execute_system_call();
		return SEM_SUCCESS;
	}
	
	// Semaphor war leer - Blockierung anfordern.
	// system call ausführen
	calldata = &sc->call_data.semaphore_modification;
	calldata->semaphore_id = id;
	os_execute_system_call();

	// NOTE: An dieser Stelle können alle static-Variablen überschrieben worden sein.
	
	// NOTE: Es scheint, dass der Funktionsparameter verändert wird!
	/*
	// sanity check
	sr = os_get_system_call_result();
	assert(semaphore->semaphore_id == sr->result_data.semaphore.semaphore_id);
	*/
	os_clear_system_call_result();
	
	return SEM_SUCCESS;
}

/**
* Führt den system call SEMAPHORE_INIT aus.
*
* @param syscall Die system call-Instanz.
*/
void kernel_exec_syscall_sem_init(const system_call_t *syscall) using 1
{
	static syscall_init_semaphore_t		*sc;
	static syscall_semaphore_result_t	*sr;
	static sem_id_t 									semaphore_id;
	static sem_list_item_t 						*sem_list_item;
	
	// system call und Ergebnis-Instanz beziehen
	sc = (syscall_init_semaphore_t *)&syscall->call_data;
	
	// Sicherstellen, dass noch nicht alle Semaphore vergeben sind.
	// Überprüfung wird bereits im user space ausgeführt.
	assert (MAX_SEMAPHORE_COUNT != semaphore_count);
	
	// Thread ID ermitteln und 
	semaphore_id = (threadno_t)semaphore_count++; // NOTE: Logik nimmt an, dass niemals Semaphore entfernt werden.
	
	// Control Block-Listenitem beziehen und initialisieren
	sem_list_item = &semaphore_list[semaphore_id];
	sem_list_item->list_head = NIL;
	sem_list_item->value = sc->initial_size;
	
	// Ergebnis des system calls speichern
	sr = &kernel_prepare_system_call_result()->result_data.semaphore;
	sr->semaphore_id = semaphore_id;	
}

/**
* Fügt einen Thread in die Liste eines Semaphors ein.
*
* @param semaphor_id 	Die ID des Semaphors
* @param threaD_id		Die ID des Threads
*/
static void kernel_add_to_semaphore_list(const sem_id_t semaphore_id, const uint8_t thread_id) using 1
{
	static uint8_t					token_id;
	
	// Zeiger extrahieren, um array-lookups zu reduzieren
	static tcb_list_item_t *token;
	static tcb_list_item_t *prev;
	static tcb_list_item_t *new_item;
	
	new_item = &tcb_list[thread_id];
		
	// Sonderfall: Es handelt sich um den ersten Thread
	if (NIL == semaphore_list[semaphore_id].list_head) 
	{	
		semaphore_list[semaphore_id].list_head = thread_id;
		new_item->next = NIL;
	}
	else // (NIL == semaphore_list[semaphore_id].list_head) 
	{
		token_id = semaphore_list[semaphore_id].list_head;
		prev = NULL;
		
		while(NIL != token_id)
		{
			token = &tcb_list[token_id];
			
			// Wenn das neue item höherer Priorität als das bestehende item ist,
			// soll der neue Eintrag vor dem bestehenden Eintrag einsortiert werden.
			if (new_item->tcb.priority > token->tcb.priority)
			{
				new_item->next = token_id;
				
				// Wenn ein Vorgänger existiert, wird dessen next-Zeiger
				// auf den neuen Thread gesetzt.
				if (NULL != prev)
				{
					prev->next = thread_id;
				}
				else
				{
					// Da kein Vorgänger existiert, sind wir am Beginn der
					// Liste und müssen somit den head-Zeiger auf uns stellen.
					semaphore_list[semaphore_id].list_head = thread_id;
				}
				break;
			}
			else // if (new_item->priority > token->priority)
			{
				// Existiert ein weiteres Element in der Liste,
				// wird der Zeiger auf dieses gesetzt und die Operation wiederholt.
				// Der prev-Zeiger wird auf das aktuelle token gesetzt.
				if (NIL != token->next)
				{
					prev = token;
					token_id = token->next;
					continue;
				}

				// An dieser Stelle angekommen, haben wir das Ende der
				// Liste erreicht, weswegen wir uns als letztes
				// Element eintragen.
				if (NULL != prev)
				{
					prev->next = thread_id;
				}
				else
				{
					token->next = thread_id;
				}
				new_item->next = NIL;
				break;
			}
		}
		assert(NIL != token_id);
	}
}

/**
* Fügt einen Thread in die Liste eines Semaphors ein.
*
* @param semaphor_id 	Die ID des Semaphors
* @param threaD_id		Die ID des Threads
*/
static uint8_t kernel_remove_from_semaphore_list(sem_id_t semaphore_id) using 1
{
	static uint8_t thread_id;
	
	assert(NIL != semaphore_list[semaphore_id].list_head);
	
	// Zeiger vom Beginn der Semaphor-Liste besorgen
	thread_id = semaphore_list[semaphore_id].list_head;
	
	// Semaphor-Zeiger auf Nachfolger setzen
	semaphore_list[semaphore_id].list_head = tcb_list[thread_id].next;
	
	return thread_id;
}

/**
* Führt den system call SEMAPHORE_WAIT aus.
*
* @param syscall Die system call-Instanz.
*/
void kernel_exec_syscall_sem_wait(const system_call_t *syscall) using 1
{
	static int8_t 		thread_id;
	static sem_id_t 	semaphore_id;

	static syscall_modify_semaphore_t	*sc;
	static syscall_semaphore_result_t	*sr;
	
	// Semaphor-ID aus syscall beziehen
	sc = (syscall_modify_semaphore_t *)&syscall->call_data;
	semaphore_id = sc->semaphore_id;
	
	// Ergebnis des system calls speichern
	sr = &kernel_prepare_system_call_result()->result_data.semaphore;
	sr->semaphore_id = semaphore_id;
	
	// Aktuellen Thread von ready-Liste entfernen
	thread_id = kernel_get_current_thread_id();
	kernel_remove_from_ready_list(thread_id);
	
	// Thread auf Liste des Semaphors setzen
	kernel_add_to_semaphore_list(semaphore_id, thread_id);
}

/**
* Führt den system call SEMAPHORE_POST aus.
*
* @param syscall Die system call-Instanz.
*/
void kernel_exec_syscall_sem_post(const system_call_t *syscall) using 1
{
	static int8_t 		thread_id;
	static sem_id_t 	semaphore_id;
	
	static syscall_modify_semaphore_t	*sc;
	static syscall_semaphore_result_t	*sr;
	
	// Semaphor-ID aus syscall beziehen
	sc = (syscall_modify_semaphore_t *)&syscall->call_data;
	semaphore_id = sc->semaphore_id;
	
	// Ergebnis des system calls speichern
	sr = &kernel_prepare_system_call_result()->result_data.semaphore;
	sr->semaphore_id = semaphore_id;
	
	// Ersten Thread von Liste des Semaphors entfernen
	thread_id = kernel_remove_from_semaphore_list(semaphore_id);
	
	// Thread auf ready-Liste setzen
	if (NIL != thread_id)
	{
		kernel_add_to_ready_list(thread_id);
	}
}