#include <reg51.h>
#include "v24.h"

void intialize_uart(void)
{
	SCON = 0x52;      		// Serial channel control register
	// 0x52 == 0101 0010b
	// SM0, SM1 = 0,1		serial mode 1: 8-bit UART, variable baud rate
	// SM2      = 0			no multiprocessor mode
	// REN			= 1			receiver enable
	// TB8			= 0			transmitter bit 8; unused (serial mode 1)
	// RB8			= 0			receiver bit 8; unused
	// TI				= 1			transmitter interrupt enabled
	// RI		    = 0			receiver interrupt disabled
	
	TMOD &= ~0xF0; 			// Bits für timer 1 clearen
	TMOD |= (0x02 << 4);		// Timer mode register
	// 0x20 = 0010 0000b
	// Gate			= 0			Gating control (timer enabled, wenn TR1 gesetzt)
	// C/T			= 0			Counter/timer select (0 = timer)
	// M1, M0   = 1,x		8-bit auto-reload timer/counter
  //									value in TH1									
	
	TCON = 0x69;      	// Timer control register
	// 0x69 == 0110 1001b
	// TF1 		  = 0			Timer 1 overflow flag (reset)
	// TR1      = 1			Timer 1 run control bit (enable)
	// TF0      = 1     Timer 0 overflow flag (set)
	// TR0			= 0			Timer 0 run control bit (disable)
	// IE1			= 1			Interrupt 1 edge flag
	// IT1			= 0			Interrupt 1 type control bit
	// IE0			= 0			Interrupt 0 edge flag
	// IT0			= 1			Interrupt 0 type control flag
	
	TH1 =  0xF3;      		// Timer 1, high byte
	// Wert, der bei Überlauf von Timer 1 nachzuladen ist
	// TH1 = 256 - ((Crystal / 384) / Baud)
	// http://www.8052.com/tutser.phtml
}