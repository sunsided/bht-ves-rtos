#include <assert.h>
#include "threads.h"

/**
* Bezieht das MSB eines Wortes
*/
#define HIGH_BYTE_FROM_PTR(ptr) (((uint16_t)(ptr) & 0xFF00U) >> 8)

/**
* Bezieht das LSB eines Wortes
*/
#define LOW_BYTE_FROM_PTR(ptr)   ((uint16_t)(ptr) & 0x00FFU)

																	//Threadstacks 
extern uint8_t idata Stack[MAX_THREADS][MAX_THREAD_STACKLENGTH]; 
extern TCB 		 xdata tcb[MAX_THREADS];									//Thread Cntrl. Bl.
extern uint8_t NrThreads;								//Anzahl registr.

/*****************************************************************************
*              Eintragen eines Threads in die Verwaltungsstrukturen          *
*****************************************************************************/
threadno_t registerThread(thread_function_t* thread, thread_priority_t priority)
{
	// TODO: In system call umwandeln (timer interrupt, user interrupt ...)
	
	threadno_t threadNumber;
	
	assert(2 == sizeof(thread_function_t*));
	
	if (NrThreads == MAX_THREADS)
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

