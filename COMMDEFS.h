/* 
 * File:   COMMDEFS.h
 * Author: TAOffice
 *
 * Created on February 8, 2018, 11:10 AM
 */

#ifndef COMMDEFS_H
#define	COMMDEFS_H

#ifdef	__cplusplus
extern "C" {
#endif

#define FRAME_BYTE 0x7E
#define MSB        0
#define API_RX_DATA 0x81
#define API_TX_STAT 0x89
#define VALID_CKSUM 0xFF

//this variable is set by comm protocol
#define MSG_SIZE       9   // Maximum length of a received mssage from the XBee

#define GAME_STATUS_SIZE 2 //number of bytes to store game status response
#define SCORE_SIZE       2    //number of bytes to store score response
#define STAT_SIZE GAME_STATUS_SIZE + SCORE_SIZE  // number of bytes needed to store status //TODO: make sure correct number

#define RESP_SIZE       3   // number of bytes needed to store response

// Define offset into the message bytes
    
// RX Offsets //within the data part fo the packet
#define API_INDENTIFIER_INDEX 0        // Offset to API type byte
#define FIELD_ADDRESS_MSB_INDEX 1 //MSB of field address
#define FIELD_ADDRESS_LSB_INDEX 2    //LSB of field address
#define RSSI_INDEX     3
#define OPTIONS_INDEX 4 //bit 0, reserved, bit 1 address broadcast, bit 2 PAN broadcast, bit 3-7 reserved
#define DATA_INDEX  OPTIONS_INDEX+1

//Rx masks
#define BROADCAST_MASK (BIT2HI | BIT1HI)
    
// TX Offsets
#define FID_OFFSET 1        // Offset to frame ID
#define STAT_OFFSET 2       // offset to TX Status byte

#define ADH_SHIFT  8        // Shift required to address high byte


#ifdef	__cplusplus
}
#endif

#endif	/* COMMDEFS_H */

