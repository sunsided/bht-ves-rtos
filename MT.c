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
#include "system/rtos.h"

// Alle Threads laufen in Registerbank 0
void thread0(void)
{
	static int i = 0;

	while(1) {
		i++;    
	}
}


void thread1(void)
{
	static int i = 0;

	while(1) {
		i++;
		printf("i = %d\n", i);
	}
}


void thread2(void)
{
	static int i = 0;
	
	while(1) {
		i++;
	}
}


void thread3(void)
{
	static int i = 0;
	
	while(1) {
		i++;
	}
}


void thread4(void)
{
	int i = 0;
	
	while(1) {
		i++;
	}
}

void main(void) {
	initOS();
	
	assert(0 == registerThread(thread0)); // so entwerfen, dass ID oder Fehler zurückgeben wird
	assert(1 == registerThread(thread1));
	assert(2 == registerThread(thread2));
	assert(3 == registerThread(thread3));
	assert(4 == registerThread(thread4));
	
	startOS();							// Starten des Multithreading
											// Diese Funktion terminiert nie!
}
