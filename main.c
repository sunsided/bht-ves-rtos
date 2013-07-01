/*****************************************************************************
*                                                                            
* Beispiel f�r einen minimalen Multithreading-Betrieb (MT-Betrieb) 
* auf einem Prozessor der 8051-Familie.
*                                                                            
* Markus Mayer   (Matr-Nr. XXXXXX)			                                 
* Patrick Kaiser (Matr-Nr. YYYYYY)			                                 
*
* Diese Datei enth�lt die Implementierung des Programmcodes gem�� der
* Aufgabenstellung. Es werden Threads und Semaphore definiert und registriert,
* der Betriebssystemkernel initialisiert und die Verarbeitung gestartet.
*                                                                            
*****************************************************************************/

#include <stdio.h>
#include <assert.h>
#include <reg515c.h>
#include "rtos/rtos.h"

/**
* Aktiviert zwei zus�tzliche Threads zur Laufzeitausgabe.
*/
static bool zeitausgabe_aktiviert = false;

/**
* Gr��e des Ringpuffers, der Erzeuger- und Verbraucherthread verbindet.
*/
#define BUFFER_SIZE 42U

/**
* Der Ringpuffer f�r Erzeuger- und Verbraucher-Thread
*/
static volatile uint32_t shared_mem[BUFFER_SIZE];

/**
* Semaphor f�r den Systemzeit-Thread.
*/
static semaphore_t timer_semaphore;

/**
* Semaphor, der angibt, ob Werte im Ringpuffer zur Verf�gung stehen.
* Wird vom Erzeugerthread gesteuert und vom Verbraucherthread gelesen.
*/
static semaphore_t wert_da;

/**
* Semaphor, der angibt, ob Platz im Ringpuffer zur Verf�gung steht.
* Wird vom Erzeugerthread gelesen und vom Verbraucherthread gesteuert.
*/
static semaphore_t platz_da;

/**
* Exemplarischer Thread zur Zeitsynchronisation alle 1000ms.
* siehe auch: print_system_time()
*/
void tick_system_time(void)
{
	while(1) {
		os_sleep(1000);
		os_semaphore_post(&timer_semaphore);
	}
}

/**
* Exemplarischer Thread zur Zeitausgabe alle 1000ms.
* siehe auch: tick_system_time()
*/
void print_system_time(void)
{
	static systime_t time;

	while(1) {
		os_semaphore_wait(&timer_semaphore);
		
		time = os_time();
		printf("system time: %.3f s\n", time/1000.0F);
	}
}

/**
* Erzeuger-Thread, welcher kontinierlich steigende Werte in einen  
* Ringpuffer schreibt.
* Alle 10 Werte soll Pin P5.0 f�r 20ms auf HIGH gesetzt werden.
*/
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

/**
* Verbraucher-Thread, welcher aus dem Ringpuffer liest.
* Liegen nicht-kontinierlich steigende Werte vor, wird Pin P5.1 
* dauerhaft auf HIGH-Pegel gesetzt.
*/
void verbraucher(void)
{
	static uint8_t read_idx = 0;
	static uint32_t vergleichswert = 0;
	static uint32_t wert;	

	while(1) {
		os_semaphore_wait(&wert_da);

		wert = shared_mem[read_idx];	
		read_idx = (++read_idx)%BUFFER_SIZE;

		 // werte m�ssten synchron sein
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
* Sichert �ber eine Assertion, dass der Thread registriert wurde.
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
		printf("tick_system_time registriert als ID %u.\r\n", (uint16_t)thread); // NOTE: Cast ist ein Fix f�r endianness
		
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
* Sichert �ber eine Assertion, dass der Thread registriert wurde.
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

/**
* Main Entry Point
*/
void main(void) {

	// pulldown der Registerb�nke
	P5 = 0x0;

	// Betriebssystem und user-Logik initialisieren
	os_init();
	register_threads();
	initialize_semaphores();
	
	// Starten des Multithreading
	// Diese Funktion terminiert nie!
	os_start();							
}
