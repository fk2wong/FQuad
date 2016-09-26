/*
 * FQuad.c
 *
 * Created: 2016-09-06 11:55:48 AM
 *  Author: Felix
 */ 




#include "Platform_FQuad.h"
#include "FQuadLogging.h"
#include "require_macros.h"
#include <util/delay.h>
#include <stdlib.h>
#include <avr/io.h>

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
	
	status = PlatformPWM_Init( FQuadPWM_MotorNW, 250, NULL );
	require_noerr_quiet( status, exit );
	
    while(1)
    {	
		// Test GPIO
		status = PlatformGPIO_Toggle( FQuadGPIO_TestLED );
		require_noerr( status, exit );
		
		// Test UART
		uint8_t rxData[5];
		status = PlatformUART_Receive( rxData, sizeof( rxData ));
		
		if ( status == PlatformStatus_Success )
		{
			FQUAD_DEBUG_LOG(( "UART Received: %s.\n", rxData ));	
		}
		
		// Test ADC
		status = PlatformADC_Init( FQuadADC_PadLeftVertical );
		require_noerr_quiet( status, exit );
		
		uint16_t adcVal;
		status = PlatformADC_Read( FQuadADC_PadLeftVertical, &adcVal );
		require_noerr( status, exit );
		
		FQUAD_DEBUG_LOG(( "ADC Read: %d\n", adcVal ));
		
		status = PlatformADC_Deinit( FQuadADC_PadLeftVertical );
		require_noerr_quiet( status, exit );
		
		// Test PWM output
		status = PlatformPWM_Start( FQuadPWM_MotorNW, 0.3 );
		FQUAD_DEBUG_LOG(( "PWM Status: %d\n", status ));
		require_noerr_quiet( status, exit );
		
		_delay_ms( 500 );
		
    }
exit:
	while ( 1 )
	{
		PlatformGPIO_Toggle( FQuadGPIO_TestLED );
		_delay_ms( 100 );
	}
}