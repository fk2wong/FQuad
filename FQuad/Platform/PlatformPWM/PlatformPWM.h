/*
 * PlatformPWM.h
 *
 * Created: 2016-09-24 3:47:14 PM
 *  Author: Felix
 */ 


#ifndef PLATFORMPWM_H_
#define PLATFORMPWM_H_

#include "PlatformStatus.h"

typedef enum
{
	PlatformPWM_0A, 
	PlatformPWM_0B,
	PlatformPWM_1A, // Maybe don't implement PWM on timer1 so that all PWM is 8bit resolution, and stick with freq = 16M/256/256Prescaler = 244 Hz.
	PlatformPWM_2A, 
	PlatformPWM_2B,
} PlatformPWM_t;

PlatformStatus PlatformPWM_Init( PlatformPWM_t inPWM );



#endif /* PLATFORMPWM_H_ */