// CONFIG
#include <xc.h>
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator: High-speed crystal/resonator on RA4/OSC2/CLKOUT and RA5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select bit (MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown-out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)

#define _LEGACY_HEADERS
#include <htc.h>
#include <stdio.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_Timers.h"
#include "sci.h"
#include "LevelSensor.h"

void main(void) {
    ES_Return_t ErrorType;

    // Your hardware initialization function calls go here
    /***** 16F690 Code ****
     *  Internal oscillator set to 8MHz
     */
    OSCCON = 0b01110000;

    USART_Init(); // Initialize the UART modules
    InitLevelSensor();

    /*
    // Set up RC3 (display LED)
    TRISC3 = 0;
    ANS7 = 0;
    RC3 = 0; // Initialize low
     */
    

    // now initialize the Events and Services Framework and start it running
    ErrorType = ES_Initialize(ES_Timer_RATE_1MS);
    
    // Start the Framework
    if (ErrorType == Success) {

        ErrorType = ES_Run();

    }
    //if we got to here, there was an error
    switch (ErrorType) {
        case FailedPointer:
            //    puts("Failed on NULL pointer");
            break;
        case FailedInit:
            //    puts("Failed Initialization");
            break;
        default:
            //    puts("Other Failure");
            break;
    }
    for (;;)
        ;

};

/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/
