#ifndef DATATYPES_H
#define DATATYPES_H

typedef   signed char 		   int8_t;		// 8-bit    signed
typedef unsigned char 		  uint8_t;		// 8-bit  unsigned
typedef  signed short int 	 int16_t;		// 16-bit   signed
typedef unsigned short int 	uint16_t;		// 16-bit unsigned
typedef unsigned long int 	uint32_t;   // 32-bit unsigned

/*
* Typ für eine Schlafdauer.
*/
typedef int16_t sleep_t;

/**
* Typ für die eine Zeitangabe.
*/
typedef uint32_t systime_t;

/**
* Fehlercode
*/
typedef int8_t error_t;

/**
* Boolean
*/
typedef enum {
	true = 1,
	false = 0
} bool;

#endif /* DATATYPES_H */