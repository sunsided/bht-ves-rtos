# Erstellung eines einfachen Multi-Threading-Betriebssystems mit Echtzeit-Scheduling auf 8051-Architektur

Beuth-Hochschule Berlin, Technische Informatik / Embedded Systems Master
Vertiefung Echtzeitsysteme (<a href="http://prof.beuth-hochschule.de/buchholz/">Prof. Dr.-Ing. Bernhard Buchholz</a>)

## Umgesetztes System

+ Echtzeit-RTOS mit Prioritätsscheduling (ohne Prioritätsvererbung) und Round Robin bei identischen Prioritäten.

## Target

+ Keil µVision Simulator
+ Infineon C515C (Speichermodell Large)

## Implementierung

### Funktionsnamen / Registerbänke

+ `os_foo()`: User-space-Aufruf in Registerbank 0
+ `kernel_foo()`: Kernel-space-Aufruf in Registerbank 1

Der Quellcode ist in zwei Klassen von Funktionen geteilt. Funktionen beginnend mit `os_` laufen (ausschließlich)
im User Space unter Verwendung der Registerbank `0`. Nur diejenigen Funktionen und Datentypen, welche in den Headerdateien im `rtos`-Ordner definiert sind, dürfen vom Benutzercode verwendet werden.

Funktionen beginnend mit `kernel_` werden ausschließlich vom Kernel aufgerufen und verwenden Registerbank `1`. Einige der o.g. `os_`-Methoden werden ebenfalls vom Kernelcode (im User Space!) verwendet, z.B. während der Initialisierung. Solche Methoden betreffen etwa das Betreten von Verlassen von atomaren Bereichen, die Initiierung von System Calls (etwa bei Registrierung von Threads und Semaphoren, etc.)

### Öffentliches API des Betriebssystems

Das öffentliche API des Sytems wird über den Header `rtos/rtos.h` bereitgestellt, welcher die anderen funktionsspezifischen Header inkludiert. 

#### Initialisierung / Startup

+ `os_init()` initialisiert das Betriebssystem
+ `os_start()` startet das Betriebssystem

Die Funktion `os_init()` muss einmal vor dem ersten Aufruf an das Betriebssystem ausgeführt werden. In dieser Funktion werden die Timer und die Threadverarbeitung initialisiert.

Mittels `os_start()` wird das Betriebssystem (d.h. die Threadverarbeitung) gestartet; Diese Funktion wird niemals verlassen. Sie sollte daher der letzte Aufruf im user code (`main()`) sein.

#### System calls: Threads

+ `os_register_thread(...)` registriert einen Thread

Mittels der Funktion `os_register_thread()` können Threads registriert werden. Die Anzahl der registrierbaren Threads ist durch die Hardware und den Kernel begrenzt (derzeit `5`). Die Funktion erwartet die Parameter

+ `const thread_function_t* thread`, einen Funktionszeiger auf die Threadfunktion (deren Logik als Endlosschleife implementiert sein muss.)
+ `thread_priority_t priority`, die Priorität des Threads, z.B. `PRIO_NORMAL`
+ `const unsigned char *threadname`, den Namen des Threads aus pur akademischen Gründen

#### System calls: Semaphore

+ `os_semaphore_init(...)` initialisiert einen Semaphor
+ `os_semaphore_post(...)` ist die V-Operation des Semaphors; Gibt den Semaphor frei
+ `os_semaphore_wait(...)` ist die P-Operation des Semaphors; Versucht, den Semaphor zu beziehen 

Die Funktion `os_semaphore_init(...)` erwartet einen Zeiger auf eine zu initialisierenden `semaphore_t`-Struktur, sowie den initialen Wert des Semaphors, `sem_size_t size`.

Die Funktionen `os_semaphore_post(...)` und `os_semaphore_wait(...)` erwarten jeweils einen Zeiger auf die zu modifizierende Semaphor-Struktur.

Alle Funktionen liefern als Rückgabewert einen `sem_error_t`, welcher den Erfolgs- oder Fehlerfall codiert.

#### System calls: Zeit

