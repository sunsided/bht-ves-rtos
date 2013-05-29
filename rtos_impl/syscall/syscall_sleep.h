#ifndef IMPL__SYSCALL_SLEEP_H
#define IMPL__SYSCALL_SLEEP_H

#include "../../rtos/datatypes.h"

/**
* syscall_register_thread_t - sleep-Anforderung
*/
typedef struct {

	/**
	* Schlafdauer in ms.
	*/
	volatile sleep_t sleep_duration;
	
} syscall_sleep_t;


#endif /* IMPL__SYSCALL_SLEEP_H */