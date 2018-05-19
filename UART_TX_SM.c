/****************************************************************************
 Module
   UART_TX_SM.c

 Revision
   1.0.1

 Description
   This is a UART TX module for the PIC16F690

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/16/12 09:58 jec      began conversion from TemplateFSM.c
 05/14/18 20:20 db       conversion from 218B REF/PAC/LOC to water level
 ****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
 */
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "PACSystemService.h"
#include "UART_TX_SM.h"

/*----------------------------- Module Defines ----------------------------*/

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


/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static UART_TX_State_t currentState;
static boolean TXError = False;
static volatile boolean TXFlag = False;

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
        case InitPTXState: // If current state is initial Pseudo State
            if (ThisEvent.EventType == ES_INIT)// only respond to ES_Init
            {
                currentState = TX_Idle;
            }
            break;
        case TX_Idle:
            if (ThisEvent.EventType == ES_TX_REQUEST_SEND) {
                // Start sending the data
                TXREG = ThisEvent.EventParam & 0xFF; 
                currentState = TX_SendMSG;
                TXIE = 1;
            }
            break;

        case TX_SendMSG:
            if (ThisEvent.EventType == ES_TX_SEND_COMPLETE) {
                currentState = TX_Idle;
            }
            break;
    } // end switch on Current State

    return ReturnEvent;
}

/***************************************************************************
 Getters/Setters
 ***************************************************************************/

inline void UARTTXIntResponse(void) {
    TXIE = 0;               // clear TX interrupt
    TXFlag = True;          // notify event checker of TX event
}

/****************************************************************************
 Function
    CheckUARTTXEvent

 Parameters
    none

 Returns
   boolean

 Description
   event checker for the transmit flag
   Note: the extra step of polling the TXFlag is here instead of posting directly 
         to the run function is because the latter causes stack overflow. 
 ****************************************************************************/
boolean CheckUARTTXEvent(void) {
    if (TXFlag) //If the received flag has been set
    {
        ES_Event CommEvent;
        CommEvent.EventType = ES_TX_SEND_COMPLETE;
        FastPostUARTTXService(CommEvent); //Hard coded macro to reduce stack use. TODO: check to make sure priority matches with es configure
        TXFlag = False; //clear the received flag
        return True;
    }
    return False;
}

/***************************************************************************
 private functions
 ***************************************************************************/


/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

