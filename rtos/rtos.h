#ifndef RTOS_H
#define RTOS_H

#include "datatypes.h"
#include "rtos_threads.h"
#include "rtos_time.h"
#include "rtos_semaphores.h"

/**
* Initialisiert das Betriebssystem.
*
* Diese Methode muss vor startOS() aufgerufen werden.
*
* @see startOS()
*/
void os_init(void);

/**
* Startet das Betriebssystem.
*
* Setzt einen vorherigen Aufruf von initOS() voraus.
*
* @returns Diese Methode wird niemals verlassen.
* @see initOS()
*/
void os_start(void);

#else /* RTOS_H */
#error rtos.h mehrfach inkludiert
#endif /* not RTOS_H */
