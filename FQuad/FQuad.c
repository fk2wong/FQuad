/*
 * FQuad.c
 *
 * Created: 2016-09-06 11:55:48 AM
 *  Author: Felix
 */ 


#include <avr/io.h>

#define F_CPU 16000000

#include "Platform_FQuad.h"
#include <util/delay.h>

int main(void)
{
	PlatformGPIO_InitAllGPIOs();
	
	PlatformGPIO_Configure( FQuadGPIO_PadX, PlatformGPIOConfig_Output );
	PlatformGPIO_Configure( FQuadGPIO_PadSelect, PlatformGPIOConfig_Output );
	
	PlatformGPIO_Toggle( FQuadGPIO_PadX );
	
    while(1)
    {
		PlatformGPIO_Toggle( FQuadGPIO_PadX );
		PlatformGPIO_Toggle( FQuadGPIO_PadSelect );
		_delay_ms( 1000 );
    }
}