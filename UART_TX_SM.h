#ifndef UART_TX_SM_H
#define	UART_TX_SM_H


#include "ES_Framework.h"

// typedefs for the states
// State definitions for use with the query function

typedef enum {
    InitPTXState, TX_Idle, TX_SendMSG, TX_WaitStatus
} UART_TX_State_t;

typedef enum {
    TX_ERR_TIMEOUT,
    TX_ERR_CCA,
    TX_ERR_NO_ADDR
} UART_TX_Error_t;

// Public Function Prototypes

boolean InitUARTTXService(uint8_t Priority);
boolean PostUARTTXService(ES_Event ThisEvent);
ES_Event RunUARTTXService(ES_Event ThisEvent);
UART_TX_State_t QueryUARTTXService(void);

boolean CheckUARTTXEvent(void);
inline void UARTTXIntResponse(void);


#endif	/* UART_TX_SM_H */

