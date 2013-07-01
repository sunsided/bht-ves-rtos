/*****************************************************************************
*                                                                            
* Beispiel f�r einen minimalen Multithreading-Betrieb (MT-Betrieb) 
* auf einem Prozessor der 8051-Familie.
*                                                                            
* Markus Mayer   (Matr-Nr. XXXXXX)			                                 
* Patrick Kaiser (Matr-Nr. YYYYYY)			                                 
*
* Implementierung des Echtzeitkernels.
* 
* Beinhaltet konkrete Implementierung der user-space-Funktionen zum
* Initialisieren und Starten des Kernels, sowie Implementierungen des
* overflow-Interrupthandlers des Timers 0, der Methoden zum Einreihen der
* Threads in die ready-Listen und sonstiger Hilfsfunktionen.
*                                                                            
*****************************************************************************/

// NOTE: Verwendung von assert aus mehreren Registerbanken erzeugt WARNING L15

#include <reg51.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "system.h"
#include "v24.h"
#include "timer.h"
#include "systemcall.h"
#include "threads.h"
#include "semaphores.h"

#include "../rtos/rtos.h"

/**
* Hilfsdefinition f�r while(ALWAYS) {}
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
volatile uint8_t thread_count = 0;

/**
* Index des aktuellen Threads
*/
volatile threadno_t current_thread_id = -1;

/**
* ID des idle-Threads
*/
volatile threadno_t idle_thread_id;

/**
* Die Systemzeit in Millisekunden seit Start.
*/
volatile systime_t system_time = 0;

/**
* Gibt an, ob das OS initialisiert wurde
*/
static bool os_initialized = false;

