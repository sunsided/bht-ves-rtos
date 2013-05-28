#include <assert.h>
#include "timer.h"
#include "threads.h"
#include "systemcall.h"

/**
* Globaler Speicher für system call.
*/
system_call_t system_call;

/**
* Initiiert einen system call.
*/
system_call_t* begin_system_call(const system_call_type type) 
{
	system_call_result_t *result;
	assert(NO_SYSTEM_CALL != type);
	
	suppress_system_timer_int();
	
	result = get_system_call_result();

	result->type = type;
	system_call.type = type;
	
	return &system_call;
}

/**
* Bezieht den aktuell laufenden system call.
*/
system_call_t* get_system_call()
{
		return &system_call;
}

/**
* Führt den system call aus.
*/
void execute_system_call()
{
	trigger_system_timer_overflow();
	allow_system_timer_int();
}

/**
* Setzt den system call zurück.
*/
void clear_system_call()
{
	system_call.type = NO_SYSTEM_CALL;
}

/**
* Bezieht das Ergebnis des system calls.
*/
system_call_result_t* get_system_call_result()
{
	thread_data_t *thread_data;
	system_call_result_t *result;
	
	thread_data = get_current_thread_data();
	result = &thread_data->syscall_result;
	
	result->type = system_call.type;
	
	return result;
}

/**
* Ermittelt, ob es sich um einen system call handelt.
*/
bool is_system_call()
{
	return NO_SYSTEM_CALL != system_call.type;
}