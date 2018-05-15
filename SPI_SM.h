#ifndef SPI_SM_H
#define SPI_SM_H

#include "ES_Framework.h"

typedef enum {
    Waiting4Command, Waiting4Dummy1, Waiting4Dummy2,
    Waiting4Dummy3
} SPI_State_t;

boolean InitSPIService(uint8_t Priority);
boolean PostSPIService(ES_Event ThisEvent);
ES_Event RunSPIService(ES_Event ThisEvent);

boolean CheckSPIEvent(void);
boolean CheckSSEvent(void);

//inline void SPIIntResponse(void);

boolean SPI_getCommOpen(void);

#endif //SPI_SM_H
