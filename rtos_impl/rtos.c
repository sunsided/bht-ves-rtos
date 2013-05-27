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
#define FIRST       (-1)
#define MAXTHREADS  (5)						// maximale Anz. der verwaltbaren Threads
#define POSRB0      (0x00)					// Position von Reg.Bank 0 im int. RAM
#define SLICE       (4*100)					// Timeslice
#define STACKLEN    (0x20)					// maximale Stacktiefe eines Threads
													// Für Änderungen siehe ***.m51-File

/**
* Bezieht das MSB eines Wortes
*/
#define HIGH_BYTE_FROM_PTR(ptr) (((uint16_t)(ptr) & 0xFF00U) >> 8)

/**
* Bezieht das LSB eines Wortes
*/
#define LOW_BYTE_FROM_PTR(ptr)   ((uint16_t)(ptr) & 0x00FFU)

typedef struct {								// Datentyp für den Thread Control Block
	uint8_t sp;
	uint8_t r0;
	uint8_t r1;
	uint8_t r2;
	uint8_t r3;
	uint8_t r4;
	uint8_t r5;
	uint8_t r6;
	uint8_t r7;
} TCB;

																	//Threadstacks 
uint8_t idata Stack[MAXTHREADS][STACKLEN] _at_ 0x30;   
TCB xdata tcb[MAXTHREADS];									//Thread Cntrl. Bl.
uint8_t NrThreads = 0;								//Anzahl registr.

bool os_initialized = false;
bool os_running = false;

/**
* Startet das Betriebssystem.
*
* @returns Diese Methode wird niemals verlassen.
*/
void startOS(void)
{
	assert(true  == os_initialized);
	assert(false == os_running);

	initialize_system_timer(); // TODO: in init verschieben, idle-Thread soll laufen
	
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
	
	V24Init();
	os_initialized = true;
}

/*****************************************************************************
*              Eintragen eines Threads in die Verwaltungsstrukturen          *
*****************************************************************************/
threadno_t registerThread(thread_function_t* thread, thread_priority_t priority)
{
	// TODO: In system call umwandeln (timer interrupt, user interrupt ...)
	
	threadno_t threadNumber;
	
	assert(2 == sizeof(thread_function_t*));
	
	if (NrThreads == MAXTHREADS)
		return THREAD_REGISTER_ERROR;
	
	threadNumber = (threadno_t)NrThreads++; // NOTE: Logik nimmt an, dass niemals Threads entfernt werden.
	
														// SP erstmals auf die nachfolgend
														// abgelegte Rücksprungadresse
														// + 5 byte für 5 PUSHes
	tcb[threadNumber].sp  = (unsigned char)(&Stack[threadNumber][0] + 6);          

	Stack[threadNumber][0] = LOW_BYTE_FROM_PTR(thread);				// Startadresse des registrierten
	Stack[threadNumber][1] = HIGH_BYTE_FROM_PTR(thread);    	// Threads als Rücksprungadresse
	
	return threadNumber;
}



/*****************************************************************************
*                    Timer 0 interrupt service function                      *
*                                                                            *
*   Hierin ist die Thread-Umschaltung als Kern des Multi-Threading Betriebs  *
*                                  enthalten.                                *
*****************************************************************************/
timer0() interrupt 1 using 1						// Int Vector at 000BH, Reg Bank 1  
{
	static uint16_t intcycle = 0;			// Zähler für die Anz. der Interr.
	static uint8_t idata * pi;				// Pointer in das interne RAM
	static uint8_t idata *pd = POSRB0;	// Pointer auf die Registerbank 0
	static uint8_t CurrentThread = 0;	// Nr des laufenden Threads
	static uint8_t NewThread = FIRST;	// Nr des naechsten Threads
															// Am Anfang ist NewThread auf 
															// einen erkennbar nicht gültigen
															// Wert gesetzt (Grund: s. 
															// "if (NewThread == FIRST)").

	if (NrThreads > 0) {								// Sind Threads zu verwalten?
		if (++intcycle == SLICE)  {
			intcycle = 0;
			NewThread = (CurrentThread + 1)%NrThreads;	// Threadumschaltung
		}  
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

			tcb[CurrentThread].r0  = *(pd + 0);		// Retten von R0-R7 aus der
			tcb[CurrentThread].r1  = *(pd + 1);		// von allen Threads gemeinsam
			tcb[CurrentThread].r2  = *(pd + 2);		// genutzten Registerbank 0
			tcb[CurrentThread].r3  = *(pd + 3);    
			tcb[CurrentThread].r4  = *(pd + 4);    
			tcb[CurrentThread].r5  = *(pd + 5);    
			tcb[CurrentThread].r6  = *(pd + 6);
			tcb[CurrentThread].r7  = *(pd + 7);
			
			SP = tcb[NewThread].sp;						// geretteten SP des Threads
			pi = (unsigned char idata *)SP;			// in Pointer pi laden
			
			*(pd + 0) = tcb[NewThread].r0;			// Wiederherstellen von R0-R7
			*(pd + 1) = tcb[NewThread].r1;			// in Registerbank 0
			*(pd + 2) = tcb[NewThread].r2;
			*(pd + 3) = tcb[NewThread].r3;
			*(pd + 4) = tcb[NewThread].r4;
			*(pd + 5) = tcb[NewThread].r5;
			*(pd + 6) = tcb[NewThread].r6;
			*(pd + 7) = tcb[NewThread].r7;
		
			CurrentThread = NewThread;					// Ab jetzt ist der neue Thread
		}                                         // der aktuelle!
	}    
}


