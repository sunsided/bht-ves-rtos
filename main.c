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
#include <reg515c.h>
#include "rtos/rtos.h"

static bool zeitausgabe_aktiviert = false;

#define BUFFER_SIZE 42U
static volatile uint32_t shared_mem[BUFFER_SIZE];

/**
* Semaphor für den Systemzeit-Thread.
*/
static semaphore_t timer_semaphore;

/**
* Test-Semaphore
*/
static semaphore_t wert_da;
static semaphore_t platz_da;

// Alle Threads laufen in Registerbank 0
void tick_system_time(void)
{
	while(1) {
		os_sleep(1000);
		os_semaphore_post(&timer_semaphore);
	}
}

void print_system_time(void)
{
	static systime_t time;

	while(1) {
		os_semaphore_wait(&timer_semaphore);
		
		time = os_time();
		printf("system time: %.3f s\n", time/1000.0F);
	}
}

void erzeuger(void)
{
	static uint32_t wert = 0;
	static uint8_t write_idx = 0;

	while(1) {
		os_semaphore_wait(&platz_da);

		shared_mem[write_idx] = wert++;
		write_idx = (++write_idx)%BUFFER_SIZE;

		if((wert%10)==0)
		{
			P5 |= 0x01; //Pin0 an
			os_sleep(20);
			P5 &= ~(0x01);
		}

		os_semaphore_post(&wert_da);
	}
}

void verbraucher(void)
{
	static uint8_t read_idx = 0;
	static uint32_t vergleichswert = 0;
	static uint32_t wert;	

	while(1) {
		os_semaphore_wait(&wert_da);

		wert = shared_mem[read_idx];	
		read_idx = (++read_idx)%BUFFER_SIZE;

		 // werte müssten synchron sein
		//assert(wert == vergleichswert);
		if(wert != vergleichswert)
		{
			P5 |= 0x02;
		}
		vergleichswert++;

		os_semaphore_post(&platz_da);
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
		
	if (zeitausgabe_aktiviert) 
	{
		thread = os_register_thread(tick_system_time, PRIO_HIGHEST, "System Tick");
		ASSERT_THREAD_REGISTERED(thread);
		printf("tick_system_time registriert als ID %u.\r\n", (uint16_t)thread); // NOTE: Cast ist ein Fix für endianness
		
		thread = os_register_thread(print_system_time, PRIO_NORMAL, "Uhrzeit");
		ASSERT_THREAD_REGISTERED(thread);
		printf("print_system_time registriert als ID %u.\r\n", (uint16_t)thread);

	}

	thread = os_register_thread(erzeuger, PRIO_HIGH, "Erzeuger-Thread");
	ASSERT_THREAD_REGISTERED(thread);
	printf("erzeuger registriert als ID %u.\r\n", (uint16_t)thread);
	
	thread = os_register_thread(verbraucher, PRIO_NORMAL, "Verbraucher-Thread");
	ASSERT_THREAD_REGISTERED(thread);
	printf("verbraucher registriert als ID %u.\r\n", (uint16_t)thread);
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
	
	result = os_semaphore_init(&timer_semaphore, 0);
	ASSERT_SEMAPHORE_INITIALIZED(result, timer_semaphore);
	printf("Timer-Semaphor initialisiert als ID %u.\r\n", (uint16_t)timer_semaphore.semaphore_id);
	
	result = os_semaphore_init(&wert_da, 00);
	ASSERT_SEMAPHORE_INITIALIZED(result, wert_da);
	printf("Semaphor \"wert_da\" initialisiert als ID %u.\r\n", (uint16_t)wert_da.semaphore_id);
	
	result = os_semaphore_init(&platz_da, BUFFER_SIZE);
	ASSERT_SEMAPHORE_INITIALIZED(result, platz_da);
	printf("Semaphor \"platz_da\" initialisiert als ID %u.\r\n", (uint16_t)platz_da.semaphore_id);
}

void main(void) {

	P5 = 0x0;

	os_init();
	register_threads();
	initialize_semaphores();
	
	// Starten des Multithreading
	// Diese Funktion terminiert nie!
	os_start();							
}
