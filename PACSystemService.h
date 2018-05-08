#ifndef PACSYSTEMSERVICE_H
#define PACSYSTEMSERVICE_H

#include "ES_Types.h"

// Public Function Prototypes

boolean InitPACSystemService(uint8_t Priority);
boolean PostPACSystemService(ES_Event ThisEvent);
ES_Event RunPACSystemService(ES_Event ThisEvent);


#endif /* PACSYSTEMSERVICE_H */

