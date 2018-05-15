/****************************************************************************
 Module
   TemplateService.c

 Revision
   1.0.1

 Description
   This is a template file for implementing a simple service under the 
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/16/12 09:58 jec      began conversion from TemplateFSM.c
 ****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
 */
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "PACSystemService.h"
#include "UART_TX_SM.h"
#include "SPI_SM.h"

/*----------------------------- Module Defines ----------------------------*/

#define FRAME_BYTE 0x7E
#define API_RX_DATA 0x81
#define API_TX_STAT 0x89
#define API_TX_REQ 0x01
#define VALID_CKSUM 0xFF

#define TX_STAT_SUCCESS 0 // TX status returned on message success
#define TX_STAT_NACK 1 // TX status returned on no acknowledge
#define TX_STAT_CCA 2// TX status returned on CCA failure
#define TX_STAT_PURGE 3 // TX status returned when ???

#define XBEE_TXPACKET_MAX_LENGTH 10

#define TX_TIMEOUT (ONE_SEC / 4)


#define superPACAddrSet() (((superPACAddr[0] | superPACAddr[1]) == 0) ? False : True)

#define retrySend() do{\
                        ES_Event LEDEvent;\
                        LEDEvent.EventType = ES_TOGGLE_LED;\
                        LEDEvent.EventParam = TX_ERR_TIMEOUT;\
                        FastPostPACSystemService(LEDEvent);\
                        index = 0;\
                        currentState = TX_SendMSG;\
                        TXIE = 1;\
                    }while(0)

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
 */

//static inline boolean superPACAddrSet(void);
//static inline void retrySend(void);

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

static UART_TX_State_t currentState;

static uint8_t XBeePacket[XBEE_TXPACKET_MAX_LENGTH]; //create an array to send
static volatile uint8_t length;
static volatile uint8_t index;

static uint8_t superPACAddr[2] = {0, 0};
static volatile boolean sendCompleteFlag = False;
static boolean TXError = False;

/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
 Function
     InitUARTService

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any 
     other required initialization for this service
 Notes

 Author
     J. Edward Carryer, 01/16/12, 10:00
 ****************************************************************************/
boolean InitUARTTXService(uint8_t Priority) {
    ES_Event ThisEvent;

    MyPriority = Priority;

    length = 0;
    index = 0;

    // post the initial transition event
    ThisEvent.EventType = ES_INIT;
    if (ES_PostToService(MyPriority, ThisEvent) == True) {
        return True;
    } else {
        return False;
    }
}

/****************************************************************************
 Function
     PostTemplateService

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
 ****************************************************************************/
