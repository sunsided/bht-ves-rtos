# Erstellung eines einfachen Multi-Threading-Betriebssystems mit Echtzeit-Scheduling auf 8051-Architektur

Beuth-Hochschule Berlin, Technische Informatik / Embedded Systems Master
Vertiefung Echtzeitsysteme (<a href="http://prof.beuth-hochschule.de/buchholz/">Prof. Dr.-Ing. Bernhard Buchholz</a>)

## Umgesetztes System

+ Echtzeit-RTOS mit Prioritätsscheduling (ohne Prioritätsvererbung) und Round Robin bei identischen Prioritäten.

## Target

+ Keil µVision Simulator
+ Infineon C515C (Speichermodell Large)

## Implementierung

### Nomenklatur / Besonderheiten

Der Quellcode ist in zwei Klassen von Funktionen geteilt. Funktionen beginnend mit `os_` laufen (ausschließlich)
im User Space unter Verwendung der Registerbank 0. Nur diejenigen Funktionen und Datentypen, welche in den Headerdateien
im `rtos`-Ordner definiert sind, dürfen vom Benutzercode verwendet werden.

Funktionen beginnend mit `kernel_` werden nur vom Kernel aufgerufen und verwenden Registerbank 1. Einige der o.g. `os_`-Methoden
werden ebenfalls vom Kernelcode (im User Space) verwendet, z.B. während der Initialisierung. Solche Methoden betreffen etwa
das Betreten von Verlassen von atomaren Bereichen, die Registrierung von Threads und Semaphoren, etc.


