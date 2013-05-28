#ifndef IMPL__SYSCALL_REGISTER_THREAD_H
#define IMPL__SYSCALL_REGISTER_THREAD_H

#include "../rtos/threads.h"

/**
* syscall_register_thread_t - Registrierung eines Threads
*/
typedef struct {
	/**
	* Die Priorität des Threads
	*/
	thread_priority_t priority;
	
	/**
	* Zeiger auf die Thread-Funktion.
	*/
	thread_function_t *function;
	
	/**
	* Name des Threads
	*/
	unsigned char name[MAX_THREAD_NAME_LENGTH+1];
	
} syscall_register_thread_t;

/**
* syscall_register_thread_result_t - Ergebnis einer Threadregistrierung
*/
typedef struct {
	/**
	* ID des zuletzt registrierten Threads
	*/
	threadno_t last_registered_thread;
	
} syscall_register_thread_result_t;


#endif /* IMPL__SYSCALL_REGISTER_THREAD_H */