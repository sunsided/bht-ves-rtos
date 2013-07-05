/*****************************************************************************
*                                                                            
* Beispiel für einen minimalen Multithreading-Betrieb (MT-Betrieb) 
* auf einem Prozessor der 8051-Familie.
*                                                                            
* Markus Mayer   (Matr-Nr. XXXXXX)			                                 
* Patrick Kaiser (Matr-Nr. YYYYYY)			                                 
*
* Diese Datei enthält die Definitionen für die UART-Operationen im
* Kernel space.
*                                                                            
*****************************************************************************/

#ifndef IMPL__V24_H
#define IMPL__V24_H

/**
* Initialisiert die UART-Schnittstelle.
*/
void os_intialize_uart(void);

#endif /* IMPL__V24_H */