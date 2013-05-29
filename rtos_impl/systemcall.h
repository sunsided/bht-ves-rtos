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
* �bergabestruktur f�r system call.
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
* Diese Methode verl�sst den �ber os_begin_system_call() betretenen
* atomaren Bereich.
*/
void os_cancel_execute_system_call();

/**
* F�hrt den system call aus.
*
* Diese Methode verl�sst den �ber os_begin_system_call() betretenen
* atomaren Bereich.
*/
void os_execute_system_call();

/**
* Bezieht das Ergebnis des system calls.
*/
system_call_result_t* os_get_system_call_result();

/**
* Setzt das system call-Ergebnis zur�ck.
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
* Setzt den system call zur�ck.
*/
void kernel_clear_system_call();

/**
* Ermittelt, ob es sich um einen system call handelt.
*/
bool kernel_is_system_call();

#endif /* IMPL__SYSTEMCALL_H */