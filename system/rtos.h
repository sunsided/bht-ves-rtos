#ifndef RTOS_H
#define RTOS_H

#include "datatypes.h"

/**
* Initialisiert das Betriebssystem.
*
* Diese Methode muss vor startOS() aufgerufen werden.
*
* @see startOS()
*/
void initOS(void);

/**
* Startet das Betriebssystem.
*
* Setzt einen vorherigen Aufruf von initOS() voraus.
*
* @returns Diese Methode wird niemals verlassen.
* @see initOS()
*/
void startOS(void);

/**
* Zeiger auf eine Threadfunktion.
*/ 
typedef void (threadFunction_t)(void);

/**
* Registriert einen Thread.
*
* @param thread Funktionszeiger auf den Thread
* @param nr 		Nummer des Threads
*/
void RegisterThread(threadFunction_t* thread, uint8_t nr);

#else /* RTOS_H */
#error rtos.h mehrfach inkludiert
#endif /* not RTOS_H */
