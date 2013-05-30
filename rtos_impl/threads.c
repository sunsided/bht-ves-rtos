#include <assert.h>
#include <string.h>
#include "system.h"
#include "systemcall.h"
#include "timer.h"
#include "threads.h"

/**
* Liefert die ID des aktuellen Threads
*
* @returns ID des aktuellen Threads oder kleiner null, wenn kein Thread aktiv ist.
*/
threadno_t kernel_get_current_thread_id()
{
	return current_thread_id;
}

/**
* Liefert die Daten des aktuellen Threads.
*
* Wenn das System nocht nicht gestartet wurde, werden
* die Thread-Daten des idle threads verwendet.
*
* @returns Die thread-lokalen Daten des aktuellen Threads oder des idle Threads.
*/
thread_data_t* os_get_current_thread_data()
{
	static uint8_t thread_id;
	
	if (0 == thread_count)
	{
		return NULL;
	}
	
		// Thread-ID ermitteln
	thread_id = current_thread_id;
	if (NIL == current_thread_id)
	{
		// Da current_thread_id aufgrund des noch nicht erfolgten
		// Kontextwechsels noch nicht definiert ist, wird die
		// ID des idle threads verwendet
		thread_id = idle_thread_id;
	}
	
	return &tcb_list[thread_id].tcb.thread_data;
}

/**
* Liefert die Daten des aktuellen Threads.
*
* Wenn das System nocht nicht gestartet wurde, werden
* die Thread-Daten des idle threads verwendet.
*
* @returns Die thread-lokalen Daten des aktuellen Threads oder des idle Threads.
*/
thread_data_t* kernel_get_current_thread_data() using 1
{	
	static uint8_t thread_id;
	
	if (0 == thread_count)
	{
		return NULL;
	}
	
	// Thread-ID ermitteln
	thread_id = current_thread_id;
	if (NIL == current_thread_id)
	{
		// Da current_thread_id aufgrund des noch nicht erfolgten
		// Kontextwechsels noch nicht definiert ist, wird die
		// ID des idle threads verwendet
		thread_id = idle_thread_id;
	}
	
	return &tcb_list[thread_id].tcb.thread_data;
}

/*****************************************************************************
*              Eintragen eines Threads in die Verwaltungsstrukturen          *
*****************************************************************************/
threadno_t os_register_thread(const thread_function_t* thread, thread_priority_t priority, const unsigned char *threadname)
{
	static system_call_t							*sc;
	static syscall_register_thread_t *calldata;
	static system_call_result_t			*sr;
	static int8_t 										id;
	
	assert(2 == sizeof(thread_function_t*));
	
	sc = os_begin_system_call(REGISTER_THREAD);
	assert(REGISTER_THREAD == sc->type);
	
	// Wenn keine Threads mehr registrierbar,
	// System call abbrechen (atomaren Bereich verlassen)
	if (MAX_THREADS == thread_count)
	{
		os_cancel_execute_system_call();
		return THREAD_REGISTER_ERROR;
	}
	
	calldata = &sc->call_data.register_thread;
	
	calldata->name = threadname;
	calldata->function = thread;
	calldata->priority = priority;
	
	os_execute_system_call();

	sr = os_get_system_call_result();
	id = sr->result_data.register_thread.last_registered_thread;
	
	os_clear_system_call_result();
	
	return id;
}

