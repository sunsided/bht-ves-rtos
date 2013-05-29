#ifndef THREADS_H
#define THREADS_H

#include "datatypes.h"

/**
* Zeiger auf eine Threadfunktion.
*/ 
typedef void (thread_function_t)(void);

/**
* Thread-Nummer.
*/
typedef int8_t threadno_t;

/**
* Definiert einen Fehlerfall bei der Threaderzeugung.
*/
#define THREAD_REGISTER_ERROR ((threadno_t)-1)

/**
* Maximale L�nge eines Threadnamens
*/
#define MAX_THREAD_NAME_LENGTH (20)

/**
* Beschreibt die Threadpriorit�t
*/
typedef enum {
	PRIO_HIGHEST 					= 10,	 //< H�chste Priorit�t
	PRIO_HIGH 						= 5,	 //< Hohe Priorit�t
	PRIO_NORMAL  					= 0,	 //< Normale Priorit�t
	PRIO_LOW 					    = -5,  //< Niedrige Priorit�t
	PRIO_LOWEST 					= -10, //< Niedrigste Priorit�t
	PRIO_RESERVED_IDLE 	  = -127 //< Niedrigste Priorit�t (reserviert f�r Idle-Thread)
} thread_priority_t;

/**
* Registriert einen Thread.
*
* @param thread 	Funktionszeiger auf den Thread
* @param priority Priorit�t des Threads
* @returns 		  	Die Nummer des Threads oder THREAD_REGISTER_ERROR im Fehlerfall.
*/
threadno_t os_register_thread(const thread_function_t* thread, thread_priority_t priority, const unsigned char *threadname);

#endif /* THREADS_H */
