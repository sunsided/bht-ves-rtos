#include "../rtos/threads.h"

#define FIRST       (-1)
#define REGISTER_COUNT	(8)
#define MAX_THREADS  (5)						// maximale Anz. der verwaltbaren Threads
#define MAX_THREAD_STACKLENGTH    (0x20)				// maximale Stacktiefe eines Threads
																	// F�r �nderungen siehe ***.m51-File

typedef struct {								// Datentyp f�r den Thread Control Block
	uint8_t sp;
	uint8_t reg[REGISTER_COUNT];
} TCB;

