#ifndef TIMER_H
#define TIMER_H

#include <reg51.h>

/**
* Initialisiert den Systemtimer.
*
* Initialisiert den Systemtimer, der f�r das Scheduling und die
* System-Calls verwendet wird.
*/
void initialize_system_timer(void);

/**
* Startet den Systemtimer.
*
* Startet den Systemtimer, der f�r das Scheduling und die
* System-Calls verwendet wird.
*/
void start_system_timer(void);

/**
* L�dt den Systemtimer neu.
*/
void reload_system_timer(void);

/**
* Deaktiviert den Systemtimer-Interrupt.
*
* Erlaubt die atomare Ausf�hrung von Anweisungen,
* bis der Timer erneut aktiviert wird.
*/
#define suppress_system_timer_int() { ET0 = 0; }

/**
* Aktiviert den Systemtimer-Interrupt.
*
* Aktiviert den Systemtimer und beendet damit einen
* atomaren Block-.
*/
#define allow_system_timer_int() { ET0 = 1; }

/**
* Erzwingt einen Overflow des Systemtimers.
*
* Setzt das Overflow-Flag des Systemtimers manuell,
* wodurch nach Aktivieren des Timers der Interrupt-Handler
* betreten wird.
*/
#define trigger_system_timer_overflow() { TF0 = 1; }

#endif /* TIMER_H */