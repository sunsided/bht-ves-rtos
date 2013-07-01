#ifndef IMPL__SLEEP_H
#define IMPL__SLEEP_H

#include "../rtos/datatypes.h"

/**
* Pausert den Thread für die angegebene Zeit.
*
* @param ms Die Zeit in Millisekunden.
*/
void os_sleep(const sleep_t ms);

/**
* Liefert die Systemzeit in Millisekunden seit Start.
*
* @returns Die Laufzeit.
*/
systime_t os_time();

#endif /* IMPL__SLEEP_H */