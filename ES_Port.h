#ifndef PORT_H
#define PORT_H

#include <htc.h>
#include "BITDEFS.H"
#include "Bin_Const.h"
#include "ES_Timers.h"

// One second, based on 13.11 ms/tick
#define ONE_SEC 76

#define EnableInterrupts {GIE = 1;}
#define DisableInterrupts {GIE = 0;}

// these macros provide the wrappers for critical regions, where ints will be off
// but the state of the interrupt enable prior to entry will be restored.

/* This macro saves INTCON register and disables global interrupts. */
#define EnterCritical(__temp)  do{ __temp = INTCON; GIE = 0;}while(0)
/* This macro restores INTCON register saved in EnterCritical(). */
#define ExitCritical(__temp)  {INTCON = __temp;}

// Prottypes for hardware specific functions

unsigned char IsTimerPending(void);
void ES_Sys_Timer_Init(void);

#endif
