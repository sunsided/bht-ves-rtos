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
#include <reg51.h>

#define TRUE        1
#define FIRST       -1
#define MAXTHREADS  5						// maximale Anz. der verwaltbaren Threads
#define POSRB0      0x00					// Position von Reg.Bank 0 im int. RAM
#define SLICE       4*100					// Timeslice
#define STACKLEN    0x20					// maximale Stacktiefe eines Threads
													// Für Änderungen siehe ***.m51-File

typedef struct {								// Datentyp für den Thread Control Block
	unsigned char sp;
	unsigned char r0;
	unsigned char r1;
	unsigned char r2;
	unsigned char r3;
	unsigned char r4;
	unsigned char r5;
	unsigned char r6;
	unsigned char r7;
} TCB;

																	//Threadstacks 
unsigned char idata Stack[MAXTHREADS][STACKLEN] _at_ 0x30;   
TCB xdata tcb[MAXTHREADS];									//Thread Cntrl. Bl.
unsigned char NrThreads = 0;								//Anzahl registr.
																	//Threads

void StartMT(void);                                
void RegisterThread(unsigned short thread, unsigned char nr);
void tinit(void);
void V24Init(void);

void thread0(void);
void thread1(void);
void thread2(void);
void thread3(void);
void thread4(void);



/*****************************************************************************
*                  Starten des MT-Betriebs (wird nie beendet!)               *
*****************************************************************************/
void StartMT(void)
{
	tinit();

	while (TRUE);
}



/*****************************************************************************
*              Eintragen eines Threads in die Verwaltungsstrukturen          *
*****************************************************************************/
void RegisterThread(unsigned short thread, unsigned char nr)
{
	if (NrThreads == MAXTHREADS)
		return;

	NrThreads++;
														// SP erstmals auf die nachfolgend
														// abgelegte Rücksprungadresse
														// + 5 byte für 5 PUSHes
	tcb[nr].sp  = (unsigned char)(&Stack[nr][0] + 6);          

	Stack[nr][0] = (thread & 0x00ffU);				// Startadresse des registrierten
	Stack[nr][1] = ((thread & 0xff00U) >> 8);	// Threads als Rücksprungadresse
}



/*****************************************************************************
*                    Timer 0 interrupt service function                      *
*                                                                            *
*   Hierin ist die Thread-Umschaltung als Kern des Multi-Threading Betriebs  *
*                                  enthalten.                                *
*****************************************************************************/
timer0() interrupt 1 using 1						// Int Vector at 000BH, Reg Bank 1  
{
	static unsigned short intcycle = 0;			// Zähler für die Anz. der Interr.
	static unsigned char idata * pi;				// Pointer in das interne RAM
	static unsigned char idata *pd = POSRB0;	// Pointer auf die Registerbank 0
	static unsigned char CurrentThread = 0;	// Nr des laufenden Threads
	static unsigned char NewThread = FIRST;	// Nr des naechsten Threads
															// Am Anfang ist NewThread auf 
															// einen erkennbar nicht gültigen
															// Wert gesetzt (Grund: s. 
															// "if (NewThread == FIRST)").

	if (NrThreads > 0) {								// Sind Threads zu verwalten?
		if (++intcycle == SLICE)  {
			intcycle = 0;
			NewThread = (CurrentThread + 1)%NrThreads;	// Threadumschaltung
		}  
		pi = (unsigned char idata *)SP;			// Kopie des Stackpointers

		if (NewThread != CurrentThread) {		// Nur bei Threadwechsel müssen
															// die Register gerettet werden!
         
			if (NewThread == FIRST)					// Beim allerersten Aufruf von                
				NewThread = 0;							// timer0 liegt der SP noch
															// im ursprünglichen Bereich
															// nach Systemstart. Er darf
															// nicht gerettet werden! Der
															// bei RegisterThread(...)
															// initialisierte Wert wird
															// verwendet!
			else {
				tcb[CurrentThread].sp  =  pi;			// Sichern des SP
			}               

			tcb[CurrentThread].r0  = *(pd + 0);		// Retten von R0-R7 aus der
			tcb[CurrentThread].r1  = *(pd + 1);		// von allen Threads gemeinsam
			tcb[CurrentThread].r2  = *(pd + 2);		// genutzten Registerbank 0
			tcb[CurrentThread].r3  = *(pd + 3);    
			tcb[CurrentThread].r4  = *(pd + 4);    
			tcb[CurrentThread].r5  = *(pd + 5);    
			tcb[CurrentThread].r6  = *(pd + 6);
			tcb[CurrentThread].r7  = *(pd + 7);
			
			SP = tcb[NewThread].sp;						// geretteten SP des Threads
			pi = (unsigned char idata *)SP;			// in Pointer pi laden
			
			*(pd + 0) = tcb[NewThread].r0;			// Wiederherstellen von R0-R7
			*(pd + 1) = tcb[NewThread].r1;			// in Registerbank 0
			*(pd + 2) = tcb[NewThread].r2;
			*(pd + 3) = tcb[NewThread].r3;
			*(pd + 4) = tcb[NewThread].r4;
			*(pd + 5) = tcb[NewThread].r5;
			*(pd + 6) = tcb[NewThread].r6;
			*(pd + 7) = tcb[NewThread].r7;
		
			CurrentThread = NewThread;					// Ab jetzt ist der neue Thread
		}                                         // der aktuelle!
	}    
}



/****************************************************************************/
/*                         setup timer 0 interrupt                          */
/****************************************************************************/
void tinit(void)  
{
	TH0 = -250;												// set timer period            
	TL0 = -250;												// set reload value
	TMOD = TMOD | 0x02;									// select mode 2               
	TR0 = 1;													// start timer 0               
	ET0 = 1;													// enable timer 0 interrupt    
	EA  = 1;													// global interrupt enable     
}


/****************************************************************************/
/*                      INITIALIZE SERIAL INTERFACE                         */
/****************************************************************************/
void V24Init(void)
{
	SCON = 0x52;      		// SCON 
	TMOD = TMOD | 0x20;		// TMOD 
	TCON = 0x69;      		// TCON 
	TH1 =  0xf3;      		// TH1 
}



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
