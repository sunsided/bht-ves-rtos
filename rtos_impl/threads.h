#ifndef IMPL__THREADS_H
#define IMPL__THREADS_H

#include "../rtos/threads.h"
#include "systemcall.h"

#define FIRST       (-1)
#define REGISTER_COUNT	(8)
#define MAX_THREADS  (5)						// maximale Anz. der verwaltbaren Threads
#define MAX_THREAD_STACKLENGTH    (0x20)				// maximale Stacktiefe eines Threads
																	// Für Änderungen siehe ***.m51-File

/**
* Daten eines Threads
*/
typedef struct {
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
} tcb_t;


/**
* Verkettete Liste der TCBs.
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
* Liefert die Daten des aktuellen Threads
*/
thread_data_t* get_current_thread_data();

#endif /* IMPL__THREADS_H */