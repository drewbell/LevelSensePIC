/****************************************************************************
 Module
   LevelSensor.c

 Revision
   0.0.1

 Description
   This module reads the state of 8 pins corresponding to level readings on 
   discrete water level sensor. Bit 0 corresponds to the lowest level of water.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 05/07/18 20:58 db      converted from UART_TX code
 
 ****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
 */
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Timers.h"
#include "LevelSensor.h"
#include "UART_RX_SM.h"
#include "UART_TX_SM.h"
#include "COMMDEFS.h"

// legacy headers
#include <htc.h>
#include "sci.h"

/*----------------------------- Module Defines ----------------------------*/
#define FUEL_EMPTY 0
#define ONE_SECOND 1000   // 1s = 1000ms
#define HALF_SECOND 500  // ms
#define QTR_SEC 250       // ms
#define FUELED_BIT 0x08
#define NUM_FUEL_PADS 0x08
#define BAD_FUEL_QUERY_MSG (uint8_t) 0xAA
#define FUEL_QUERY_COMMAND 0xAA

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
 */
uint8_t get3GarbageBits(void);
uint8_t constructFuelByte(void);

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static uint8_t fuelLevel = 0;
static uint8_t rawFuelReading = 0;
static LevelSensorState_t CurrentState;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitLevelSensorService

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, initializes the LevelSensor module, 
     especially the hardware for reading from the level sensor array
 Notes

 Author
     J. Edward Carryer, 01/16/12, 10:00
     Drew Bell, 05/14/18 
 ****************************************************************************/
