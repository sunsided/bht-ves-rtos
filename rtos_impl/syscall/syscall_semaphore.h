/*****************************************************************************
*                                                                            
* Beispiel für einen minimalen Multithreading-Betrieb (MT-Betrieb) 
* auf einem Prozessor der 8051-Familie.
*                                                                            
* Markus Mayer   (Matr-Nr. XXXXXX)			                                 
* Patrick Kaiser (Matr-Nr. YYYYYY)			                                 
*
* Diese Datei enthält die konkrete Definition die Über- und Rückgabestrukturen
* von Semaphor-spezifischen system calls.
*                                                                            
*****************************************************************************/

#ifndef IMPL__SYSCALL_SEMAPHORE_H
#define IMPL__SYSCALL_SEMAPHORE_H

#include "../../rtos/rtos_semaphores.h"

/**
* syscall_init_semaphore_t - Initialisierung eines Semaphors
*/
typedef struct {
	/**
	* Initiale Größe des Semaphors
	*/
	volatile sem_size_t initial_size;
	
} syscall_init_semaphore_t;

/**
* syscall_modify_semaphore_t - Modifikation eines Semaphors
*/
typedef struct {
	/**
	* ID des Semaphors
	*/
	volatile sem_id_t semaphore_id;
	
} syscall_modify_semaphore_t;

/**
* syscall_register_thread_result_t - Ergebnis einer Threadregistrierung
*/
typedef struct {
	/**
	* ID des Semaphors
	*/
	volatile sem_id_t semaphore_id;
	
} syscall_semaphore_result_t;


#endif /* IMPL__SYSCALL_REGISTER_THREAD_H */