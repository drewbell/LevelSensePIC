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
#include "LevelSensor.h"


/*----------------------------- Module Defines ----------------------------*/


/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
 */

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t lastReading = 0;

/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
 Function
     InitLevelSensor

 Parameters
    Takes nothing

 Returns
     bool, false if error in initialization, true otherwise

 Description
     initializes the LevelSensor module, especially the hardware for reading from
     from the level sensor array
 Notes

 Author
     Drew Bell, 05/07/18, 20:00
 ****************************************************************************/
boolean InitlevelSensor(void) {
    // Set up RC0-7 as inputs to the 
    TRISC = 0xFF;      // set all as inputs
    ANSEL = 0;         // set all as digital
    return True;
}


/****************************************************************************
 Function
    takeLevelReading

 Parameters
    nothing

 Returns
    uint8_t - returns 8 bits corresponding to the state of the array 
        

 Description
    reads the state of the 8 input lines reading the individual level sense pads
 Notes
   
 Author
   Drew Bell, 05/07/18, 20:23
 ****************************************************************************/
uint8_t takeLevelReading(void){   
    lastReading = PORTC;
    return lastReading;
}

/****************************************************************************
 Function
    getLastLevelReading

 Parameters
    nothing

 Returns
    uint8_t - returns 8 bits corresponding to the last state of the array 
        

 Description
    returns the last level reading
 Notes
   
 Author
   Drew Bell, 05/07/18, 20:23
 ****************************************************************************/
uint8_t getLastLevelReading(void){
    return lastReading;
}