/*****************************************************************************
*                                                                            
* Beispiel für einen minimalen Multithreading-Betrieb (MT-Betrieb) 
* auf einem Prozessor der 8051-Familie.
*                                                                            
* Markus Mayer   (Matr-Nr. XXXXXX)			                                 
* Patrick Kaiser (Matr-Nr. YYYYYY)			                                 
*
* Diese Datei enthält die konkrete Definition die Über- und Rückgabestrukturen
* von Thread-spezifischen system calls.
*                                                                            
*****************************************************************************/

#ifndef IMPL__SYSCALL_REGISTER_THREAD_H
#define IMPL__SYSCALL_REGISTER_THREAD_H

#include "../../rtos/rtos_threads.h"

/**
* syscall_register_thread_t - Registrierung eines Threads
*/
typedef struct {
	/**
	* Die Priorität des Threads
	*/
	volatile thread_priority_t priority;
	
	/**
	* Zeiger auf die Thread-Funktion.
	*/
	volatile thread_function_t *function;
	
	/**
	* Name des Threads
	*/
	volatile unsigned char *name;
	
} syscall_register_thread_t;

/**
* syscall_register_thread_result_t - Ergebnis einer Threadregistrierung
*/
typedef struct {
	/**
	* ID des zuletzt registrierten Threads
	*/
	volatile threadno_t last_registered_thread;
	
} syscall_register_thread_result_t;


#endif /* IMPL__SYSCALL_REGISTER_THREAD_H */