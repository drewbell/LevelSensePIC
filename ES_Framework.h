/****************************************************************************
 Module
     EF_Framework.h
 Description
     header file for use with the top level functions of the EF Event Framework
 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 10/17/06 07:41 jec      started coding
*****************************************************************************/

#ifndef ES_Framework_H
#define ES_Framework_H

#include "ES_Port.h"
#include "ES_Types.h"
#include "ES_General.h"
#include "ES_CheckEvents.h"
#include "ES_PostList.h"
#include "ES_Events.h"
#include "ES_Timers.h"

typedef enum {
              Success = 0,
              FailedPost = 1,
              FailedRun,
              FailedPointer,
              FailedIndex,
              FailedInit
} ES_Return_t;

ES_Return_t ES_Initialize( TimerRate_t NewRate  );
ES_Return_t ES_Run( void );
boolean ES_PostAll( ES_Event ThisEvent );
boolean ES_PostToService( uint8_t WhichService, ES_Event ThisEvent);

#endif   // ES_Framework_H
