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

boolean RX_hasHeartbeat(void);

inline boolean RX_newStatusReady(void);

inline boolean getFlag2(void);
inline boolean getFlag3(void);

volatile uint8_t * RX_getStatusBuffer(void);
volatile uint8_t * RX_getResponseBuffer(void);
uint16_t RX_getFieldAddr(void);

inline void UARTRXIntResponse(void);
boolean CheckUARTRXEvent(void);


#endif	/* UART_SM_H */


