#ifndef IMPL__SYSTEMCALL_H
#define IMPL__SYSTEMCALL_H

#include "syscall/syscall_register_thread.h"
#include "syscall/syscall_sleep.h"
#include "syscall/syscall_semaphore.h"

/**
* System call type.
*
* Definiert die Art des medizinischen Notfalls.
*/
typedef enum {
	NO_SYSTEM_CALL = 0,				//< Kein system call (default-Wert)
	REGISTER_THREAD = 1,			//< Thread-Registrierung wird angefordert
	SLEEP = 2,								//< Sleep
	SEMAPHOR_INIT = 3,				//< Initialisierung eines Semaphors
	SEMAPHOR_POST = 4,				//< Post-Operation eines Semaphors
	SEMAPHOR_WAIT = 5,				//< Wait-Operation eines Semaphors
} system_call_type;

/**
* Übergabestruktur für system call.
*/
typedef struct {
	/**
	* Gibt die Art des system calls an
	*/
	system_call_type type;
	
	/**
	* System call-Zugriff
	*/
	union {
		/**
		* register_thread - Registrierung eines Threads
		*/
		syscall_register_thread_t register_thread;
		
		/**
		* sleep - Schlafanforderung
		*/
		syscall_sleep_t sleep;
		
		/**
		* semaphore_init - Initialisierung eines Semaphors
		*/
		syscall_init_semaphore_t semaphore_init;
		
		/**
		* semaphore_modification - Veränderung eines Semaphors
		*/
		syscall_modify_semaphore_t semaphore_modification;
	} call_data;
} system_call_t;

typedef struct {
	/**
	* Gibt die Art des system calls an
	*/
	system_call_type type;
	
	/**
	* System call-Ergebnisse
	*/
	union {
		/**
		* register_thread - Registrierung eines Threads
		*/
		syscall_register_thread_result_t register_thread;
		
		/**
		* semaphore - Ergebnis einer Semaphor-Operation
		*/
		syscall_semaphore_result_t semaphore;
	} result_data;
	
} system_call_result_t;

/**
* Initiiert einen system call.
*
* Diese Methode betritt implizit einen atomaren Bereich.
*
* @param type Der Typ des system calls
* @returns Die system call-Struktur
*/
system_call_t* os_begin_system_call(const system_call_type type);

/**
* Bricht den system call ab.
*
* Diese Methode verlässt den über os_begin_system_call() betretenen
* atomaren Bereich.
*/
void os_cancel_execute_system_call();

/**
* Führt den system call aus.
*
* Diese Methode verlässt den über os_begin_system_call() betretenen
* atomaren Bereich.
*/
void os_execute_system_call();

/**
* Bezieht das Ergebnis des system calls.
*/
system_call_result_t* os_get_system_call_result();

/**
* Setzt das system call-Ergebnis zurück.
*/
void os_clear_system_call_result();

/**
* Bezieht den aktuell laufenden system call.
*
* @returns Der aktuelle system call.
*/
system_call_t* kernel_get_system_call();

/**
* Bezieht das Ergebnis des system calls.
*
* @returns Das system call-Ergebnis des aktuellen Threads.
*/
system_call_result_t* kernel_get_system_call_result();

/**
* Setzt den system call zurück.
*/
void kernel_clear_system_call();

/**
* Ermittelt, ob es sich um einen system call handelt.
*/
bool kernel_is_system_call();

#endif /* IMPL__SYSTEMCALL_H */