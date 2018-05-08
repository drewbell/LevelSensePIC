/****************************************************************************
 Module
   SPI_SM.c

 Revision
   1.0.1

 Description
   This is a template file for implementing a simple service under the 
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/16/12 09:58 sas      began conversion from TemplateFSM.c
 *02/02/18      hr       edits for 2018 B field
 *****************************************************************.**********/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
 */
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "UART_RX_SM.h"
#include "COMMDEFS.h"
#include "SPI_SM.h"

/*----------------------------- Module Defines ----------------------------*/

#define CMD_BYTE 0x00 // Byte returned in response to the command byte
#define DUMMY_BYTE 0xFF // Byte returned in response to the second byte while we compute

#define CMD_TYPE_MASK (BIT7HI | BIT6HI)     // bits that determine what the response type is
#define CMD_QUERY_SCORE     (0xC3)          // Command for score query
#define CMD_QUERY_STATUS    (0x3F)          // Command for game status query

#define GAME_STATUS_INDEX 0 //start position of game status bytes in the status response array
#define GAME_SCORE_INDEX  GAME_STATUS_INDEX+2 //start position of game score in the status response array

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
 */



/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static SPI_State_t currentState;
static volatile uint8_t responseBytes[2] = {DUMMY_BYTE, DUMMY_BYTE};

static volatile uint8_t *SPIBuffer; //to store variables that will be sent out thro SPI
static volatile uint8_t *StatusResponseStore; //to store status buffer from UART

static volatile boolean commOpen = False;       
static volatile boolean newRequest = False;

/*------------------------------ Module Code ------------------------------*/

/****************************************************************************
 Function
     InitSPIService

 Parameters
 * uint8_t
 Returns
 * bool
 Description
    
 Notes

 Author
     SC, HR - 2/2/18
 ****************************************************************************/
