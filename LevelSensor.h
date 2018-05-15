#ifndef LevelSensor_H
#define	LevelSensor_H


#include "ES_Framework.h"

// typedefs for the states
// State definitions for use with the query function

typedef enum {
    InitLevelState, TankEmpty, TankFueled
} LevelSensorState_t;


boolean InitLevelSensorService(uint8_t Priority);
boolean PostLevelSensorService(ES_Event ThisEvent);
ES_Event RunLevelSensorService(ES_Event ThisEvent);
LevelSensorState_t QueryLevelSensorState(void);


// Public Function Prototypes
uint8_t readFuelLevel(void);
uint8_t getLastLevelReading(void);

#endif	/* LevelSensor_H */


