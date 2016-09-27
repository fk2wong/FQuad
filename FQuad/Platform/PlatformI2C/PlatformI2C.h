/*
 * PlatformI2C.h
 *
 * Created: 2016-09-26 11:39:46 PM
 *  Author: Felix
 */ 


#ifndef PLATFORMI2C_H_
#define PLATFORMI2C_H_

#include "PlatformStatus.h"

PlatformStatus PlatformI2C_Init( void );

PlatformStatus PlatformI2C_Deinit( void );

#endif /* PLATFORMI2C_H_ */