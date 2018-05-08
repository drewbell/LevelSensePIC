#ifndef SCI_H_
#define SCI_H_

#include "ES_Types.h"

void USART_Init(void);
boolean USART_IsTxOK(void);
boolean USART_IsRxReady(void);
boolean USART_SendByte(unsigned char Byte2Send);
unsigned char USART_GetRxByte(void);


#endif /*SCI_H_*/
