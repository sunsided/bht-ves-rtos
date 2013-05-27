#include <reg51.h>
#include "timer.h"
#include "system.h"

void initialize_system_timer(void)
{
	TMOD = (TMOD & 0xF0) | 0x02;	// timer mode 1: 16-bit timer/counter
	
	TH0 = -250;												// set timer period            
	TL0 = -250;												// set reload value
	
	ET0 = 1;													// enable timer 0 interrupt
	EA  = 1;													// global interrupt enable     
}

void start_system_timer(void) {
	TR0 = 1;													// start timer 0
}