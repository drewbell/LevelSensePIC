#ifndef UART_RX_SM_H
#define	UART_RX_SM_H


#include "ES_Framework.h"

// typedefs for the states
// State definitions for use with the query function
uint8_t checksum;

typedef enum {
    WaitFor7E, WaitForMSB, WaitForLSB,
    SuckUpOverhead, SuckUpData, WaitForCkSum
} UART_RX_State_t;

// Public Function Prototypes

boolean InitUARTRXService(uint8_t Priority);
boolean PostUARTRXService(ES_Event ThisEvent);
ES_Event RunUARTRXService(ES_Event ThisEvent);


uint8_t RX_getResponse(void);

inline void UARTRXIntResponse(void);
boolean CheckUARTRXEvent(void);


#endif	/* UART_RX_SM_H */


