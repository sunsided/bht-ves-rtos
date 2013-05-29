#include <reg51.h>
#include "../rtos/datatypes.h"
#include "../rtos/rtos.h"
#include "timer.h"
#include "system.h"

/**
* Prescaler des Timers.
*
* "In the “timer” function (C/T = ‘0’) the register is incremented every machine cycle.
* Therefore the count rate is fOSC/6."
*/
#define OSC_PRESCALER	6U

/**
* Anzahl der Millisekunden pro Sekunde.
*/
#define MS_PER_SEC					1000U

/**
* Der Wert, mit dem der Timer geladen werden muss.
*/
											//  (0xFFFFU - (F_CPU / OSC_PRESCALER * TICK_DURATION_MS / MS_PER_SEC))
#define TIMER_LOAD_VALUE  (0xFFFFU - (F_CPU / MS_PER_SEC * TICK_DURATION_MS / OSC_PRESCALER))

/**
* Schwellwert für TH0, der steuert, wann der Timer neu geladen werden muss.
*
* Ist TH0 unterhalb des Schwellwertes während eines Overflow-Interrupts,
* müssem TH0 und TR0 neu gesetzt werden, da timer mode 1 keinen auto-reload
* unterstützt. Der Shift um 9 entspricht einem Shift der oberen Hälfte,
* geteilt durch 2.
*/
#define RELOAD_THRESHOLD ((TIMER_LOAD_VALUE & 0xFF00) >> 9)

void os_initialize_system_timer(void)
{
	TH0 = (uint8_t)((TIMER_LOAD_VALUE & 0xFF00) >> 8);
  TL0 = (uint8_t)(TIMER_LOAD_VALUE & 0x00FF);
	
 	TMOD &= ~0x0F; // Bits für timer 0 clearen
	TMOD |= 0x01;	 // timer mode 1: 16-bit timer/counter
								// NOTE: Der Timer ist nicht auto-reloading, daher Werte manuell setzen!
	
	ET0 = 1;													// enable timer 0 interrupt
	EA  = 1;													// global interrupt enable     
}

void kernel_reload_system_timer(void) using 1
{
	static uint16_t value;
	
	if (TH0 > RELOAD_THRESHOLD) return;
	
	// bereits abgelaufene Zeit kompensieren
	value = ((TH0 << 8) | TL0) + TIMER_LOAD_VALUE;
	TH0 = (uint8_t)((value & 0xFF00) >> 8);
  TL0 = (uint8_t)(value & 0x00FF);
}

void os_start_system_timer(void) 
{
	TR0 = 1;													// start timer 0
}

/**
* Deaktiviert den Systemtimer-Interrupt.
*
* Erlaubt die atomare Ausführung von Anweisungen,
* bis der Timer erneut aktiviert wird.
*/
void os_suppress_system_timer_int() 
{ 
	ET0 = 0; 
}

/**
* Aktiviert den Systemtimer-Interrupt.
*
* Aktiviert den Systemtimer und beendet damit einen
* atomaren Block.
*/
void os_allow_system_timer_int() 
{ 
	ET0 = 1; 
}

/**
* Erzwingt einen Overflow des Systemtimers.
*
* Setzt das Overflow-Flag des Systemtimers manuell,
* wodurch nach Aktivieren des Timers der Interrupt-Handler
* betreten wird.
*/
void os_trigger_system_timer_overflow() 
{ 
	TF0 = 1;
}
