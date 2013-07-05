/*****************************************************************************
*                                                                            
* Beispiel f�r einen minimalen Multithreading-Betrieb (MT-Betrieb) 
* auf einem Prozessor der 8051-Familie.
*                                                                            
* Markus Mayer   (Matr-Nr. XXXXXX)			                                 
* Patrick Kaiser (Matr-Nr. YYYYYY)			                                 
*
* Diese Datei enth�lt die systemspezifische Einstellungen und sonstige
* Defines.
*                                                                            
*****************************************************************************/

#ifndef IMPL__SYSTEM_H
#define IMPL__SYSTEM_H

/**
* CPU-Frequenz in Hz.
*/
#define F_CPU (10000000UL)

/**
* L�nge eines Ticks in ms.
*/
#define TICK_DURATION_MS (10U)

/**
* Not in list.
*
* Markiert das Ende einer Liste.
*/
#define NIL (0xFF)

#endif /* IMPL__SYSTEM_H */