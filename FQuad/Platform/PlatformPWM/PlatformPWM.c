/*
 * PlatformPWM.c
 *
 * Created: 2016-09-24 3:47:26 PM
 *  Author: Felix
 */ 

#include "PlatformPWM.h"

PlatformStatus PlatformPWM_Init( PlatformPWM_t inPWM )
{
	// Check if the required PWM timer is already init
	
	// If so, then set the compare register and stuff for the new pwm
	
	// Otherwise init the timer, set the frequency, set the DDR in gpio, etc
	// Use MAX as TOP and allow A and B output compare
	
	// Save the bitmask that indicates which pwms are init
}