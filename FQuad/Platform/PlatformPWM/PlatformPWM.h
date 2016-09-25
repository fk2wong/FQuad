/*
 * PlatformPWM.h
 *
 * Created: 2016-09-24 3:47:14 PM
 *  Author: Felix
 */ 


#ifndef PLATFORMPWM_H_
#define PLATFORMPWM_H_

#include "PlatformStatus.h"
#include <stdint.h>

typedef enum
{
	PlatformPWM_0A, 
	PlatformPWM_0B,
	PlatformPWM_1A,
	PlatformPWM_1B,
	PlatformPWM_2A, 
	PlatformPWM_2B,
} PlatformPWM_t;

PlatformStatus PlatformPWM_Init( const PlatformPWM_t inPWM, const uint32_t inRequestedPWMFrequency );

PlatformStatus PlatformPWM_Start( const PlatformPWM_t inPWM, const uint8_t inDutyCycle );

PlatformStatus PlatformPWM_Stop( const PlatformPWM_t inPWM );

PlatformStatus PlatformPWM_Deinit( const PlatformPWM_t inPWM );



#endif /* PLATFORMPWM_H_ */