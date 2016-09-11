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
	
	status = PlatformPowerSave_PowerOffAllPeripherals();
	require_noerr_quiet( status, exit );
	
	status = PlatformGPIO_InitAllGPIOs();
	require_noerr_quiet( status, exit );
	
	ringBuf = PlatformRingBuffer_Create( 32 );
	require_quiet( ringBuf, exit );
	
	status = PlatformUART_Init( 19200, ringBuf );
	require_noerr_quiet( status, exit );
	
    while(1)
    {	
		status = PlatformGPIO_Toggle( FQuadGPIO_TestLED );
		require_noerr( status, exit );
		
		uint8_t rxData[5];
		
		status = PlatformUART_Receive( rxData, sizeof( rxData ));
		
		if ( status == 0 )
		{
			uint8_t rxString[] = "Received: ";
					
			status = PlatformUART_Transmit( rxString, sizeof( rxString ) - 1 );
			require_noerr( status, exit );
		
			status = PlatformUART_Transmit( rxData, sizeof( rxData ) );
			require_noerr( status, exit );
					
			status = PlatformUART_Transmit(( uint8_t* )".\n", 2 );
			require_noerr( status, exit );	
		}
				
		_delay_ms( 500 );
    }
exit:
	while ( 1 );
}