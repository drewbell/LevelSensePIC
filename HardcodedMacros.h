/* 
 * File:   HardcodedMacros.h
 * Author: TAOffice
 *
 * Created on February 4, 2016, 8:52 PM
 */

#ifndef HARDCODEDMACROS_H
#define	HARDCODEDMACROS_H

// Make sure the first argument number reflects the service number
#define FastPostLevelSensorService(X) ES_PostToService(0, X)    
#define FastPostUARTRXService(X) ES_PostToService(2, X)


#endif	/* HARDCODEDMACROS_H */

