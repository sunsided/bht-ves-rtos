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
* Thread-Nummer.
*/
typedef uint8_t threadno_t;

/**
* Definiert einen Fehlerfall bei der Threaderzeugung.
*/
#define THREAD_REGISTER_ERROR ((threadno_t)-1)

/**
* Registriert einen Thread.
*
* @param thread Funktionszeiger auf den Thread
* @param nr 		Nummer des Threads
* @returns 		  Die Nummer des Threads oder THREAD_REGISTER_ERROR im Fehlerfall.
*/
threadno_t registerThread(threadFunction_t* thread);

#else /* RTOS_H */
#error rtos.h mehrfach inkludiert
#endif /* not RTOS_H */
