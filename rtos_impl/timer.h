#ifndef TIMER_H
#define TIMER_H

#include <reg51.h>

/**
* Initialisiert den Systemtimer.
*
* Initialisiert den Systemtimer, der für das Scheduling und die
* System-Calls verwendet wird.
*/
void initialize_system_timer(void);

/**
* Deaktiviert den Systemtimer.
*
* Erlaubt die atomare Ausführung von Anweisungen,
* bis der Timer erneut aktiviert wird.
*/
#define disable_system_timer() { ET0 = 0; }

/**
* Aktiviert den Systemtimer.
*
* Aktiviert den Systemtimer und beendet damit einen
* atomaren Block-.
*/
#define enable_system_timer() { ET0 = 1; }

/**
* Erzwingt einen Overflow des Systemtimers.
*
* Setzt das Overflow-Flag des Systemtimers manuell,
* wodurch nach Aktivieren des Timers der Interrupt-Handler
* betreten wird.
*/
#define trigger_system_timer_overflow() { TF0 = 1; }

#endif /* TIMER_H */