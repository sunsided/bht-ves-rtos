/*****************************************************************************
*                                                                            *
*      Implementierung des Echtzeitkernels                                   *
*                                                                            *
*****************************************************************************/

// NOTE: Verwendung von assert aus mehreren Registerbanken erzeugt WARNING L15

#include <reg51.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "v24.h"
#include "timer.h"
#include "systemcall.h"
#include "threads.h"

#include "../rtos/rtos.h"

/**
* Hilfsdefinition für while(ALWAYS) {}
*/
#define ALWAYS 		  (1)

/**
* Position der Registerbank 0 im internen RAM
*/
#define POSRB0      (0x00)

/**
* Bezieht das MSB eines Wortes
*/
#define HIGH_BYTE_FROM_PTR(ptr) (((uint16_t)(ptr) & 0xFF00U) >> 8)

/**
* Bezieht das LSB eines Wortes
*/
#define LOW_BYTE_FROM_PTR(ptr)   ((uint16_t)(ptr) & 0x00FFU)

/**
* Stacks der Threads
*/
uint8_t idata Stack[MAX_THREADS][MAX_THREAD_STACKLENGTH] _at_ 0x30;   

/**
* Liste der Thread Control Blocks
*/
tcb_list_item_t xdata tcb_list[MAX_THREADS];

/**
* Not in list.
*
* Markiert das Ende einer Liste.
*/
#define NIL (0xFF)

/**
* Invalid
*
* Markiert einen ungültigen Zeiger.
*/
#define INV (0xFE)

/**
* Kopf der Ready-Liste
*
* Kopf der Liste der rechenwilligen Threads.
*/
static uint8_t tcb_list_ready_head = NIL;

/**
* Kopf der Sleep-Liste
*
* Kopf der Liste der schlafenden Threads.
*/
static uint8_t tcb_list_sleep_head = NIL;

/**
* Anzahl der registrierten Threads
*/
uint8_t thread_count = 0;

/**
* Index des aktuellen Threads
*/
int8_t current_thread_id = INV;

/**
* Gibt an, ob das OS initialisiert wurde
*/
static bool os_initialized = false;

/**
* Gibt an, ob das OS läuft wurde
*/
static bool os_running = false;

/**
* Der idle thread
*/
static void idle_thread(void)
{
	while(1) {
	}
}

/**
* Startet das Betriebssystem.
*
* @returns Diese Methode wird niemals verlassen.
*/
void os_start(void)
{
	assert(true  == os_initialized);
	assert(false == os_running);

	os_start_system_timer();
	
	os_running = true;
	while (ALWAYS);
}

/**
* Initialisiert das Betriebssystem.
*/
void os_init(void)
{
	threadno_t idle_thread_no;
	uint8_t tcb_idx;
	
	assert(false == os_running);
	
	for (tcb_idx = 0; tcb_idx < MAX_THREADS; ++tcb_idx)
	{
		tcb_list[tcb_idx].next = INV;
	}
	
	os_intialize_uart();
	os_initialize_system_timer();
	
	idle_thread_no = os_register_thread(idle_thread, PRIO_RESERVED_IDLE, "Idle Thread");
	assert(0 == idle_thread_no);

	os_initialized = true;
}

/**
* strncpy-Implementierung im kernel space.
*/
static void kernel_strncpy(unsigned char *dst, const unsigned char *src, const uint8_t length) using 1
{
	static uint8_t index;
	assert(NULL != dst);
	assert(NULL != src);
	
	index = 0;
	while (0 != src[index])
	{
		dst[index] = src[index];
		++index;
	};
	
	while (index < length) 
	{
		dst[index] = 0;
		++index;
	}
}

