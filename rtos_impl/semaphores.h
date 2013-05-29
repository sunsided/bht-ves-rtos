#ifndef IMPL__SEMAPHORES_H
#define IMPL__SEMAPHORES_H

#include "../rtos/semaphores.h"

/**
* Maximale Anzahl an Semaphoren
*/
#define	MAX_SEMAPHORE_COUNT		(5)

/**
* Semaphor-Kontrollblock
*/
typedef struct
{
	/**
	* Wert des Semaphors
	*/
	sem_size_t value;
	
	/**
	* Nächster Semaphor in der Warteliste.
	*/
	sem_id_t next;
	
} sem_list_item_t;

/**
* Initialisiert die Semaphorliste
*/
void os_initialize_semaphore_list(void);

#endif /* IMPL__SEMAPHORES_H */