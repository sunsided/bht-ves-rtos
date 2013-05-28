/*****************************************************************************
*                                                                            *
*      Implementierung des Echtzeitkernels                                   *
*                                                                            *
*****************************************************************************/
 

#include <reg51.h>
#include <stdlib.h>
#include <assert.h>

#include "../rtos/rtos.h"
#include "v24.h"
#include "timer.h"

#define ALWAYS 		  (1)
#define POSRB0      (0x00)					// Position von Reg.Bank 0 im int. RAM
#define SLICE       (4*100)					// Timeslice

																	//Threadstacks 
uint8_t idata Stack[MAX_THREADS][MAX_THREAD_STACKLENGTH] _at_ 0x30;   
TCB xdata tcb[MAX_THREADS];									//Thread Cntrl. Bl.
uint8_t NrThreads = 0;								//Anzahl registr.

static bool os_initialized = false;
static bool os_running = false;

/**
* Startet das Betriebssystem.
*
* @returns Diese Methode wird niemals verlassen.
*/
void startOS(void)
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
void initOS(void)
{
	assert(false == os_running);
	// TODO: Idle Thread registrieren	
	
	intialize_uart();
	initialize_system_timer();
	os_initialized = true;
}

/*****************************************************************************
*                    Timer 0 interrupt service function                      *
*                                                                            *
*   Hierin ist die Thread-Umschaltung als Kern des Multi-Threading Betriebs  *
*                                  enthalten.                                *
*****************************************************************************/
timer0() interrupt 1 using 1						// Int Vector at 000BH, Reg Bank 1  
{
	static uint8_t idata * pi;				// Pointer in das interne RAM
	static uint8_t idata *pd = POSRB0;	// Pointer auf die Registerbank 0
	static uint8_t CurrentThread = 0;	// Nr des laufenden Threads
	static uint8_t NewThread = FIRST;	// Nr des naechsten Threads
															// Am Anfang ist NewThread auf 
															// einen erkennbar nicht gültigen
															// Wert gesetzt (Grund: s. 
															// "if (NewThread == FIRST)").

	reload_system_timer();
	
	if (NrThreads > 0) {								// Sind Threads zu verwalten?

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
				tcb[CurrentThread].sp  =  pi;			// Sichern des SP
			}               

			tcb[CurrentThread].reg[0]  = *(pd + 0);		// Retten von R0-R7 aus der
			tcb[CurrentThread].reg[1]  = *(pd + 1);		// von allen Threads gemeinsam
			tcb[CurrentThread].reg[2]  = *(pd + 2);		// genutzten Registerbank 0
			tcb[CurrentThread].reg[3]  = *(pd + 3);    
			tcb[CurrentThread].reg[4]  = *(pd + 4);    
			tcb[CurrentThread].reg[5]  = *(pd + 5);    
			tcb[CurrentThread].reg[6]  = *(pd + 6);
			tcb[CurrentThread].reg[7]  = *(pd + 7);
			
			SP = tcb[NewThread].sp;						// geretteten SP des Threads
			pi = (unsigned char idata *)SP;			// in Pointer pi laden
			
			*(pd + 0) = tcb[NewThread].reg[0];			// Wiederherstellen von R0-R7
			*(pd + 1) = tcb[NewThread].reg[1];			// in Registerbank 0
			*(pd + 2) = tcb[NewThread].reg[2];
			*(pd + 3) = tcb[NewThread].reg[3];
			*(pd + 4) = tcb[NewThread].reg[4];
			*(pd + 5) = tcb[NewThread].reg[5];
			*(pd + 6) = tcb[NewThread].reg[6];
			*(pd + 7) = tcb[NewThread].reg[7];
		
			CurrentThread = NewThread;					// Ab jetzt ist der neue Thread
		}                                         // der aktuelle!
	}    
}


