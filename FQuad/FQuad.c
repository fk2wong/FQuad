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
#include "require_macros.h"

int main(void)
{
	PlatformStatus status;
	status = PlatformGPIO_InitAllGPIOs();
	require_noerr( status, exit );
	
    while(1)
    {	
		status = PlatformGPIO_Toggle( FQuadGPIO_TestLED );
		require_noerr( status, exit );
		
		_delay_ms( 1000 );
    }
exit:
	while ( 1 );
}