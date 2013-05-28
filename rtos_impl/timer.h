#ifndef IMPL__TIMER_H
#define IMPL__TIMER_H

#include <reg51.h>

/**
* Initialisiert den Systemtimer.
*
* Initialisiert den Systemtimer, der für das Scheduling und die
* System-Calls verwendet wird.
*/
void initialize_system_timer(void);

/**
* Startet den Systemtimer.
*
* Startet den Systemtimer, der für das Scheduling und die
* System-Calls verwendet wird.
*/
void start_system_timer(void);

/**
* Lädt den Systemtimer neu.
*/
void reload_system_timer(void);

/**
* Deaktiviert den Systemtimer-Interrupt.
*
* Erlaubt die atomare Ausführung von Anweisungen,
* bis der Timer erneut aktiviert wird.
*/
void suppress_system_timer_int();

/**
* Aktiviert den Systemtimer-Interrupt.
*
* Aktiviert den Systemtimer und beendet damit einen
* atomaren Block.
*/
void allow_system_timer_int();

/**
* Erzwingt einen Overflow des Systemtimers.
*
* Setzt das Overflow-Flag des Systemtimers manuell,
* wodurch nach Aktivieren des Timers der Interrupt-Handler
* betreten wird.
*/
void trigger_system_timer_overflow();

#endif /* IMPL__TIMER_H */