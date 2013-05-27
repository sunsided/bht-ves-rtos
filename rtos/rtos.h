#ifndef RTOS_H
#define RTOS_H

#include "datatypes.h"
#include "threads.h"

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

#else /* RTOS_H */
#error rtos.h mehrfach inkludiert
#endif /* not RTOS_H */
