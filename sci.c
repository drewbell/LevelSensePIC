#include <htc.h>
#include "ES_Types.h"
#include "sci.h"

// Baudrate, assuming 8MHz & the register settings below
#define BAUD9_6K (207)
#define BAUD19_2K 103

// core code for sci(USART) communications on PIC 16F690

void USART_Init(void) {
    // Disable interrupts while we set up
    GIE = 0;

    // Set up timing
    SPBRG = 129; // Baud rate = 9600, assuming 20MHz Fosc
    SPBRGH = 0; // BR = Fosc(20M) / (16 * (SPBRG + 1)
    BRGH = 1; // BR = 20M/(16 * 130) = 9615
    BRG16 = 0;

    // Set up in 8 bit async mode
    SYNC = 0;
    SPEN = 1;

    // Disable TX interrupt, enable RX interrupt
    TXIE = 0;
    RCIE = 1;
    PEIE = 1;

    // enable transmission
    TXEN = 1;

    // enable reception
    CREN = 1;
    ANS11 = 0;

    // Enable interrupts
    GIE = 1;

}

boolean USART_IsTxOK(void) {
    if (TXIF == 1)
        return True;
    else
        return False;
}

boolean USART_IsRxReady(void) {
    if (RCIF == 1)
        return (1);
    else
        return ( 0);
}

boolean USART_SendByte(unsigned char Byte2Send) {
    if (TXIF == 1) {
        TXREG = Byte2Send;
        return True;
    } else
        return False;
}

unsigned char USART_GetRxByte(void) {
    return RCREG;
}