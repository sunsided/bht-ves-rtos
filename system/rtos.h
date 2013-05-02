#ifndef RTOS_H
#define RTOS_H

#include "datatypes.h"

void StartMT(void);                                
void RegisterThread(uint16_t thread, uint8_t nr);
void tinit(void);

#else /* RTOS_H */
#error rtos.h mehrfach inkludiert
#endif /* not RTOS_H */
