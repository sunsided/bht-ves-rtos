/*****************************************************************************
*                                                                            
* Beispiel f�r einen minimalen Multithreading-Betrieb (MT-Betrieb) 
* auf einem Prozessor der 8051-Familie.
*                                                                            
* Markus Mayer   (Matr-Nr. XXXXXX)			                                 
* Patrick Kaiser (Matr-Nr. YYYYYY)			                                 
*
* Diese Datei enth�lt die konkrete Definition die �ber- und R�ckgabestrukturen
* von Timer-spezifischen system calls.
*                                                                            
*****************************************************************************/

#ifndef IMPL__SYSCALL_SLEEP_H
#define IMPL__SYSCALL_SLEEP_H

#include "../../rtos/datatypes.h"

/**
* syscall_register_thread_t - sleep-Anforderung
*/
typedef struct {

	/**
	* Schlafdauer in ms.
	*/
	volatile sleep_t sleep_duration;
	
} syscall_sleep_t;


#endif /* IMPL__SYSCALL_SLEEP_H */