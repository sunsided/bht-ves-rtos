/*****************************************************************************
*                                                                            
* Beispiel für einen minimalen Multithreading-Betrieb (MT-Betrieb) 
* auf einem Prozessor der 8051-Familie.
*                                                                            
* Markus Mayer   (Matr-Nr. XXXXXX)			                                 
* Patrick Kaiser (Matr-Nr. YYYYYY)			                                 
*
* Beinhaltet user-space-Definitionen für Sempahor-spezifische Operationen-
*                                                                            
*****************************************************************************/

#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#include "datatypes.h"


/**
* ID des Semaphors
*/
typedef int8_t sem_id_t;

/**
* Größe des Semaphors
*/
typedef uint8_t sem_size_t;

/**
* Semaphor-Typ.
*/ 
typedef struct {
	/**
	* ID des Semaphors
	*/
	sem_id_t semaphore_id;
} semaphore_t;

/**
* Fehlercode für Semaphor-Operationen
*/
typedef enum
{
	SEM_SUCCESS = 0,
	SEM_INVALID_SEMAPHORE = 1,
	SEM_TOO_MANY_SEMAPHORES = 2,
	SEM_INVALID_STATE = 3,
} sem_error_t;

/**
* Initialisiert einen Semaphor
*
* @param semaphore	Der zu initialisierende Semaphor
* @param sem_size		Die initiale Größe des Semaphors
* @returns 		  		SEM_SUCCESS im Erfolgsfall, ansonsten Fehlercode.
*/
sem_error_t os_semaphore_init(semaphore_t* semaphore, const sem_size_t sem_size);

/**
* Gibt den Semaphor frei.
*
* @param semaphore	Der Semaphor
* @returns 		  		SEM_SUCCESS im Erfolgsfall, ansonsten Fehlercode.
*/
sem_error_t os_semaphore_post(const semaphore_t* semaphore);

/**
* Blockiert auf den Semaphor.
*
* @param semaphore	Der Semaphor
* @returns 		  		SEM_SUCCESS im Erfolgsfall, ansonsten Fehlercode.
*/
sem_error_t os_semaphore_wait(const semaphore_t* semaphore);

#endif /* SEMAPHORES_H */