boolean InitSPIService(uint8_t Priority) {
    ES_Event ThisEvent;

    MyPriority = Priority;


    GIE = 0; // Disable interrupts while we set up

    // set CKE = 0 to get the right edge to match SPI Mode 3
    // clear SMP to sample at the middle of the frame
    SSPSTAT = 0b00000000;

    // Init port pins associated with SPI
    // SDO = PC7, SPI data output
    // SDI = RB4, SPI clock input
    // SCK = RB6, SPI clock input
    // SS =  RC6, Slave Select input

    TRISC7 = 0; // output for SPI data (MISO)
    TRISB4 = 1; // input for SPI data (MOSI)
    TRISB6 = 1; // input for SPI clock
    TRISC6 = 1; // input for SS

    // Clear relevant ANSEL
    ANSELH &= (BIT0LO & BIT1LO & BIT2LO);


    SSPBUF = CMD_BYTE; // Initial response always 0x00 per spec.

    // enable the SPI
    // set CKP to get the right clock phase to match SPI Mode 3
    // slave mode with SS enabled
    SSPCON = 0b00110100;

    // init the receive state machine
    currentState = Waiting4Command;

    SSPIE = 1; // enable interrupt for SSP
    PEIE = 1; // enable peripheral interrupts (SSP)
    GIE = 1; // Global interrupts enabled

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
     PostSPIService

 Parameters
 * ES_Event
 Returns
 * bool
 Description
    
 Notes

 Author
     SC, HR - 2/2/18
 ****************************************************************************/
boolean PostSPIService(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

/****************************************************************************
 Function
     RunSPIService

 Parameters
 ES_Event
 Returns
 ES_Event
 Description
  always returns ES_NO_EVENT   
 Notes

 Author
     SC, HR - 2/2/18
 ****************************************************************************/
ES_Event RunSPIService(ES_Event ThisEvent) {
    ES_Event ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    ThisEvent;
    return ReturnEvent;
}
/****************************************************************************
 Function
     SPIIntResponse

 Parameters
 none
 Returns
 * none
 Description
     
 Notes

 Author
     SC, HR - 2/2/18
 ****************************************************************************/
inline void SPIIntResponse(void) 
{
    uint8_t readByte;
    //BF is a register on the PIC for SPI data pending
    if (BF == 1) 
    {   
        // grab data out of SSP fifo, clears the interrupt
        readByte = SSPBUF;

        // State machine to process SPI bytes
        switch (currentState) 
        {
            case Waiting4Command:
                
                SSPBUF = DUMMY_BYTE; // return 0xFF on first byte after command // we put this here now since we wont have time to process all the bits
                //if the UART has a new status ready
                if (RX_newStatusReady() != True) 
                {
                    // We haven't received anything yet, return 0xFF
                  
                    responseBytes[0] = DUMMY_BYTE;
                    responseBytes[1] = DUMMY_BYTE;
                } 
                //if there s a status ready
                else
                { 
                    //Code specific for 2018 field
                    //since we are only broadcasting read in the broadcast status message into status response store
                    StatusResponseStore = RX_getStatusBuffer();
                    // check if command is a query for game status info
                    if (readByte == CMD_QUERY_STATUS) 
                    {
                        //get only the status part of array into the response bytes
                        //TODO: replace test code bytes
                        responseBytes[0] = StatusResponseStore[GAME_STATUS_INDEX];
                        responseBytes[1] = StatusResponseStore[GAME_STATUS_INDEX + 1];
                    } 
                    // check if command is a query for the game score
                    else if (readByte == CMD_QUERY_SCORE) 
                    {
                        //get only score part of array into the response bytes
                        //TODO: replace test code bytes
                        responseBytes[0] = StatusResponseStore[GAME_SCORE_INDEX];
                        responseBytes[1] = StatusResponseStore[GAME_SCORE_INDEX + 1];
                    } 
                    else { // illegal request so return dummy bytes
                        responseBytes[0] = DUMMY_BYTE;
                        responseBytes[1] = DUMMY_BYTE;
                    }
                }
                SPIBuffer = &responseBytes; //Point SPIBuffer to the responseBytes
                currentState = Waiting4Dummy1;
                break;
            case Waiting4Dummy1:                //State: waiting for first dummy byte from ATHLETE
                SSPBUF = SPIBuffer[0];          //Load the second response to buffer
                currentState = Waiting4Dummy2;  //change state to waiting for second dummy byte
                break;
            case Waiting4Dummy2:                //State: waiting for second dummy byte from ATHLETE
                SSPBUF = SPIBuffer[1]; 
                currentState = Waiting4Dummy3; //change state to waiting for third dummy byte
                break;
            case Waiting4Dummy3:                //State: waiting for third dummy byte from ATHLETE
                SSPBUF = CMD_BYTE;
                currentState = Waiting4Command; //change state to waiting for new command
                break;
            default:
                break;
        }
        SSPIF = 0; // clear source of interrupt
    }
}


/****************************************************************************
 Function
    SPI_getCommOpen

 Parameters

 Returns

 Description
     
 Notes

 Author
     SC, HR - 2/2/18
 ****************************************************************************/
boolean SPI_getCommOpen(void) {
    return commOpen;
}

/****************************************************************************
 Function
    CheckSSEvent

 Parameters

 Returns

 Description
     
 Notes

 Author
     SC, HR - 2/2/18
 ****************************************************************************/
boolean CheckSSEvent(void) {
    static uint8_t lastSS = 0; //stores last slave select state
    uint8_t thisSS; //variable for current slave select state
    boolean returnVal = False;

    thisSS = RC6; //read the RC6 pin for the current slave select state
    if (thisSS != lastSS && thisSS) // if the slave select state went from low to high
    {
        // Rising edge on SS, reset to initial state
        currentState = Waiting4Command; //set current state to Wait4Command
        returnVal = True;
    }

    lastSS = thisSS; //store value of current state in last state
    return returnVal;
}

/***************************************************************************
 private functions
 ***************************************************************************/

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/

