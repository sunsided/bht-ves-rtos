/*****************************************************************************
*                                                                            
* Beispiel für einen minimalen Multithreading-Betrieb (MT-Betrieb) 
* auf einem Prozessor der 8051-Familie.
*                                                                            
* Markus Mayer   (Matr-Nr. XXXXXX)			                                 
* Patrick Kaiser (Matr-Nr. YYYYYY)			                                 
*
* Diese Datei enthält die Definitionen für die Timer-Operationen im
* Kernel space.
*                                                                            
*****************************************************************************/

#ifndef IMPL__TIMER_H
#define IMPL__TIMER_H

/**
* Initialisiert den Systemtimer.
*
* Initialisiert den Systemtimer, der für das Scheduling und die
* System-Calls verwendet wird.
*/
void os_initialize_system_timer(void);

/**
* Startet den Systemtimer.
*
* Startet den Systemtimer, der für das Scheduling und die
* System-Calls verwendet wird.
*/
void os_start_system_timer(void);

/**
* Lädt den Systemtimer neu.
*/
void kernel_reload_system_timer(void);

/**
* Deaktiviert den Systemtimer-Interrupt.
*
* Erlaubt die atomare Ausführung von Anweisungen,
* bis der Timer erneut aktiviert wird.
*/
void os_suppress_system_timer_int();

/**
* Aktiviert den Systemtimer-Interrupt.
*
* Aktiviert den Systemtimer und beendet damit einen
* atomaren Block.
*/
void os_allow_system_timer_int();

/**
* Erzwingt einen Overflow des Systemtimers.
*
* Setzt das Overflow-Flag des Systemtimers manuell,
* wodurch nach Aktivieren des Timers der Interrupt-Handler
* betreten wird.
*/
void os_trigger_system_timer_overflow();

#endif /* IMPL__TIMER_H */