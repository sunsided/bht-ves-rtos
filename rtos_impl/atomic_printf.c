#include <stdarg.h>
#include <stdio.h>
#include "../rtos/atomic_printf.h"
#include "timer.h"

void atomic_printf(const char *format, ...)
{
	va_list args;
	
	suppress_system_timer_int();
  
	va_start(args, format);
  printf(format, args);
  va_end(args);
	
	allow_system_timer_int();
}
