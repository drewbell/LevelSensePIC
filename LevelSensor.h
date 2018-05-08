#ifndef LevelSensor_H
#define	LevelSensor_H


#include "ES_Framework.h"

// Public Function Prototypes
boolean InitLevelSensor(void);
uint8_t takeLevelReading(void);
uint8_t getLastLevelReading(void);

#endif	/* LevelSensor_H */


