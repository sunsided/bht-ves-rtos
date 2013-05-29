#include <assert.h>
#include "syscall/syscall_sleep.h"
#include "systemcall.h"
#include "timer.h"
#include "../rtos/time.h"

extern systime_t system_time;

/**
* Pausert den Thread für die angegebene Zeit.
*
* @param ms Die Zeit in Millisekunden.
*/
void os_sleep(const sleep_t ms)
{
	system_call_t							*sc;
	syscall_sleep_t					 *calldata;
	
	sc = os_begin_system_call(SLEEP);
	assert(SLEEP == sc->type);
	
	// TODO: assert, dass nur aus Thread aufgerufen wird.
	
	// Zeit setzen
	calldata = &sc->call_data.sleep;
	calldata->sleep_duration = ms;
	
	os_execute_system_call();
	os_clear_system_call_result();
}

/**
* Liefert die Systemzeit in Millisekunden seit Start.
*
* @returns Die Laufzeit.
*/
systime_t os_time()
{
	systime_t time; 
	
	os_suppress_system_timer_int();
	time = system_time;
	os_allow_system_timer_int();
	return time;
}