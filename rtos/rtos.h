/*****************************************************************************
*                                                                            
* Beispiel für einen minimalen Multithreading-Betrieb (MT-Betrieb) 
* auf einem Prozessor der 8051-Familie.
*                                                                            
* Markus Mayer   (Matr-Nr. XXXXXX)			                                 
* Patrick Kaiser (Matr-Nr. YYYYYY)			                                 
*
* Haupt-Includefile für OS-spezifische Operationen. Beinhaltet 
* Funktionsdefinitonen zum Initialisieren und Starten des Betriebssystems,
* sowie Includes der Thread-, Zeit- und Semaphorfunktionalität.
*                                                                            
*****************************************************************************/

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
