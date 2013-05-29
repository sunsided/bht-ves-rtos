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
	uint8_t list_head;
	
} sem_list_item_t;

/**
* Initialisiert die Semaphorliste
*/
void os_initialize_semaphore_list(void);

/**
* Führt den system call SEMAPHORE_INIT aus.
*
* @param syscall Die system call-Instanz.
*/
void kernel_exec_syscall_sem_init(const system_call_t *syscall);

/**
* Führt den system call SEMAPHORE_WAIT aus.
*
* @param syscall Die system call-Instanz.
*/
void kernel_exec_syscall_sem_wait(const system_call_t *syscall);

/**
* Führt den system call SEMAPHORE_POST aus.
*
* @param syscall Die system call-Instanz.
*/
void kernel_exec_syscall_sem_post(const system_call_t *syscall);

#endif /* IMPL__SEMAPHORES_H */