#include <assert.h>
#include <string.h>
#include "systemcall.h"
#include "timer.h"
#include "threads.h"

																	//Threadstacks 
extern uint8_t idata Stack[MAX_THREADS][MAX_THREAD_STACKLENGTH]; 
extern TCB 		 xdata tcb[MAX_THREADS];									//Thread Cntrl. Bl.
extern uint8_t NrThreads;								//Anzahl registr.
extern int8_t CurrentThread;	// Nr des laufenden Threads

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
		td = &tcb[CurrentThread].thread_data;
	}
	
	allow_system_timer_int();
	
	return td;
}

/*****************************************************************************
*              Eintragen eines Threads in die Verwaltungsstrukturen          *
*****************************************************************************/
threadno_t register_thread(const thread_function_t* thread, thread_priority_t priority, const unsigned char *threadname)
{
	system_call_t							*call;
	syscall_register_thread_t *calldata;
	system_call_result_t			*resultdata;
	
	assert(2 == sizeof(thread_function_t*));
	
	call = begin_system_call(REGISTER_THREAD);
	assert(REGISTER_THREAD == call->type);
	
	calldata = &call->call_data.register_thread;
	
	strncpy(calldata->name, threadname, MAX_THREAD_NAME_LENGTH);
	calldata->function = thread;
	calldata->priority = priority;
	
	execute_system_call();

	resultdata = get_system_call_result();
	return resultdata->last_registered_thread;
}

