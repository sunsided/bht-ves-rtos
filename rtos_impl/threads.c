#include <assert.h>
#include <string.h>
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
* Liefert die Daten des aktuellen Threads
*/
thread_data_t* os_get_current_thread_data()
{
	if (0 == thread_count)
	{
		return NULL;
	}
	
	return &tcb_list[current_thread_id].tcb.thread_data;
}

/**
* Liefert die Daten des aktuellen Threads
*/
thread_data_t* kernel_get_current_thread_data() using 1
{	
	if (0 == thread_count)
	{
		return NULL;
	}
	
	return &tcb_list[current_thread_id].tcb.thread_data;
}

/*****************************************************************************
*              Eintragen eines Threads in die Verwaltungsstrukturen          *
*****************************************************************************/
threadno_t os_register_thread(const thread_function_t* thread, thread_priority_t priority, const unsigned char *threadname)
{
	system_call_t							*sc;
	syscall_register_thread_t *calldata;
	system_call_result_t			*sr;
	int8_t 										id;
	
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

