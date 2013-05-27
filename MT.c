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
	V24Init();
	
	RegisterThread(thread0, 0); // so entwerfen, dass ID oder Fehler zurückgeben wird
	RegisterThread(thread1, 1);
	RegisterThread(thread2, 2);
	RegisterThread(thread3, 3);
	RegisterThread(thread4, 4);
	
	StartMT();							// Starten des Multithreading
											// Diese Funktion terminiert nie!
}
