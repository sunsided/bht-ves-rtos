#include "v24.h"
#include <reg51.h>

void V24Init(void)
{
	SCON = 0x52;      		// SCON 
	TMOD = TMOD | 0x20;		// TMOD 
	TCON = 0x69;      		// TCON 
	TH1 =  0xf3;      		// TH1 
}