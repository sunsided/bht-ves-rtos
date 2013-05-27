#include "../rtos/threads.h"

#define FIRST       (-1)
#define MAX_THREADS  (5)						// maximale Anz. der verwaltbaren Threads
#define MAX_THREAD_STACKLENGTH    (0x20)				// maximale Stacktiefe eines Threads
																	// Für Änderungen siehe ***.m51-File

typedef struct {								// Datentyp für den Thread Control Block
	uint8_t sp;
	uint8_t r0;
	uint8_t r1;
	uint8_t r2;
	uint8_t r3;
	uint8_t r4;
	uint8_t r5;
	uint8_t r6;
	uint8_t r7;
} TCB;

