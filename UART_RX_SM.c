/****************************************************************************
 Module
   UART_RX_SM.c

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
 * 02/02/18     hr       editing for b field '18
 ****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
 */
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "LevelSensor.h"
#include "UART_RX_SM.h"
#include "UART_TX_SM.h"
#include "COMMDEFS.h"

/*----------------------------- Module Defines ----------------------------*/


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
 */
static void ClearReceivedData(void);

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

static UART_RX_State_t currentState;
static volatile uint8_t ReceivedData;
static volatile boolean RXFlag = False;


/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
 Function
     InitUARTService

 Parameters
     uint8_t : the priority of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any 
     other required initialization for this service
 Notes

 Author
     J. Edward Carryer, 01/16/12, 10:00
 ****************************************************************************/
boolean InitUARTRXService(uint8_t Priority) {
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
     PostUARTRXService

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
boolean PostUARTRXService(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunUART_RXService

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
ES_Event RunUARTRXService(ES_Event ThisEvent)
{
    ES_Event ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    switch (ThisEvent.EventType){
        case ES_RX_NEW_PACKET: //if a new packet event has been posted from the event checker
            if(ThisEvent.EventParam == 0xAA){
                ES_Event UARTEvent; 
                UARTEvent.EventType = ES_FUEL_QUERY;
                FastPostLevelSensorService(UARTEvent);
            }
            break;
        default:
            break;
    }

    return ReturnEvent;
}


/****************************************************************************
 Function
    RX_getResponse

 Parameters
    none

 Returns
    uint8_t 

 Description
   
 ****************************************************************************/
uint8_t RX_getResponse(void) {
    return ReceivedData;
}

/****************************************************************************
 Function
    UARTRXIntResponse

 Parameters
    none

 Returns
    none

 Description
    respond to incoming data and error check
 ****************************************************************************/
inline void UARTRXIntResponse(void) {
    if(FERR){   // framing error
        uint8_t ignore = RCREG; // Read to get past frame error byte
    }
    else{
        ReceivedData = RCREG; // Read pending byte and clear interrupt
        RXFlag = True;

        if(OERR){           // if overrun, clear error by clearing the receiver
            CREN = 0;       // enable bit, then re-enable to receive more
            CREN = 1; 
        }
    }
}

/****************************************************************************
 Function
    CheckUARTRXEvent

 Parameters
    none

 Returns
   boolean

 Description
   event checker for the receive flag
   Note: the extra step of polling the RXFlag is here instead of posting directly 
         to the run function is because the latter causes stack overflow. 
 ****************************************************************************/
boolean CheckUARTRXEvent(void) {
    if (RXFlag) //If the received flag has been set
    {
        ES_Event CommEvent;
        CommEvent.EventType = ES_RX_NEW_PACKET;
        uint8_t numToSend = ReceivedData;
        CommEvent.EventParam = numToSend;
        FastPostUARTRXService(CommEvent); //Hard coded macro to reduce stack use. TODO: check to make sure priority matches with es configure

        RXFlag = False; //clear the received flag
        return True;
    }
    return False;
}


/***************************************************************************
 private functions
 ***************************************************************************/
/****************************************************************************
 Function
    ClearReceivedDataArray

 Parameters
    none

 Returns
 none

 Description
 clears the received data array
 ****************************************************************************/
/*
static void ClearReceivedData(void) {
        ReceivedData = 0; //clear
}
*/
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

