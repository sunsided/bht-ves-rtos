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
tcb_list_item_t xdata tcb_list[MAX_THREADS];									//Thread Cntrl. Bl.

/**
* Not in list.
*
* Markiert das Ende einer Liste
*/
#define NIL (255)

/**
* Kopf der Ready-Liste
*
* Kopf der Liste der rechenwilligen Threads.
*/
static uint8_t tcb_list_ready_head = NIL;

/**
* Anzahl der registrierten Threads
*/
uint8_t thread_count = 0;

/**
* Index des aktuellen Threads
*/
int8_t current_thread_id = 0;

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
	
	assert(false == os_running);
	
	os_intialize_uart();
	os_initialize_system_timer();
	
	idle_thread_no = os_register_thread(idle_thread, PRIO_RESERVED_IDLE, "Idle Thread");
	assert(0 == idle_thread_no);

	os_initialized = true;
}

static void kernel_strncpy(unsigned char *dst, const unsigned char *src, uint8_t length) using 1
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
* Führt den system call REGISTER_THREAD aus.
*
* @param syscall Die system call-Instanz.
*/
static void kernel_exec_syscall_register_thread(const system_call_t *syscall) using 1
{
	static syscall_register_thread_t *sc;
	static syscall_register_thread_result_t *sr;
	static threadno_t threadNumber;
	static tcb_t *tcb;
	static tcb_list_item_t *tcb_list_item;
	
	// system call und Ergebnis-Instanz beziehen
	sc = (syscall_register_thread_t *)&syscall->call_data;
	sr = &kernel_get_system_call_result()->result_data.register_thread;
	
	// Sicherstellen, dass noch nicht alle Threads vergeben sind
	if (MAX_THREADS == thread_count)
	{
		threadNumber = THREAD_REGISTER_ERROR;
	}
	else
	{	
		threadNumber = (threadno_t)thread_count++; // NOTE: Logik nimmt an, dass niemals Threads entfernt werden.
		
		// Control Block-Listenitem beziehen und initialisieren
		tcb_list_item = &tcb_list[threadNumber];
		tcb_list_item->next = NIL;
		
		// Control Block beziehen und Werte setzen
		tcb = &tcb_list_item->tcb;
		tcb->priority = sc->priority;
		kernel_strncpy(tcb->thread_data.name, sc->name, MAX_THREAD_NAME_LENGTH);
		
		// SP erstmals auf die nachfolgend abgelegte Rücksprungadresse + 5 byte für 5 PUSHes
		tcb->sp  = (unsigned char)(&Stack[threadNumber][0] + 6);

		// Startadresse des registrierten Threads als Rücksprungadresse sichern
		Stack[threadNumber][0] = LOW_BYTE_FROM_PTR(sc->function);
		Stack[threadNumber][1] = HIGH_BYTE_FROM_PTR(sc->function);
	}
	
	sr->last_registered_thread = threadNumber;
}

/*****************************************************************************
*                    Timer 0 interrupt service function                      *
*                                                                            *
*   Hierin ist die Thread-Umschaltung als Kern des Multi-Threading Betriebs  *
*                                  enthalten.                                *
*****************************************************************************/
timer0() interrupt 1 using 1						// Int Vector at 000BH, Reg Bank 1  
{
	static system_call_t *syscall;	// Zeiger auf den aktuell laufenden system call
	
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
			default:
				assert(0);
				break;
		}
		
		// system call zurücksetzen
		kernel_clear_system_call();
	}
	else // if (is_system_call())
	if (os_running)
	{
		// Sind Threads zu verwalten?
		if (thread_count > 0) 
		{

			next_thread_id = (current_thread_id + 1)%thread_count;	// Threadumschaltung

			pi = (unsigned char idata *)SP;			// Kopie des Stackpointers

			if (next_thread_id != current_thread_id) {		// Nur bei Threadwechsel müssen
																// die Register gerettet werden!
					 
				if (next_thread_id == FIRST)					// Beim allerersten Aufruf von                
					next_thread_id = 0;							// timer0 liegt der SP noch
																// im ursprünglichen Bereich
																// nach Systemstart. Er darf
																// nicht gerettet werden! Der
																// bei RegisterThread(...)
																// initialisierte Wert wird
																// verwendet!
				else {
					tcb_list[current_thread_id].tcb.sp  =  pi;			// Sichern des SP
				}               

				// Retten von R0-R7 aus der von allen Threads gemeinsam genutzten Registerbank 0
				for(regIdx=0; regIdx<REGISTER_COUNT; ++regIdx)
				{
					tcb_list[current_thread_id].tcb.reg[regIdx]  = *(pd + regIdx);
				}
				
				SP = tcb_list[next_thread_id].tcb.sp;						// geretteten SP des Threads
				pi = (unsigned char idata *)SP;			// in Pointer pi laden
				
				// Wiederherstellen von R0-R7 in Registerbank 0
				for(regIdx=0; regIdx<REGISTER_COUNT; ++regIdx)
				{
					*(pd + regIdx) = tcb_list[next_thread_id].tcb.reg[regIdx];
				}
			
				current_thread_id = next_thread_id;					// Ab jetzt ist der neue Thread
			}                                         // der aktuelle!
		}
	}	
}


