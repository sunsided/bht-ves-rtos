#include <assert.h>
#include <string.h>
#include "systemcall.h"
#include "timer.h"
#include "threads.h"

extern uint8_t thread_count;
extern int8_t current_thread_id;
extern tcb_list_item_t xdata tcb_list[MAX_THREADS];

/**
* Liefert die Daten des aktuellen Threads
*/
thread_data_t* os_get_current_thread_data()
{
	static thread_data_t* td;
	
	// Atomare Ausführung beginnen, um Veränderung des
	// current_thread_id-Wertes zu verhindern
	os_suppress_system_timer_int();
	
	if (0 == thread_count)
	{
		td = NULL;
	}
	else 
	{
		td = &tcb_list[current_thread_id].tcb.thread_data;
	}
	
	os_allow_system_timer_int();
	
	return td;
}

/**
* Liefert die Daten des aktuellen Threads
*/
thread_data_t* kernel_get_current_thread_data() using 1
{
	static thread_data_t* td;
		
	if (0 == thread_count)
	{
		td = NULL;
	}
	else 
	{
		td = &tcb_list[current_thread_id].tcb.thread_data;
	}
	
	return td;
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