boolean PostUARTTXService(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunTemplateService

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   
 Author
   J. Edward Carryer, 01/15/12, 15:23
 ****************************************************************************/
ES_Event RunUARTTXService(ES_Event ThisEvent) {
    ES_Event ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    switch (currentState) {
        case InitPTXState: // If current state is initial Psedudo State
            if (ThisEvent.EventType == ES_INIT)// only respond to ES_Init
            {
                currentState = TX_Idle;
            }
            break;
        case TX_Idle:
            if (ThisEvent.EventType == ES_TX_REQUEST_SEND) {
                // If we know who the SuperPAC is
                if (superPACAddrSet()) {
                    // Assemble the packet to send to the XBee
                    TXError = False;

                    TX_clearXBeePacket();
                    TX_appendXBeePacket(FRAME_BYTE);
                    TX_appendXBeePacket(0x00); // MSB MSG Len
                    TX_appendXBeePacket(0x06); // LSB MSG Len
                    TX_appendXBeePacket(API_TX_REQ);
                    TX_appendXBeePacket(0x01); // Frame ID, set to nonzero
                    TX_appendXBeePacket(superPACAddr[0]);
                    TX_appendXBeePacket(superPACAddr[1]);
                    TX_appendXBeePacket(0x00);
                    TX_appendXBeePacket((uint8_t) ThisEvent.EventParam);
                    TX_appendXBeePacket(TX_getChecksum());

                    // Start sending it
                    currentState = TX_SendMSG;
                    TXIE = 1;
                } else {
                    TXError = True;
                }
            } else if (ThisEvent.EventType == ES_TX_NEW_ADDR) {
                superPACAddr[0] = (ThisEvent.EventParam >> 8) & 0xFF;
                superPACAddr[1] = ThisEvent.EventParam & 0xFF;

                // Tell SPI module that we have established communications
                setRadioPing(True);
            }
            break;

        case TX_SendMSG:
            if (ThisEvent.EventType == ES_TX_SEND_COMPLETE) {
                currentState = TX_WaitStatus;
                ES_Timer_InitTimer(TX_TIMER, TX_TIMEOUT);
            }
            break;

        case TX_WaitStatus:
        {
            // include another if for the possibility of a TX send request while waiting for TX_STATUS
            if (ThisEvent.EventType == ES_TIMEOUT) {
                if (ThisEvent.EventParam == TX_TIMER) {
                    // We timed out, toggle the LED and try again
                    retrySend();
                }
            } else if (ThisEvent.EventType == ES_TX_STATUS) {
                switch (ThisEvent.EventParam) {
                    case TX_STAT_SUCCESS:
                        currentState = TX_Idle;
                        break;
                    case TX_STAT_NACK:
                    case TX_STAT_CCA:
                    case TX_STAT_PURGE: // Intentional fall through
                        retrySend();
                        break;
                    default:
                        break;
                }
            }
        }
            break;


    } // end switch on Current State

    // Emulate state entry events without hierarchical organization
    if (currentState == TX_Idle) {
        index = 0;
    }

    return ReturnEvent;
    c

/***************************************************************************
 Getters/Setters
 ***************************************************************************/

boolean TX_clearXBeePacket(void) {
    if (currentState != TX_Idle) {
        return False;
    } else {
        length = 0;
        return True;
    }
}

boolean TX_appendXBeePacket(uint8_t newByte) {
    if ((length == XBEE_TXPACKET_MAX_LENGTH) || (currentState != TX_Idle)) {
        return False;
    } else {
        XBeePacket[length++] = newByte;
        return True;
    }
}

uint8_t TX_getChecksum(void) {
    uint8_t chk = 0;
    uint8_t i;

    for (i = 3; i < length; i++) {
        chk += XBeePacket[i];
    }

    return 0xFF - chk;
}

uint8_t TX_getXBeePacket(uint8_t _index) {
    return XBeePacket[_index];
}

uint8_t TX_getLength(void) {
    return length;
}

uint8_t *TX_getSuperPACAddr(void){
    return &superPACAddr;
}

inline void UARTTXIntResponse(void) {
    if (index >= length) {
        sendCompleteFlag = True;
        TXIE = 0;
    } else {
        TXREG = XBeePacket[index++];
    }
}

boolean CheckUARTTXEvent(void) {
    if (sendCompleteFlag) {
        ES_Event TXEvent;
        TXEvent.EventType = ES_TX_SEND_COMPLETE;
        TXEvent.EventParam = 0;
        ES_PostToService(MyPriority, TXEvent);

        sendCompleteFlag = False;
        return True;
    }
    return False;
}

/***************************************************************************
 private functions
 ***************************************************************************/

/*
static inline boolean superPACAddrSet(void) {
    return (superPACAddr[0] | superPACAddr[1] == 0) ? False : True;
}
*/
/*
static inline void retrySend(void) {
    ES_Event LEDEvent;
    LEDEvent.EventType = ES_TOGGLE_LED;
    LEDEvent.EventParam = TX_ERR_TIMEOUT;
    FastPostPACSystemService(LEDEvent);

    // we control the buffer, so it's still set up
    index = 0; // reset the index
    currentState = TX_SendMSG;
    TXIE = 1; // re-enable interrupt to try again
}
*/

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

