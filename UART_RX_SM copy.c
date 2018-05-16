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
#include "UART_RX_SM.h"
#include "COMMDEFS.h"

/*----------------------------- Module Defines ----------------------------*/


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
 */
static void ClearReceivedDataArray(void);

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;

static UART_RX_State_t currentState;

static volatile boolean newResponseAvailable = False;

static volatile boolean flag2 = False;
static volatile boolean flag3 = False;

static volatile uint8_t ReceivedData [MSG_SIZE];
static volatile uint8_t StatusArray [MSG_SIZE];

static volatile boolean RXFlag = False;
static boolean hasHeartbeat = False;
static uint16_t FieldAddr = 0;


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

    switch (ThisEvent.EventType)
    {
        case ES_RX_NEW_PACKET: //if a new packet event has been posted from the event checker
            if (ReceivedData[API_INDENTIFIER_INDEX] == API_RX_DATA) 
            {
                //newStatusAvailable = True; //reactivate ready state
                if (ReceivedData[OPTIONS_INDEX] & BROADCAST_MASK)  // If we had received a broadcast message: used for status and score in 2018 field
                {
                    //newStatusAvailable = False; //deactivate ready state
                    //store the field address
                    uint8_t FieldAddressMSB = ReceivedData[OPTIONS_INDEX];
                    uint8_t FieldAddressLSB = ReceivedData[OPTIONS_INDEX];
                    //these bytes can be combined for field address
                    FieldAddr = (FieldAddressMSB<<8) | FieldAddressLSB;
                    
                    // Copy status into the status buffer
                    for (uint8_t i = 0; i < STAT_SIZE; i++)
                    {
                        StatusArray[i] = ReceivedData[DATA_INDEX + i];
                    }
                    
                    //newStatusAvailable = True; //reactivate ready state
                    
                    ES_Timer_InitTimer(HEARTBEAT_TIMER, ONE_SEC); // Restart heartbeat timer
                    hasHeartbeat = True; // Set heartbeat status

                } //end if
            }
            break;
        case ES_TIMEOUT: // if time out from heartbeat timer
            if (ThisEvent.EventParam == HEARTBEAT_TIMER)
            {
                hasHeartbeat = False;
            }
            break;
        default:
            break;
    }

    return ReturnEvent;
}

/****************************************************************************
 Function
    RX_hasHeartbeat

 Parameters
    none

 Returns
 boolean

 Description
   
 ****************************************************************************/
boolean RX_hasHeartbeat(void) {
    return hasHeartbeat;
}


/****************************************************************************
 Function
    RX_getStatusBuffer

 Parameters
    none

 Returns
    pointer to uint8_t array

 Description
   
 ****************************************************************************/
volatile uint8_t * RX_getStatusBuffer(void) {
    return &StatusArray;
}

/****************************************************************************
 Function
    RX_getResponseBuffer

 Parameters
    none

 Returns
    pointer to uint8_t array

 Description
   
 ****************************************************************************/
volatile uint8_t * RX_getResponseBuffer(void) {
    return &ReceivedData;
}

/****************************************************************************
 Function
    RX_getFieldAddr

 Parameters
    none

 Returns
 boolean

 Description
   
 ****************************************************************************/
uint16_t RX_getFieldAddr(void){
    return FieldAddr;
}
/****************************************************************************
 Function
    RX_getFieldAddr

 Parameters
    none

 Returns
 boolean

 Description
 used to return the status of test flag
 ****************************************************************************/
inline boolean getFlag2(void) 
{
    return flag2;
}

/****************************************************************************
 Function
    RX_getFieldAddr

 Parameters
    none

 Returns
 boolean

 Description
 used to return the status of test flag
 ****************************************************************************/
inline boolean getFlag3(void) 
{
    return flag3;
}
/****************************************************************************
 Function
    UARTRXIntResponse

 Parameters
    none

 Returns
    none

 Description
 respond to the incoming data based on state of state machine
 ****************************************************************************/
inline void UARTRXIntResponse(void) {
    static uint8_t currentByteIndex; //stores the index of byte being read in
    static uint8_t bodyByteCount; //total bytes in the body of the received message
    //static uint8_t checksum;
    
    uint8_t newByte = RCREG; // Read pending byte and clear interrupt
    
    switch (currentState) {
        case WaitFor7E: // state is wait for frame byte: 7E
            if (newByte == FRAME_BYTE)//if we receive a frame byte
            {
                ClearReceivedDataArray(); //clear the prev responses
                currentState = WaitForMSB; //change state to wait for MSB of length byte
            }
            break;

        case WaitForMSB: //if state is wait for MSB of the length byte
            if (newByte == MSB) // MSB is zero
            {
                currentState = WaitForLSB; //change state to wait for LSB of length byte
            }
            else // if we didn't get a 0 for MSB
            {
                currentState = WaitFor7E; // go back to wait for frame byte
            }
            break;

        case WaitForLSB: //if state is wait for LSB
            bodyByteCount = newByte; //set the body byte count to the new byte coming in
            if (bodyByteCount > MSG_SIZE)//if the body byte count > msg size, means too long
            { 
                currentState = WaitFor7E; //go back to wait for frame byte
            } 
            else //if we got a message with permissable size
            {
                currentByteIndex = 0; //reset current byte index
                checksum = 0; //reset checksum
                
                currentState = SuckUpData; //change states to SuckUpData
            }
            break;
            
        case SuckUpData: //if current state is SuckUpData
            if (currentByteIndex < MSG_SIZE) //if the byte index is equal or under the size(MSG_SIZE-1))
            {
                ReceivedData[currentByteIndex] = newByte; //add new byte to buffer
                ++currentByteIndex; //increment the byte index
                
            } 
            else if (currentByteIndex == bodyByteCount) //if the body index matches the byte count
            {
                currentByteIndex = 0; //reset byte index
                currentState = WaitForCkSum; //change state to wait for check sum
            }
            else//if the byte index and byte count don't match
            { 
                currentState = WaitFor7E; //go back to wait for frame byte
            }
            checksum += newByte; //add the new byte to the checksum
            break;
            
        case WaitForCkSum: //if the current state is wait for check sum
            if (VALID_CKSUM == checksum) //if the check sum is valid
            {
                RXFlag = True; //set the RX flag to true
            }
            currentState = WaitFor7E; //change state to wait for frame byte
            break;
        default:
            break;
    }//end switch
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
 ****************************************************************************/
boolean CheckUARTRXEvent(void) {
    if (RXFlag) //If the received flag has been set
    {
        //Post a New Packet Event to the run RX Service
        ES_Event CommEvent;
        CommEvent.EventType = ES_RX_NEW_PACKET;
        CommEvent.EventParam = 0;
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
static void ClearReceivedDataArray(void) {
    for (uint8_t i = 0; i < MSG_SIZE; i++)
    {
        ReceivedData[i] = 0; //clear
    }
}
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

