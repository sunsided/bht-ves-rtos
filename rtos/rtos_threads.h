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
* Maximale Länge eines Threadnamens
*/
#define MAX_THREAD_NAME_LENGTH (20)

/**
* Beschreibt die Threadpriorität
*/
typedef enum {
	PRIO_HIGHEST 					= 10,	 //< Höchste Priorität
	PRIO_HIGH 						= 5,	 //< Hohe Priorität
	PRIO_NORMAL  					= 0,	 //< Normale Priorität
	PRIO_LOW 					    = -5,  //< Niedrige Priorität
	PRIO_LOWEST 					= -10, //< Niedrigste Priorität
	PRIO_RESERVED_IDLE 	  = -127 //< Niedrigste Priorität (reserviert für Idle-Thread)
} thread_priority_t;

/**
* Registriert einen Thread.
*
* @param thread 	Funktionszeiger auf den Thread
* @param priority Priorität des Threads
* @returns 		  	Die Nummer des Threads oder THREAD_REGISTER_ERROR im Fehlerfall.
*/
threadno_t os_register_thread(const thread_function_t* thread, thread_priority_t priority, const unsigned char *threadname);

#endif /* THREADS_H */
