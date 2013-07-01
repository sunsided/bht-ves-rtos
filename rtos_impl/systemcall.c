/*****************************************************************************
*                                                                            
* Beispiel für einen minimalen Multithreading-Betrieb (MT-Betrieb) 
* auf einem Prozessor der 8051-Familie.
*                                                                            
* Markus Mayer   (Matr-Nr. XXXXXX)			                                 
* Patrick Kaiser (Matr-Nr. YYYYYY)			                                 
*
* Implementierung von user- und kernel space-Funktionen für die
* Ausführung von system calls.
*                                                                            
*****************************************************************************/

#include <assert.h>
#include "timer.h"
#include "threads.h"
#include "systemcall.h"

/**
* Globaler Speicher für system call.
*/
volatile system_call_t system_call;

// TODO: Globaler Zeiger auf call-result, den der Dispatcher umsetzt.

/**
* Initialisiert die system call-Schnittstelle.
*/
void os_initialize_system_calls()
{
	system_call.type = NO_SYSTEM_CALL;
}

/**
* Initiiert einen system call.
*/
system_call_t* os_begin_system_call(const system_call_type type) 
{
	assert(NO_SYSTEM_CALL != type);
	
	os_suppress_system_timer_int();
	
	system_call.type = type;
	return &system_call;
}

/**
* Bricht den system call ab.
*/
void os_cancel_execute_system_call()
{
	system_call.type = NO_SYSTEM_CALL;
	os_allow_system_timer_int();
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
	static thread_data_t *thread_data;
	
	thread_data = os_get_current_thread_data();
	return &thread_data->syscall_result;
}

/**
* Bezieht das Ergebnis des system calls.
*/
system_call_result_t* kernel_prepare_system_call_result() using 1
{
	static thread_data_t *thread_data;
	static system_call_result_t *result;
	
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