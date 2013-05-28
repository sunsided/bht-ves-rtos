#ifndef IMPL__SYSTEMCALL_H
#define IMPL__SYSTEMCALL_H

#include "syscall/syscall_register_thread.h"

/**
* System call type.
*
* Definiert die Art des medizinischen Notfalls.
*/
typedef enum {
	NO_SYSTEM_CALL = 0,				//< Kein system call (default-Wert)
	REGISTER_THREAD = 1,			//< Thread-Registrierung wird angefordert
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
	} result_data;
	
} system_call_result_t;

/**
* Initiiert einen system call.
*/
system_call_t* begin_system_call(const system_call_type type);

/**
* Bezieht den aktuell laufenden system call.
*/
system_call_t* get_system_call();

/**
* Führt den system call aus.
*/
void execute_system_call();

/**
* Setzt den system call zurück.
*/
void clear_system_call();

/**
* Bezieht das Ergebnis des system calls.
*/
system_call_result_t* get_system_call_result();

/**
* Setzt das system call-Ergebnis zurück.
*/
void clear_system_call_result();

/**
* Ermittelt, ob es sich um einen system call handelt.
*/
bool is_system_call();

#endif /* IMPL__SYSTEMCALL_H */