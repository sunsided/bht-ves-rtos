/*****************************************************************************
*                                                                            
* Beispiel für einen minimalen Multithreading-Betrieb (MT-Betrieb) 
* auf einem Prozessor der 8051-Familie.
*                                                                            
* Markus Mayer   (Matr-Nr. XXXXXX)			                                 
* Patrick Kaiser (Matr-Nr. YYYYYY)			                                 
*
* Beinhaltet user-space-Defitionen für Zeit-spezifische Operationen.
*                                                                            
*****************************************************************************/

#ifndef IMPL__SLEEP_H
#define IMPL__SLEEP_H

#include "../rtos/datatypes.h"

/**
* Pausert den Thread für die angegebene Zeit.
*
* @param ms Die Zeit in Millisekunden.
*/
void os_sleep(const sleep_t ms);

/**
* Liefert die Systemzeit in Millisekunden seit Start.
*
* @returns Die Laufzeit.
*/
systime_t os_time();

#endif /* IMPL__SLEEP_H */