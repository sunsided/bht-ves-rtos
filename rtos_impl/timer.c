#include <reg51.h>
#include "timer.h"

void initialize_system_timer(void)
{
	TH0 = -250;												// set timer period            
	TL0 = -250;												// set reload value
	TMOD = TMOD | 0x02;									// select mode 2               
	TR0 = 1;													// start timer 0               
	ET0 = 1;													// enable timer 0 interrupt
	EA  = 1;													// global interrupt enable     
}
