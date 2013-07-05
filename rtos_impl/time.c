/*****************************************************************************
*                                                                            
* Beispiel für einen minimalen Multithreading-Betrieb (MT-Betrieb) 
* auf einem Prozessor der 8051-Familie.
*                                                                            
* Markus Mayer   (Matr-Nr. XXXXXX)			                                 
* Patrick Kaiser (Matr-Nr. YYYYYY)			                                 
*
* Implementierung von user- und kernel space-Funktionen von Zeit-spezifischen
* Operationen (os_sleep, os_time).
*                                                                            
*****************************************************************************/

#include <assert.h>
#include "syscall/syscall_sleep.h"
#include "systemcall.h"
#include "timer.h"
#include "../rtos/rtos_time.h"

extern systime_t system_time;

/**
* Pausert den Thread für die angegebene Zeit.
*
* @param ms Die Zeit in Millisekunden.
*/
void os_sleep(const sleep_t ms)
{
	static system_call_t			*sc;
	static syscall_sleep_t		*calldata;
	
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
	static systime_t time; 
	
	os_suppress_system_timer_int(); // systime_t ist 32bit
	time = system_time;
	os_allow_system_timer_int();
	return time;
}