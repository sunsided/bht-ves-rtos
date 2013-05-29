#include <assert.h>
#include <string.h>
#include "systemcall.h"
#include "timer.h"
#include "threads.h"

extern uint8_t NrThreads;
extern int8_t CurrentThread;
extern tcb_list_item_t xdata tcb_list[MAX_THREADS];

/**
* Liefert die Daten des aktuellen Threads
*/
thread_data_t* get_current_thread_data()
{
	static thread_data_t* td;
	
	suppress_system_timer_int();
	
	if (0 == NrThreads)
	{
		td = NULL;
	}
	else 
	{
		td = &tcb_list[CurrentThread].tcb.thread_data;
	}
	
	allow_system_timer_int();
	
	return td;
}

/*****************************************************************************
*              Eintragen eines Threads in die Verwaltungsstrukturen          *
*****************************************************************************/
threadno_t register_thread(const thread_function_t* thread, thread_priority_t priority, const unsigned char *threadname)
{
	system_call_t							*sc;
	syscall_register_thread_t *calldata;
	system_call_result_t			*sr;
	
	assert(2 == sizeof(thread_function_t*));
	
	sc = begin_system_call(REGISTER_THREAD);
	assert(REGISTER_THREAD == sc->type);
	
	calldata = &sc->call_data.register_thread;
	
	strncpy(calldata->name, threadname, MAX_THREAD_NAME_LENGTH);
	calldata->function = thread;
	calldata->priority = priority;
	
	execute_system_call();

	sr = get_system_call_result();
	return sr->result_data.register_thread.last_registered_thread;
}

