/*****************************************************************************
*                                                                            *
*      Implementierung des Echtzeitkernels                                   *
*                                                                            *
*****************************************************************************/
 

#include <reg51.h>
#include <stdio.h>
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
* Anzahl der registrierten Threads
*/
uint8_t NrThreads = 0;

/**
* Index des aktuellen Threads
*/
int8_t CurrentThread = 0;

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
void start_os(void)
{
	assert(true  == os_initialized);
	assert(false == os_running);

	start_system_timer();
	
	os_running = true;
	while (ALWAYS);
}

/**
* Initialisiert das Betriebssystem.
*/
void init_os(void)
{
	threadno_t idle_thread_no;
	
	assert(false == os_running);
	
	intialize_uart();
	initialize_system_timer();
	
	idle_thread_no = register_thread(idle_thread, PRIO_RESERVED_IDLE, "Idle Thread");
	assert(0 == idle_thread_no);
	
	os_initialized = true;
}

/**
* Führt den system call REGISTER_THREAD aus.
*
* @param syscall Die system call-Instanz.
*/
static void exec_syscall_register_thread(const system_call_t *syscall) using 1
{
	static syscall_register_thread_t *sc;
	static syscall_register_thread_result_t *sr;
	static threadno_t threadNumber;
	static tcb_t *tcb;
	
	// system call und Ergebnis-Instanz beziehen
	sc = (syscall_register_thread_t *)&syscall->call_data;
	sr = &get_system_call_result()->result_data.register_thread;
	
	printf("* registering thread \"%s\" with priority %d ...\r\n", sc->name, (uint16_t)sc->priority);
	
	// Sicherstellen, dass noch nicht alle Threads vergeben sind
	if (MAX_THREADS == NrThreads)
	{
		printf("* error registering thread: no more threads allowed.\r\n");
		threadNumber = THREAD_REGISTER_ERROR;
	}
	else
	{
		threadNumber = (threadno_t)NrThreads++; // NOTE: Logik nimmt an, dass niemals Threads entfernt werden.
		printf("* assigning thread id: %d.\r\n", (uint16_t)threadNumber);
		
		// Control Block beziehen und Priorität setzen
		tcb = &tcb_list[threadNumber].tcb;
		tcb->priority = sc->priority;
		strncpy(tcb->thread_data.name, sc->name, MAX_THREAD_NAME_LENGTH);
		
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
	static int8_t  NewThread = FIRST;	// Nr des naechsten Threads
															// Am Anfang ist NewThread auf 
															// einen erkennbar nicht gültigen
															// Wert gesetzt (Grund: s. 
															// "if (NewThread == FIRST)").

	reload_system_timer();
		
	// Verarbeitung der system calls beginnen
	if (is_system_call())
	{
		// system call beziehen und auswerten
		syscall = get_system_call();
		switch (syscall->type)
		{
			case REGISTER_THREAD:
			{
				exec_syscall_register_thread(syscall);
				break;
			}
			default:
				assert(0);
		}
		
		// system call zurücksetzen
		clear_system_call();
	}
	else // if (is_system_call())
	if (os_running)
	{
		// Sind Threads zu verwalten?
		if (NrThreads > 0) 
		{

			NewThread = (CurrentThread + 1)%NrThreads;	// Threadumschaltung

			pi = (unsigned char idata *)SP;			// Kopie des Stackpointers

			if (NewThread != CurrentThread) {		// Nur bei Threadwechsel müssen
																// die Register gerettet werden!
					 
				if (NewThread == FIRST)					// Beim allerersten Aufruf von                
					NewThread = 0;							// timer0 liegt der SP noch
																// im ursprünglichen Bereich
																// nach Systemstart. Er darf
																// nicht gerettet werden! Der
																// bei RegisterThread(...)
																// initialisierte Wert wird
																// verwendet!
				else {
					tcb_list[CurrentThread].tcb.sp  =  pi;			// Sichern des SP
				}               

				// Retten von R0-R7 aus der von allen Threads gemeinsam genutzten Registerbank 0
				for(regIdx=0; regIdx<REGISTER_COUNT; ++regIdx)
				{
					tcb_list[CurrentThread].tcb.reg[regIdx]  = *(pd + regIdx);
				}
				
				SP = tcb_list[NewThread].tcb.sp;						// geretteten SP des Threads
				pi = (unsigned char idata *)SP;			// in Pointer pi laden
				
				// Wiederherstellen von R0-R7 in Registerbank 0
				for(regIdx=0; regIdx<REGISTER_COUNT; ++regIdx)
				{
					*(pd + regIdx) = tcb_list[NewThread].tcb.reg[regIdx];
				}
			
				CurrentThread = NewThread;					// Ab jetzt ist der neue Thread
			}                                         // der aktuelle!
		}
	}	
}