/**
* Gibt an, ob das OS l�uft wurde
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
* Initialisiert die TCB-Liste
*/
void os_initialize_tcb_list(void)
{
	uint8_t tcb_idx;
	for (tcb_idx = 0; tcb_idx < MAX_THREADS; ++tcb_idx)
	{
		tcb_list[tcb_idx].next = NIL;
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
	assert(false == os_running);
	
	os_initialize_system_calls();
	
	os_initialize_tcb_list();
	os_initialize_semaphore_list();
	
	os_intialize_uart();
	os_initialize_system_timer();
	
	idle_thread_id = os_register_thread(idle_thread, PRIO_RESERVED_IDLE, "Idle Thread");
	assert(0 == idle_thread_id);

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
* Aktualisiert die Systemzeit
*/
static void kernel_update_system_time() using 1
{
	system_time += TICK_DURATION_MS;
}

/**
* F�gt einen Thread zur ready-Liste hinzu.
*
* @param thread_id Die ID des einzusortierenden Threads.
*/
void kernel_add_to_ready_list(const uint8_t thread_id) using 1
{
	static uint8_t					token_id;
	
	// Zeiger extrahieren, um array-lookups zu reduzieren
	static tcb_list_item_t *token;
	static tcb_list_item_t *prev;
	static tcb_list_item_t *new_item;
	
	new_item = &tcb_list[thread_id];
	
	// Als rechenwillig markieren
	new_item->tcb.state = READY;
	
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
			
			// Wenn das neue item h�herer Priorit�t als das bestehende item ist,
			// soll der neue Eintrag vor dem bestehenden Eintrag einsortiert werden.
			if (new_item->tcb.priority > token->tcb.priority)
			{
				new_item->next = token_id;
				
				// Wenn ein Vorg�nger existiert, wird dessen next-Zeiger
				// auf den neuen Thread gesetzt.
				if (NULL != prev)
				{
					prev->next = thread_id;
				}
				else
				{
					// Da kein Vorg�nger existiert, sind wir am Beginn der
					// Liste und m�ssen somit den head-Zeiger auf uns stellen.
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

				// An dieser Stelle angekommen, ist die Priorit�t des neuen
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
void kernel_remove_from_ready_list(const uint8_t thread_id) using 1
{
	static uint8_t token_id;
	
	// wenn keine rechenwilligen Threads registriert sind - abbrechen.
	if (NIL == tcb_list_ready_head) {
		return;
	}
	
	// Als blockiert markieren
	tcb_list[thread_id].tcb.state = BLOCKED;
	
	// ist der zu entfernende Thread am Kopf der Liste, 
	// Liste auf den next-Eintrag �ndern.
	if (thread_id == tcb_list_ready_head) 
	{
		tcb_list_ready_head = tcb_list[thread_id].next;
		tcb_list[thread_id].next = NIL;
		return;
	}
	
	// Liste durchlaufen, bis Vorg�nger von thread_id gefunden
	token_id = tcb_list_ready_head;
	while (NIL != token_id)
	{
		// Wenn der Vorg�nger gefunden wurde, den next-Zeiger
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
* F�gt einen Thread zur sleep-Liste hinzu.
*
* Es wird davon ausgegangen, dass die Schlafzeit bereits im TCB hinterlegt ist.
* Die Zeit des ersten Elementes der sleep list ist absolut gespeichert, alle
* weiteren Eintr�ge speichern die Differenz zum Vorg�ngerwert, so dass die Summe
* des Eintrags und aller Vorg�nger gleich der gew�nschten Schlafzeit ist.
*
* Beispiel:
* Die Schalfzeiten 7s, 5s und 10s sollen hinterlegt werden. Die Liste enth�lt
* folglich die Eintr�ge: 5s -> 2s -> 3s (5+2=7, 5+2+3=10)
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
	
	// Wenn noch keine Eintr�ge existieren, kann der Wert 
	// direkt �bernommen (d.h. beibehalten) werden.
	// Der Kopf der Liste wird auf den thread gesetzt.
	if (NIL == tcb_list_sleep_head)
	{
		tcb_list_sleep_head = thread_id;
		tcb_list[thread_id].next = NIL;
		return;
	}
	
	// Da die Liste bereits Eintr�ge beinhaltet, muss
	// der korrekte Einf�gepunkt ermittelt werden, wozu
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
			
			// Die nachfolgenden Elemente m�ssen entsprechend der reduzierten Zeit
			// korrigiert werden.
			if (NIL != token_id)
			{
				tcb_list[token_id].tcb.sleep_duration -= thread_sleep_time;
			}
			
			// Wenn ein Vorg�ngerelement existiert, dieses anpassen
			if (NIL != prev_id) 
			{
				// wenn prev identisch mit thread,
				// war nur ein Eintrag in der Liste.
				if (prev_id != thread_id)
				{
					tcb_list[prev_id].next = thread_id;
				}
			}
			else
			{
				// Da kein Vorg�nger existiert, muss der Listenkopf gesetzt werden.
				tcb_list_sleep_head = thread_id;
			}
			
			return;
		}
		
		// Die Schlafzeit des Threads ist gr��er als die integrierte
		// Schlafzeit, daher Thread-Zeit verringern und n�chstes Element anw�hlen.
		thread_sleep_time -= tcb_list[token_id].tcb.sleep_duration;
		prev_id = token_id;
		token_id = tcb_list[token_id].next;
	}
}

/**
* Aktualisiert die sleep-Liste.
*/
static void kernel_update_sleep_list() using 1
{
	static uint8_t thread_id;
	
	if (NIL == tcb_list_sleep_head) {
		return;
	}
	
	// Schlafzeit am head reduzieren
	tcb_list[tcb_list_sleep_head].tcb.sleep_duration -= TICK_DURATION_MS;
	
	// Solange sich an der Spitze der Liste items mit Schlafzeiten
	// kleiner oder gleich null befinden (etwa, weil sich identische 
	// Restzeiten ergaben), diese entfernen und rechenwillig machen.
	while (NIL != tcb_list_sleep_head && tcb_list[tcb_list_sleep_head].tcb.sleep_duration <= 0)
	{
		thread_id = tcb_list_sleep_head;

		tcb_list_sleep_head = tcb_list[thread_id].next;	
		tcb_list[thread_id].tcb.sleep_duration = 0;
		tcb_list[thread_id].next = NIL;
	
		kernel_add_to_ready_list(thread_id);
	}
}

/**
* F�hrt den system call REGISTER_THREAD aus.
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
	// �berpr�fung wird bereits im user space ausgef�hrt.
	assert (MAX_THREADS != thread_count);
	
	// Thread ID ermitteln und 
	thread_id = (threadno_t)thread_count++; // NOTE: Logik nimmt an, dass niemals Threads entfernt werden.
	
	// Control Block-Listenitem beziehen und initialisieren
	tcb_list_item = &tcb_list[thread_id];
	tcb_list_item->next = NIL;
	
	// Control Block beziehen und Werte setzen
	tcb = &tcb_list_item->tcb;
	tcb->sleep_duration = 0;
	tcb->priority = sc->priority;
	kernel_strncpy(tcb->thread_data.name, sc->name, MAX_THREAD_NAME_LENGTH);
	
	// SP erstmals auf die nachfolgend abgelegte R�cksprungadresse + 5 byte f�r 5 PUSHes
	tcb->sp  = (unsigned char)(&Stack[thread_id][0] + 6);

	// Startadresse des registrierten Threads als R�cksprungadresse sichern
	Stack[thread_id][0] = LOW_BYTE_FROM_PTR(sc->function);
	Stack[thread_id][1] = HIGH_BYTE_FROM_PTR(sc->function);
	
	// In ready-Liste einh�ngen
	kernel_add_to_ready_list(thread_id);

	// Ergebnis des system calls speichern
	sr = &kernel_prepare_system_call_result()->result_data.register_thread;
	sr->last_registered_thread = thread_id;
}

/**
* F�hrt den system call SLEEP aus.
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
		
	// Zeit korrigieren
	if (tcb->sleep_duration <= TICK_DURATION_MS)
	{
		tcb->sleep_duration = TICK_DURATION_MS;
	}
	
	// Aus ready-Liste entfernen
	kernel_remove_from_ready_list(current_thread_id);
	kernel_add_to_sleep_list(current_thread_id);
}

/**
* Scheduler
*
* Ermittelt den n�chsten rechenwilligen Thread. 
* @returns id des n�chsten rechenwilligen Threads
*/
uint8_t kernel_schedule_next_thread() using 1
{
	// Der n�chste rechenwillige Thread h�chster Priorit�t
	// befindet sich prinzipiell stets am Anfang der Liste.
	
	const bool current_is_ready 		= (READY == tcb_list[current_thread_id].tcb.state);
	const int8_t current_priority 	= tcb_list[current_thread_id].tcb.priority;
	const int8_t head_priority 			= tcb_list[tcb_list_ready_head].tcb.priority;
	uint8_t next_thread_id;
	
	// Wenn die Priorit�t des aktuellen Threads �bereinstimmend mit
	// der Priorit�t am Listenkopf ist und ein nachfolgendes Element
	// mit selber Priorit�t existiert, soll dieses verwendet werden.
	if (current_is_ready && 0 <= current_thread_id && current_priority == head_priority)
	{
		next_thread_id = tcb_list[current_thread_id].next;
		if (current_priority == tcb_list[tcb_list[current_thread_id].next].tcb.priority) 
		{
			return next_thread_id;
		}
	}
	
	// Da kein fairer Nachfolger gefunden wurde, Kopf der Liste w�hlen.
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
	
	// Gibt an, ob es sich um einen system call handelte
	static bool is_system_call;
	
	// Gibt an, ob ein context switch ausgef�hrt werden soll
	static bool switch_context;
	
	static uint8_t regIdx;			// Register-Index in der Schleife
	static uint8_t idata *pi;				// Pointer in das interne RAM
	static uint8_t idata *pd = POSRB0;	// Pointer auf die Registerbank 0
	static int8_t  next_thread_id;	// Nr des naechsten Threads


	kernel_reload_system_timer();
		
	// Verarbeitung der system calls beginnen
	is_system_call = kernel_is_system_call();
	
	// Bei system calls davon ausgehen, dass prinzipiell
	// kein context switch erforderlich ist.
	// Impliziert, dass bei regul�ren Aufrufen ein switch
	// durchgef�rt wird.
	switch_context = !is_system_call;
	
	if (is_system_call)
	{
		// system call beziehen und auswerten
		syscall = kernel_get_system_call();
		switch (syscall->type)
		{
			case REGISTER_THREAD:
			{
				kernel_exec_syscall_register_thread(syscall);
				
				// Registrierungs eines Threads bewirkt
				// keinen Threadwechsel.
				switch_context = false;
				break;
			}
			case SLEEP:
			{
				kernel_exec_syscall_sleep(syscall);
				
				// os_sleep() blockiert den aktuellen Thread,
				// weswegen ein Threadwechsel stattfindet.
				switch_context = true;
				break;
			}
			case SEMAPHORE_INIT:
			{
				kernel_exec_syscall_sem_init(syscall);
				
				// Initialisierung eines Semaphors bewirkt
				// keinen Threadwechsel.
				switch_context = false;
				break;
			}
			case SEMAPHORE_WAIT:
			{
				kernel_exec_syscall_sem_wait(syscall);
				
				// wait im system call tritt auf, wenn Semaphor
				// blockiert, daher Threadwechsel durchf�hren.
				switch_context = true;
				break;
			}
			case SEMAPHORE_POST:
			{
				kernel_exec_syscall_sem_post(syscall);
				
				// post im system call tritt auf, wenn Semaphor
				// Threads aufwecken kann, daher Threadwechsel durchf�hren.
				switch_context = true;
				break;
			}
			default:
				assert(0);
				break;
		}
		
		// system call zur�cksetzen
		kernel_clear_system_call();
	}
	
	if (os_running)
	{
		// Kernelzeit nur weiterz�hlen, wenn Timer regul�r getickt hat
		if (!is_system_call)
		{
			kernel_update_system_time();
		}
		
		// Threadmanagement nur, wenn Threads registriert und
		// Threadwechsel erw�nscht ist.
		if (thread_count > 0 && switch_context)
		{
			// sleep list nur modifizieren, wenn Timer regul�r getickt
			if (!is_system_call) 
			{
				kernel_update_sleep_list();
			}
			
			// Kontextwechsel ist n�tig, wenn aktueller Thread in sleep() geht
			next_thread_id = kernel_schedule_next_thread();

			pi = (unsigned char idata *)SP;			// Kopie des Stackpointers

			// Registertausch nur bei Threadwechsel
			if (next_thread_id != current_thread_id) 
			{						 
				if (is_first_context_switch)
				{	
					// Beim allerersten Aufruf von timer0 liegt der SP noch
					// im urspr�nglichen Bereich nach Systemstart. Er darf
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
			} // next != current
		} // thread_count
	}	// if (os_running)
}


