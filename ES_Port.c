#include <htc.h>
#include "ES_Port.h"
#include "SPI_SM.h"
#include "UART_RX_SM.h"
#include "UART_TX_SM.h"

static unsigned char TickCount;

/****************************************************************************
 Function
     ES_Timer_Init
 Parameters
     unsigned char Rate set to one of the TMR_RATE_XX values to set the
     RTI rate
 Returns
     None.
 Description
     Initializes the timer module by setting up the RTI with the requested
    rate
 Notes
     For PIC testing: a quick hack that fixes the rate at 10ms
 Author
     J. Edward Carryer, 02/24/97 14:23
 ****************************************************************************/
void ES_Sys_Timer_Init(void) {
    
    //db: PortC is an input reading the level, for this function use another pin
    /*
     *  RC0 = heartbeat output
     */
    //TRISC0 = 0; // Output
    //ANS4 = 0; // disable A/D on RC0 so we can read it
    //RC0 = 0;

    //db: RC5 is an input reading the level, for this function use another pin
    // RC5  = Interrupt Timing
    //TRISC5 = 0;
    //RC5 = 0; // Initialize low

    // Use timer 0 for system clock so that we can still use ECCP module.
    DisableInterrupts; // Disable interrupts to be sure for setup

    T0CS = 0; // Internal clock, Fosc/4 = 2MHz
    PSA = 0; // Assign prescaler to Timer 0
    PS0 = 0; // Prescale by 256.  This gives 13.11 ms/tick //111
    PS1 = 1;
    PS2 = 0;

    TMR0 = 0; // Clear Timer 0
    T0IF = 0; // Clear pending interrupt
    T0IE = 1; // Enable Timer 0 overflow interrupt

    EnableInterrupts; // re-enable global interrupts
}

void interrupt ISR(void) {
    //db: RC5 is an input reading the level, for this function use another pin
    //RC5 = 1; // Raise line for heartbeat / interrupt timing

    if (SSPIF && SSPIE) {
        //SPIIntResponse();
    }

    if (RCIF && RCIE) {
        UARTRXIntResponse();
    }

    // remove this block to remove transmitting functions
    if (TXIF && TXIE) {
        UARTTXIntResponse();
    }

    if (T0IF) {
        T0IF = 0; // clear Timer 0 flag
        TickCount++; // count number of timer ticks that need to be processed
        PORTC ^= BIT0HI;
    }
    //db: RC5 is an input reading the level, for this function use another pin
    //RC5 = 1; // Raise line for heartbeat / interrupt timing
}

unsigned char IsTimerPending(void) {
    if (TickCount > 0) {
        TickCount--;
        return (1);
    } else
        return (0);
}