boolean InitLevelSensorService(uint8_t Priority) {
    ES_Event ThisEvent;
    MyPriority = Priority;
   
    fuelLevel = 0;     // set fuel level to empty
    CurrentState = InitLevelState;

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
     PostLevelSensorService

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
boolean PostLevelSensorService(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}


/****************************************************************************
 Function
    RunLevelSensorService

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
ES_Event RunLevelSensorService(ES_Event ThisEvent) {
    ES_Event ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    
    // in all states, if we get an LED_TIMER event, turn off the timer
    if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == LED_TIMER){
       RA0 = 0;     // turn off LED
    }

    switch (CurrentState) {
        case InitLevelState:    // If current state is initial Psedudo State
            RA0 = 1;            // raise for successful init of LevelSensor
            ES_Timer_InitTimer(LED_TIMER, 750); 
            
            if (ThisEvent.EventType == ES_INIT)// only respond to ES_Init
            { 
                if(fuelLevel = readFuelLevel())     // read state and do test
                    CurrentState = TankFueled;
                else{
                    CurrentState = TankEmpty; 
                    ES_Timer_InitTimer(FUEL_EMPTY_TIMER, ONE_SECOND);
                }
            }
            break;
            
        case TankFueled:
            if (ThisEvent.EventType == ES_EMPTY) {
                CurrentState = TankEmpty;
                ES_Timer_InitTimer(FUEL_EMPTY_TIMER, ONE_SECOND);
            } 
            else if (ThisEvent.EventType == ES_FUELED) {
                CurrentState = TankFueled;
            } 
            else if(ThisEvent.EventType == ES_FUEL_QUERY){
                ES_Event FuelEvent;  // transmit fuel Level
                FuelEvent.EventType = ES_TX_REQUEST_SEND;
                FuelEvent.EventParam = constructFuelByte();
                FastPostUARTTXService(FuelEvent);
                
                // Raise Status LED for qtr second on fueled message
                RA0 = 1;
                ES_Timer_InitTimer(LED_TIMER, HALF_SECOND/2);
            }
            break;

        case TankEmpty:
            if (ThisEvent.EventType == ES_FUELED) {
                CurrentState = TankFueled;
                ES_Timer_StopTimer(FUEL_EMPTY_TIMER);   // stop the empty timer
            }
            else if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam ==
                    FUEL_EMPTY_TIMER){
                ES_Event FuelEvent; 
                FuelEvent.EventType = ES_TX_REQUEST_SEND;
                FuelEvent.EventParam = constructFuelByte();
                FastPostUARTTXService(FuelEvent);
                ES_Timer_InitTimer(FUEL_EMPTY_TIMER, ONE_SECOND);
                
                // Raise Status LED for half second on empty message
                RA0 = 1;
                ES_Timer_InitTimer(LED_TIMER, HALF_SECOND);
            }
            break;
    } // end switch on Current State
    return ReturnEvent;
}

/****************************************************************************
 Function
     QueryLevelSensorState

 Parameters
     None

 Returns
     LevelSensorState The current state of the level sensor state machine

 Description
     returns the current state of the level sensor state machine
 Notes

 Author
     Drew Bell, 05/23/18, 19:21
****************************************************************************/
LevelSensorState_t QueryTemplateFSM ( void )
{
   return(CurrentState);
}

/****************************************************************************
 Function
    getFuelLevel

 Parameters
    nothing

 Returns
    uint8_t - returns a number indicating the fuel level as defined in 
              readFuelLevel, i.e. 0 = empty and 8 is completely full.
       
 Description
    returns the last level reading
 Notes
   
 Author
   Drew Bell, 05/14/18, 20:23
 ****************************************************************************/
uint8_t getFuelLevel(void){
    return fuelLevel;
}

/***************************************************************************
 private functions
 ***************************************************************************/


/****************************************************************************
 Function
    readFuelLevel

 Parameters
    nothing

 Returns
    uint8_t - returns 8 bits corresponding to the state of the array 
        
 Description
    reads the state of the 8 input lines reading the individual level sense pads
    and saves the fuel level as a number between 0 (empty) and 8 (full). Number
    is returned and save as a module-level variable.
 Notes
   
 Author
   Drew Bell, 05/07/18, 20:23
 ****************************************************************************/
uint8_t readFuelLevel(void){   
    uint8_t level = 0;
    uint8_t buffer = PORTC;
    
    // get the highest bit set
    while(buffer){                  // loop as long as there is a bit set
        level++;
        buffer = buffer >> 1;
    }
    if(level > NUM_FUEL_PADS) level = NUM_FUEL_PADS;    // clamp at 8
    return fuelLevel = level;      // store value in module-level variable
}

/****************************************************************************
 Function
 CheckFuelLevel

 Parameters
    none

 Returns
   boolean

 Description
   event checker for fuelLevel
 ****************************************************************************/
boolean CheckFuelLevel(void) {
    uint8_t oldFuelLevel = fuelLevel;
    ES_Event LevelEvent;
    readFuelLevel();
    if(fuelLevel != 0 && oldFuelLevel == 0){    // became fueled
        LevelEvent.EventType = ES_FUELED;
        FastPostLevelSensorService(LevelEvent); //Hard coded macro to reduce stack use. TODO: check to make sure priority matches with es configure
        return True;
    }
    else if(fuelLevel == 0 && oldFuelLevel != 0){   // became empty
        LevelEvent.EventType = ES_EMPTY;
        FastPostLevelSensorService(LevelEvent); //Hard coded macro to reduce stack use. TODO: check to make sure priority matches with es configure
        return True;
    }    
    return False;       // otherwise, no event to note
}





/****************************************************************************
 Function
    constructFuelByte

 Parameters
    nothing

 Returns
 uint8_t containing the 8 bit fuel byte
        

 Description
    returns the ready-to-transmit byte containing take status and reading
 Notes
   
 Author
   Drew Bell, 05/14/18, 20:23
 ****************************************************************************/
uint8_t constructFuelByte(void){
    uint8_t fuelMsg = 0;
    if(fuelLevel){    // HAS FUEL
        fuelMsg = fuelLevel - 1;  // need to subtract one to have fill level 
        fuelMsg |= FUELED_BIT;    // span 0-7 for fueled, with empty bit set
    }
    else{   // OUT OF FUEL
        // write garbage in bits except for the "empty" bit
        //fuelMsg = 0x07 & get3GarbageBits();     //debug
        fuelMsg = 0x07;
    }
    fuelMsg |= ((fuelMsg ^ 0x0F) << 4);   //upper 4 bits are complement of lower 4
    return fuelMsg;
}


/****************************************************************************
 Function
    get3GarbageBits

 Parameters
    nothing

 Returns
 uint8_t containing garbage
        

 Description
    returns a byte of mostly-random garbage from scraping the first 16 special 
    function registers
 Notes
   
 Author
   Drew Bell, 05/14/18, 20:23
 ****************************************************************************/
uint8_t get3GarbageBits(void){
    uint8_t garbage = 0x00;
    char *p = 0x00;     // pointer to the special function registers
    for(int i = 0; i < 16; i++){
        garbage ^= *(p + i);
    }
    return garbage;  
}

/****************************************************************************
 Function
 initLevelSensorHW

 Parameters
    nothing

 Returns
    nothing
        

 Description
    inits the level sensor hardware
 Notes
   
 Author
   Drew Bell, 05/14/18, 20:23
 ****************************************************************************/
void initLevelSensorHW(void){
    // Set up RC0-7 as inputs to the 
    TRISC = 0xFF;      // set all of Port C as inputs
    ANSEL = 0x00;      // set analog 0-7 as normal inputs
    ANSELH &= ~0x0F;   // set analog 8-11 as normal inputs
    
    // Set up RA0 (display LED)
    TRISA0 = 0;
    // analog already turned off above
    RA0 = 0;    // start with RA0 Low
}