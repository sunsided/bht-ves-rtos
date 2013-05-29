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
system_call_t* os_begin_system_call(const system_call_type type) 
{
	system_call_result_t *result;
	assert(NO_SYSTEM_CALL != type);
	
	os_suppress_system_timer_int();
	
	result = os_get_system_call_result();

	result->type = type;
	system_call.type = type;
	
	return &system_call;
}

/**
* Bezieht den aktuell laufenden system call.
*/
system_call_t* kernel_get_system_call() using 1
{
		return &system_call;
}

/**
* Führt den system call aus.
*/
void os_execute_system_call()
{
	os_trigger_system_timer_overflow();
	os_allow_system_timer_int();
}

/**
* Setzt den system call zurück.
*/
void kernel_clear_system_call() using 1
{
	system_call.type = NO_SYSTEM_CALL;
}

/**
* Bezieht das Ergebnis des system calls.
*/
system_call_result_t* os_get_system_call_result()
{
	thread_data_t *thread_data;
	system_call_result_t *result;
	
	thread_data = os_get_current_thread_data();
	result = &thread_data->syscall_result;
	
	result->type = system_call.type;
	
	return result;
}

/**
* Bezieht das Ergebnis des system calls.
*/
system_call_result_t* kernel_get_system_call_result() using 1
{
	thread_data_t *thread_data;
	system_call_result_t *result;
	
	thread_data = kernel_get_current_thread_data();
	result = &thread_data->syscall_result;
	
	result->type = system_call.type;
	
	return result;
}

/**
* Setzt das system call-Ergebnis zurück.
*/
void os_clear_system_call_result()
{
	static system_call_result_t *result;
	
	result = os_get_system_call_result();
	result->type = NO_SYSTEM_CALL;
}

/**
* Ermittelt, ob es sich um einen system call handelt.
*/
bool kernel_is_system_call() using 1
{
	return NO_SYSTEM_CALL != system_call.type;
}