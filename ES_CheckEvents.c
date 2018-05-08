/****************************************************************************
 Module
     ES_CheckEvents.c
 Description
     source file for the module to call the User event checking routines
 Notes
     Users should only modify the contents of the EF_EventList array.
 History
 When           Who     What/Why
 -------------- ---     --------
 10/16/11 12:32 jec      started coding
 *****************************************************************************/

#include "ES_Configure.h"
#include "ES_Events.h"
#include "ES_General.h"
#include "EventCheckers.h"
#include "ES_CheckEvents.h"

// Include the header files for the module(s) with your event checkers. 
// This gets you the prototypes for the event checking functions.

#include EVENT_CHECK_HEADER

// Fill in this array with the names of your event checking functions

static CheckFunc * const ES_EventList[] = {EVENT_CHECK_LIST};


// Implementation for public functions

/****************************************************************************
 Function
   ES_CheckUserEvents
 Parameters
   None
 Returns
   True if any of the user event checkers returned True, False otherwise
 Description
   loop through the EF_EventList array executing the event checking functions
 Notes
   
 Author
   J. Edward Carryer, 10/25/11, 08:55
 ****************************************************************************/
boolean ES_CheckUserEvents(void) {
    // TODO: Not using function pointers because that can only use hardware 
    // stack space, and not managed stack

    if (CheckUARTRXEvent()) return True;
   
    return False;
    
}
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
