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
	PlatformRingBuffer* ringBuf = NULL;
	
	status = PlatformGPIO_InitAllGPIOs();
	require_noerr( status, exit );
	
	ringBuf = PlatformRingBuffer_Create( 32 );
	require( ringBuf, exit );
	
	status = PlatformUART_Init( 9600, ringBuf );
	require_noerr( status, exit );
	
    while(1)
    {	
		status = PlatformGPIO_Toggle( FQuadGPIO_TestLED );
		require_noerr( status, exit );
		
		uint8_t rxData;
		
		status = PlatformUART_ReceiveBuffer( &rxData, 1 );
// 		
// 		if ( status == 0 )
// 		{
// 			uint8_t rxString[] = "Received: ";
// 					
// 			status = PlatformUART_Transmit( rxString, sizeof( rxString ) - 1 );
// 			require_noerr( status, exit );
// 		
// 			status = PlatformUART_Transmit( &rxData, 1 );
// 			require_noerr( status, exit );
// 					
// 			status = PlatformUART_Transmit(( uint8_t* )".\n", 1 );
// 			require_noerr( status, exit );	
// 		}
				
		_delay_ms( 3000 );
    }
exit:
	while ( 1 );
}