/**
* Fügt einen Thread zur ready-Liste hinzu.
*
* @param thread_id Die ID des einzusortierenden Threads.
*/
static void kernel_add_to_ready_list(const uint8_t thread_id) using 1
{
	static uint8_t					token_id;
	
	// Zeiger extrahieren, um array-lookups zu reduzieren
	static tcb_list_item_t *token;
	static tcb_list_item_t *prev;
	static tcb_list_item_t *new_item;
	
	new_item = &tcb_list[thread_id];
	
	// Sonderfall: Es handelt sich um den ersten Thread
	if (NIL == tcb_list_ready_head) 
	{	
		tcb_list_ready_head = thread_id;
		new_item->next = NIL;
	}
	else // (NIL == tcb_list_ready_head) 
	{
		token_id = tcb_list_ready_head;
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
					tcb_list_ready_head = thread_id;
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

				// An dieser Stelle angekommen, ist die Priorität des neuen
				// Threads geringer als die des idle-Threads, was einen
				// Fehler darstellt.
				assert(NULL);
				break;
			}
		}
		assert(NIL != token_id);
	}
}

/**
* Entfernt einen thread aus der ready-Liste.
*
* @param thread_id Die id des zu entfernenden Threads.
*/
static void kernel_remove_from_ready_list(const uint8_t thread_id) using 1
{
	static uint8_t token_id;
	
	// wenn keine rechenwilligen Threads registriert sind - abbrechen.
	if (NIL == tcb_list_ready_head) {
		return;
	}
	
	// ist der zu entfernende Thread am Kopf der Liste, 
	// Liste auf den next-Eintrag ändern.
	if (thread_id == tcb_list_ready_head) 
	{
		tcb_list_ready_head = tcb_list[thread_id].next;
		tcb_list[thread_id].next = NIL;
		return;
	}
	
	// Liste durchlaufen, bis Vorgänger von thread_id gefunden
	token_id = tcb_list_ready_head;
	while (NIL != token_id)
	{
		// Wenn der Vorgänger gefunden wurde, den next-Zeiger
		// auf den Nachfolger des aktuellen Threads setzen.
		if (thread_id == tcb_list[token_id].next) 
		{
			tcb_list[token_id].next = tcb_list[thread_id].next;
			tcb_list[thread_id].next = NIL;
			break;
		}
		
		// Token aufd den Nachfolger setzen
		token_id = tcb_list[token_id].next;
	}
}

/**
* Fügt einen Thread zur sleep-Liste hinzu.
*
* Es wird davon ausgegangen, dass die Schlafzeit bereits im TCB hinterlegt ist.
* Die Zeit des ersten Elementes der sleep list ist absolut gespeichert, alle
* weiteren Einträge speichern die Differenz zum Vorgängerwert, so dass die Summe
* des Eintrags und aller Vorgänger gleich der gewünschten Schlafzeit ist.
*
* Beispiel:
* Die Schalfzeiten 7s, 5s und 10s sollen hinterlegt werden. Die Liste enthält
* folglich die Einträge: 5s -> 2s -> 3s (5+2=7, 5+2+3=10)
*
* Ein Eintrag auf der sleep list kann nicht gleichzeitig Teil der ready list sein.
*
* @param thread_id Die ID des einzusortierenden Threads.
*/
static void kernel_add_to_sleep_list(const uint8_t thread_id) using 1
{
	static sleep_t thread_sleep_time;
	static sleep_t integrated_sleep_time;
	static uint8_t token_id;
	static uint8_t prev_id;
	
	// Wenn noch keine Einträge existieren, kann der Wert 
	// direkt übernommen (d.h. beibehalten) werden.
	// Der Kopf der Liste wird auf den thread gesetzt.
	if (NIL == tcb_list_sleep_head)
	{
		tcb_list_sleep_head = thread_id;
		tcb_list[thread_id].next = NIL;
		return;
	}
	
	// Da die Liste bereits Einträge beinhaltet, muss
	// der korrekte Einfügepunkt ermittelt werden, wozu
	// die Schlafzeiten integriert werden.
	token_id = tcb_list_sleep_head;
	prev_id = NIL;
	
	integrated_sleep_time = tcb_list[token_id].tcb.sleep_duration;
	thread_sleep_time = tcb_list[thread_id].tcb.sleep_duration;
	
	while (1)
	{
		// Ist die (reduzierte) Schlafzeit des Threads geringer als die
		// integrierte Schlafzeit, wird der Thread vor dem aktuellen Element
		// eingereiht und die nachfolgenden Werte reduziert.
		if (NIL == token_id || thread_sleep_time < integrated_sleep_time) 
		{
			tcb_list[thread_id].next = token_id;
			tcb_list[thread_id].tcb.sleep_duration = thread_sleep_time;
			
			// Wenn ein Vorgängerelement existiert, dieses anpassen
			if (NIL != prev_id) 
			{
				tcb_list[prev_id].next = thread_id;
			}
			else
			{
				// Da kein Vorgänger existiert, muss der Listenkopf gesetzt werden.
				tcb_list_sleep_head = thread_id;
			}
			
			return;
		}
		
		// Die Schlafzeit des Threads ist größer als die integrierte
		// Schlafzeit, daher Thread-Zeit verringern und nächstes Element anwählen.
		thread_sleep_time -= tcb_list[token_id].tcb.sleep_duration;
		token_id = tcb_list[token_id].next;
	}
}

