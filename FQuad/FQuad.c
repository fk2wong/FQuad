/*
 * FQuad.c
 *
 * Created: 2016-09-06 11:55:48 AM
 *  Author: Felix
 */ 


#include <avr/io.h>

#define F_CPU 16000000
#include <util/delay.h>
#include "Platform_FQuad.h"
#include "PlatformGPIO.h"

int main(void)
{
	PlatformGPIO_Configure( FQuadGPIO_TestLED, PlatformGPIOConfig_Output );
    while(1)
    {
		PlatformGPIO_Toggle( FQuadGPIO_TestLED );
		_delay_ms( 1000 );
    }
}