+ `os_sleep(...)` schläft für die gegebene Anzahl Millisekunden
+ `os_time()` liefert die aktuelle Systemzeit in Millisekunden

Mittels der Funktion `os_sleep(...)` kann die Ausführung des Threads für eine gegebene Anzahl Millisekunden `sleep_t ms` unterbrochen werden. Dieser Wert ist als garantierte Mindestzeit zu verstehen.

Soll die aktuelle Systemzeit bezogen werden, kann die Funktion `os_time()` verwendet werden.

### Implementierung der system calls im Kernel

#### Atomare Bereiche

Das Betreten und Verlassen von atomaren Bereichen wird über die Timer-Methoden, bereitgestellt im Kernelheader `rtos_impl/timer.h`, realisiert. Die hierfür verwendeten Methoden sind:

+ `os_suppress_system_timer_int()` maskiert den overflow-Interrupt des Timers 0, wodurch der atomare Bereich betreten wird
+ `os_allow_system_timer_int()` demaskiert den overflow-Interrrupt des Timers 0, wodurch der atomare Bereich verlassen wird
+ `os_trigger_system_timer_overflow()` setzt das overflow-Bit des Timers 0, wodurch ein software interrupt ausgeführt wird

#### Ausführung von system calls

Die system call-Funktionalität wird über den Kernelheader `rtos_impl/systemcall.h` bereitgestellt. System calls werden prinzipiell durch das Befüllen der globalen system call-Struktur (Typ `system_call_t`), sowie einen software interrupt (realisiert durch Setzen des Overflow-Bits in timer 0) durchgeführt.

##### Initiierung und Durchführung eines system calls

+ `system_call_t`
+ `os_begin_system_call(...)`
+ `os_execute_system_call()`
+ `os_cancel_execute_system_call()`

Mittels `os_begin_system_call(...)` wird ein atomarer Bereich betreten und der Zeiger auf die globale system call-Struktur vom Typ  `system_call_t` bezogen. Die Funktion erwartet einen Parameter vom Typ `system_call_type` - der Art des system calls - mit welchem die Struktur initialisiert wird.

Die `system_call_t`-Struktur besitzt ein union-Feld `call_data`, welches Spezialisierungen der system call-Daten beinhaltet. Die Definitionen dieser (etwa `syscall_register_thread_t`, `syscall_modify_semaphore_t`) befinden sich im Ordner `rtos_impl/syscall`.

Sind die Parameter des system calls gesetzt, kann dieser mittels `os_execute_system_call()` ausgeführt werden. Hierbei wird ein software interrupt ausgelöst und der atomare Bereich verlassen.
Da nur eine globale Instanz der `system_call_t`-Struktur existiert, erwartet diese Funktion keinen Parameter.

Soll ein system call regulär *nicht* ausgeführt werden, kann die Funktion `os_cancel_execute_system_call()` verwendet werden, welche den Typ des aktuellen system calls auf `NO_SYSTEM_CALL` zurücksetzt und den atomaren Bereich ohne Auslösen eines software interrupts verlässt. 
Dieses Verhalten wird von den Funktionen  `os_semaphore_post(...)` und `os_semaphore_wait(...)` verwendet, bei welchen eine atomare Ausführung benötigt, das tatsächliche Ausführen des system calls jedoch nur in Sonderfällen nötig ist.

##### Auswertung eines system calls

+ `system_call_result_t`
+ `os_get_system_call_result()`
+ `os_clear_system_call_result()`

Der TCB jedes Threads besitzt einen lokalen Speicher vom Typ `thread_data_t`, welcher ein Feld `system_call_result_t syscall_result` beinhaltet. Dieses beinhaltet die Art und das Ergebnis des zuletzt ausgeführten system calls. Wie auch in `system_call_t` sind hier Spezialisierungen der system calls über ein union-Feld realisiert.

Mittels der Funktion `os_get_system_call_result()` kann ein Zeiger auf das system call-Ergebnis des aktuell laufenden Threads bezogen werden. Die Funktion `os_clear_system_call_result()` setzt das Ergebnis zurück auf einen leeren Zustand (`NO_SYSTEM_CALL`).