/**
* Führt den system call REGISTER_THREAD aus.
*
* @param syscall Die system call-Instanz.
*/
static void kernel_exec_syscall_register_thread(const system_call_t *syscall) using 1
{
	static syscall_register_thread_t *sc;
	static syscall_register_thread_result_t *sr;
	static threadno_t thread_id;
	static tcb_t *tcb;
	static tcb_list_item_t *tcb_list_item;
	
	// system call und Ergebnis-Instanz beziehen
	sc = (syscall_register_thread_t *)&syscall->call_data;
	
	// Sicherstellen, dass noch nicht alle Threads vergeben sind.
	// Überprüfung wird bereits im user space ausgeführt.
	assert (MAX_THREADS != thread_count);
	
	// Thread ID ermitteln und 
	thread_id = (threadno_t)thread_count++; // NOTE: Logik nimmt an, dass niemals Threads entfernt werden.
	
	// Control Block-Listenitem beziehen und initialisieren
	tcb_list_item = &tcb_list[thread_id];
	tcb_list_item->next = INV;
	
	// Control Block beziehen und Werte setzen
	tcb = &tcb_list_item->tcb;
	tcb->sleep_duration = 0;
	tcb->priority = sc->priority;
	kernel_strncpy(tcb->thread_data.name, sc->name, MAX_THREAD_NAME_LENGTH);
	
	// SP erstmals auf die nachfolgend abgelegte Rücksprungadresse + 5 byte für 5 PUSHes
	tcb->sp  = (unsigned char)(&Stack[thread_id][0] + 6);

	// Startadresse des registrierten Threads als Rücksprungadresse sichern
	Stack[thread_id][0] = LOW_BYTE_FROM_PTR(sc->function);
	Stack[thread_id][1] = HIGH_BYTE_FROM_PTR(sc->function);
	
	// In ready-Liste einhängen
	kernel_add_to_ready_list(thread_id);

	// Ergebnis des system calls speichern
	sr = &kernel_get_system_call_result()->result_data.register_thread;
	sr->last_registered_thread = thread_id;
}

/**
* Führt den system call SLEEP aus.
*
* @param syscall Die system call-Instanz.
*/
static void kernel_exec_syscall_sleep(const system_call_t *syscall) using 1
{
	static syscall_sleep_t *sc;
	static tcb_t *tcb;
	static tcb_list_item_t *tcb_list_item;
	
	// system call und Ergebnis-Instanz beziehen
	sc = (syscall_sleep_t *)&syscall->call_data;
		
	// Control Block-Listenitem beziehen und initialisieren
	tcb_list_item = &tcb_list[current_thread_id];
	
	// Control Block beziehen und Werte setzen
	tcb = &tcb_list_item->tcb;
	tcb->sleep_duration = sc->sleep_duration;
		
	// Aus ready-Liste entfernen
	kernel_remove_from_ready_list(current_thread_id);
	kernel_add_to_sleep_list(current_thread_id);
}

