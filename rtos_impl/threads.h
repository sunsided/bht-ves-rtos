/*****************************************************************************
*                                                                            
* Beispiel für einen minimalen Multithreading-Betrieb (MT-Betrieb) 
* auf einem Prozessor der 8051-Familie.
*                                                                            
* Markus Mayer   (Matr-Nr. XXXXXX)			                                 
* Patrick Kaiser (Matr-Nr. YYYYYY)			                                 
*
* Diese Datei enthält die Definitionen für die Thread-Verarbeitung im
* Kernel space.
*                                                                            
*****************************************************************************/

#ifndef IMPL__THREADS_H
#define IMPL__THREADS_H

#include "../rtos/rtos_threads.h"
#include "systemcall.h"

#define FIRST       (-1)
#define REGISTER_COUNT	(8)
#define MAX_THREADS  (5)						// maximale Anz. der verwaltbaren Threads
#define MAX_THREAD_STACKLENGTH    (0x20)				// maximale Stacktiefe eines Threads
																	// Für Änderungen siehe ***.m51-File
	
/**
* Definiert dem Zustand des Threads
*/
typedef enum {
	/**
	* Blockiert
	*/
	BLOCKED = 0,
	
	/**
	* Rechenwillig
	*/
	READY = 1,
} thread_state_t;	
																	

/**
* Daten eines Threads
*/
typedef struct {
	/**
	* Name des Threads
	*/
	unsigned char name[MAX_THREAD_NAME_LENGTH+1];
	
	/**
	* Ergebnis des letzten system calls
	*/
	system_call_result_t syscall_result;
} thread_data_t;
	

/**
* Thread Control Block
*/
typedef struct {	
	/**
	* Stack pointer
	*/
	uint8_t sp;
	
	/**
	* Register des Threads
	*/
	uint8_t reg[REGISTER_COUNT];
	
	/**
	* Daten des aktuellen Threads
	*/
	thread_data_t thread_data;
	
	/**
	* Die Priorität des Threads.
	*/
	thread_priority_t priority;
	
	/**
	* Dauer eines Sleep-Vorganges.
	*/
	sleep_t sleep_duration;
	
	/**
	* Der Zustand des Threads.
	*/
	thread_state_t state;
} tcb_t;


/**
* Eintrag der verketteten Liste der TCBs.
*/
typedef struct {
	/**
	* Der TCB
	*/
	tcb_t 	tcb;
	
	/**
	* Index des nächsten Listeneintrags
	*/
	uint8_t next;	
} tcb_list_item_t;

/**
* Anzahl der registrierten Threads
*/
extern volatile uint8_t thread_count;

/**
* ID des aktuellen Threads
*/
extern volatile threadno_t current_thread_id;

/**
* ID des idle-Threads
*/
extern volatile threadno_t idle_thread_id;

/**
* Liste der Thread Control Blocks
*/
extern tcb_list_item_t xdata tcb_list[MAX_THREADS];

/**
* Stack-Pointer.
*/
extern uint8_t idata Stack[MAX_THREADS][MAX_THREAD_STACKLENGTH];

/**
* Liefert die Daten des aktuellen Threads
*
* @returns Thread-lokale Daten
*/
thread_data_t* os_get_current_thread_data();

/**
* Liefert die Daten des aktuellen Threads
*
* Wenn das System nocht nicht gestartet wurde, werden
* die Thread-Daten des idle threads verwendet.
*
* @returns Die thread-lokalen Daten des aktuellen Threads oder des idle Threads.
*/
thread_data_t* kernel_get_current_thread_data();

/**
* Liefert die ID des aktuellen Threads
*
* @returns ID des aktuellen Threads oder kleiner null, wenn kein Thread aktiv ist.
*/
threadno_t kernel_get_current_thread_id();

/**
* Entfernt einen thread aus der ready-Liste.
*
* @param thread_id Die id des zu entfernenden Threads.
*/
void kernel_remove_from_ready_list(const uint8_t thread_id);

/**
* Fügt einen Thread zur ready-Liste hinzu.
*
* @param thread_id Die ID des einzusortierenden Threads.
*/
void kernel_add_to_ready_list(const uint8_t thread_id);

#endif /* IMPL__THREADS_H */