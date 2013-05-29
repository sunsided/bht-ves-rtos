/*****************************************************************************
*                                                                            *
*      Beispiel für einen minimalen Multithreading-Betrieb (MT-Betrieb)      *
*                      auf einem Prozessor der 8051-Familie                  *
*                                                                            *
*  to do: siehe Aufgabenblatt zur 1. Aufgabe                                 *
*                                                                            *
*****************************************************************************/
 
/*****************************************************************************
*                                                                            *
*                              A C H T U N G                                 *
*                                                                            *
*    - Unbedingt in µVision unter "Options > BL51 > Size/Location"           *
*      RamSize auf 256 setzen!                                               *
*                                                                            *
*    - "Lokale" Variablen in einem Thread oder einer von ihm aufgerufenen    *
*      Funktion als static definieren!                                       *
*                                                                            *
*    - Verwenden Sie für Funktionen, die aus der Int-Funktion, bzw. einer    *
*      Thread-Funktion aufgerufen werden, mit "using n" immer die richtige   *
*      Registerbank                                                          *
*                                                                            *
*****************************************************************************/

#include <stdio.h>
#include <assert.h>
#include "rtos/rtos.h"

/**
* Test-Semaphore
*/
static semaphore_t test_semaphore[2];

// Alle Threads laufen in Registerbank 0
void thread0(void)
{
	static int i = 0;

	while(1) {
		i++; 
		os_sleep(1000);
		// os_semaphore_post(&test_semaphore[0]);
	}
}

void thread1(void)
{
	static int i = 0;
	static systime_t time;

	while(1) {
		os_semaphore_wait(&test_semaphore[0]);
		
		i++;
		time = os_time();
		
		printf("T1 i = %d, system time: %.3f s\n", i, time/1000.0F);
	}
}

void thread2(void)
{
	static int i = 0;
	
	while(1) {
		i++;
		os_sleep(0);
	}
}

void thread3(void)
{
	static int i = 0;
	
	while(1) {
		i++;
		os_sleep(100);
	}
}

/**
* Sichert über eine Assertion, dass der Thread registriert wurde.
*/
#define ASSERT_THREAD_REGISTERED(threadno) \
	assert(THREAD_REGISTER_ERROR != (threadno))

/**
* Registriert die user threads.
*/
void register_threads() {
	threadno_t thread;
		
	thread = os_register_thread(thread0, PRIO_HIGH, "Erster Thread");
	ASSERT_THREAD_REGISTERED(thread);
	printf("thread0 registriert als ID %u.\r\n", (uint16_t)thread); // NOTE: Cast ist ein Fix für endianness
	
	thread = os_register_thread(thread1, PRIO_HIGH, "Zweiter Thread");
	ASSERT_THREAD_REGISTERED(thread);
	printf("thread1 registriert als ID %u.\r\n", (uint16_t)thread);
	
	thread = os_register_thread(thread2, PRIO_LOW, "Dritter Thread");
	ASSERT_THREAD_REGISTERED(thread);
	printf("thread2 registriert als ID %u.\r\n", (uint16_t)thread);
	
	thread = os_register_thread(thread3, PRIO_NORMAL, "Vierter Thread");
	ASSERT_THREAD_REGISTERED(thread);
	printf("thread3 registriert als ID %u.\r\n", (uint16_t)thread);
}

/**
* Sichert über eine Assertion, dass der Thread registriert wurde.
*/
#define ASSERT_SEMAPHORE_INITIALIZED(sem_result, sem) \
	assert(SEM_SUCCESS == (sem_result)); \
	assert(0xFF > ((sem).semaphore_id)); \

/**
* Initialisiert die Semaphore
*/
void initialize_semaphores() {
	sem_error_t result;
		
	result = os_semaphore_init(&test_semaphore[0], 10);
	ASSERT_SEMAPHORE_INITIALIZED(result, test_semaphore[0]);
	printf("Test-Semaphor 1 initialisiert als ID %u.\r\n", (uint16_t)test_semaphore[0].semaphore_id);
	
	result = os_semaphore_init(&test_semaphore[1], 42);
	ASSERT_SEMAPHORE_INITIALIZED(result, test_semaphore[1]);
	printf("Test-Semaphor 2 initialisiert als ID %u.\r\n", (uint16_t)test_semaphore[1].semaphore_id);
}

void main(void) {
	os_init();
	register_threads();
	initialize_semaphores();
	
	// Starten des Multithreading
	// Diese Funktion terminiert nie!
	os_start();							
}