/**
* Scheduler
*
* Ermittelt den nächsten rechenwilligen Thread. 
* @returns id des nächsten rechenwilligen Threads
*/
uint8_t kernel_schedule_next_thread() using 1
{
	// Der nächste rechenwillige Thread höchster Priorität
	// befindet sich prinzipiell stets am Anfang der Liste.
	
	const int8_t current_priority 	= tcb_list[current_thread_id].tcb.priority;
	const int8_t head_priority 		= tcb_list[tcb_list_ready_head].tcb.priority;
	uint8_t next_thread_id;
	
	// Wenn die Priorität des aktuellen Threads übereinstimmend mit
	// der Priorität am Listenkopf ist und ein nachfolgendes Element
	// mit selber Priorität existiert, soll dieses verwendet werden.
	if (current_thread_id != INV && current_priority == head_priority)
	{
		next_thread_id = tcb_list[current_thread_id].next;
		if (current_priority == tcb_list[tcb_list[current_thread_id].next].tcb.priority) 
		{
			return next_thread_id;
		}
	}
	
	// Da kein fairer Nachfolger gefunden wurde, Kopf der Liste wählen.
	return tcb_list_ready_head;
}

/*****************************************************************************
*                    Timer 0 interrupt service function                      *
*                                                                            *
*   Hierin ist die Thread-Umschaltung als Kern des Multi-Threading Betriebs  *
*                                  enthalten.                                *
*****************************************************************************/
timer0() interrupt 1 using 1						// Int Vector at 000BH, Reg Bank 1  
{
	// Zeiger auf den aktuell laufenden system call
	static system_call_t *syscall;
	
	// Gibt an, ob es sich um den ersten context switch handelt
	static bool is_first_context_switch = true;
	
	static uint8_t regIdx;			// Register-Index in der Schleife
	static uint8_t idata *pi;				// Pointer in das interne RAM
	static uint8_t idata *pd = POSRB0;	// Pointer auf die Registerbank 0
	static int8_t  next_thread_id = FIRST;	// Nr des naechsten Threads
															// Am Anfang ist NewThread auf 
															// einen erkennbar nicht gültigen
															// Wert gesetzt (Grund: s. 
															// "if (NewThread == FIRST)").

	kernel_reload_system_timer();
		
	// Verarbeitung der system calls beginnen
	if (kernel_is_system_call())
	{
		// system call beziehen und auswerten
		syscall = kernel_get_system_call();
		switch (syscall->type)
		{
			case REGISTER_THREAD:
			{
				kernel_exec_syscall_register_thread(syscall);
				break;
			}
			case SLEEP:
			{
				kernel_exec_syscall_sleep(syscall);
				break;
			}
			default:
				assert(0);
				break;
		}
		
		// system call zurücksetzen
		kernel_clear_system_call();
	}
	else // if (is_system_call())
	if (os_running && thread_count > 0)
	{
		// TODO: Liste der Rechenwilligen Threads durchlaufen
		next_thread_id = kernel_schedule_next_thread();

		pi = (unsigned char idata *)SP;			// Kopie des Stackpointers

		// Registertausch nur bei Threadwechsel
		if (next_thread_id != current_thread_id) 
		{						 
			if (is_first_context_switch)
			{	
				// Beim allerersten Aufruf von timer0 liegt der SP noch
				// im ursprünglichen Bereich nach Systemstart. Er darf
				// nicht gerettet werden! Der bei register_thread(...)
				// initialisierte Wert wird verwendet!
			
				is_first_context_switch	 = false;
				next_thread_id = 0;							
			}
			else 
			{
				// Stack pointer sichern
				tcb_list[current_thread_id].tcb.sp  =  pi;
			}               

			// Retten von R0-R7 aus der von allen Threads gemeinsam genutzten Registerbank 0
			for(regIdx=0; regIdx<REGISTER_COUNT; ++regIdx)
			{
				tcb_list[current_thread_id].tcb.reg[regIdx]  = *(pd + regIdx);
			}
			
			// geretteten SP des Threads in Pointer pi laden
			SP = tcb_list[next_thread_id].tcb.sp;						
			pi = (unsigned char idata *)SP;
			
			// Wiederherstellen von R0-R7 in Registerbank 0
			for(regIdx=0; regIdx<REGISTER_COUNT; ++regIdx)
			{
				*(pd + regIdx) = tcb_list[next_thread_id].tcb.reg[regIdx];
			}
		
			// Threadwechsel vollzogen
			current_thread_id = next_thread_id;
		}
	}	// if (os_running)
}


