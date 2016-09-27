/*
 * PlatformI2C.c
 *
 * Created: 2016-09-26 11:39:35 PM
 *  Author: Felix
 */ 

#include "PlatformI2C.h"
#include "PlatformStatus.h"
#include "PlatformPowerSave.h"
#include "PlatformClock.h"
#include "require_macros.h"
#include <avr/io.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define PLATFORM_I2C_FAST_MODE_HZ ( 400000 )

#define PLATFORM_I2C_TWBR_MAX     ( 0xFF )
#define PLATFORM_I2C_TWBR_MIN     ( 0x00 )

static const uint8_t kPlatformI2CBitRatePrescalers[] = { 1, 4, 16, 64 };
	
static PlatformStatus _PlatformI2C_GetClockPrescalerBitsAndBitRateValues( uint32_t inCPUFreq, uint8_t *const outPrescalerBits, uint8_t *const outBitRateVal );

PlatformStatus PlatformI2C_Init( void )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	uint8_t clockPrescalerBits;
	uint8_t bitRateRegisterValue;
	
	// Enable Power to the I2C peripheral
	status = PlatformPowerSave_PowerOnPeripheral( PlatformPowerSavePeripheral_I2C );
	require_noerr_quiet( status, exit );
	
	// Get the prescaler and bit rate values that create an SCL frequency of 400kHz
	status = _PlatformI2C_GetClockPrescalerBitsAndBitRateValues( F_CPU, &clockPrescalerBits, &bitRateRegisterValue );
	require_noerr_quiet( status, exit );
	
	// Set the prescaler bits
	TWSR |= clockPrescalerBits;
	
	// Set the bit rate register
	TWBR = bitRateRegisterValue;
	
	// Enable the I2C peripheral, and enable the ACK bit
	TWCR = ( 1 << TWEA ) | ( 1 << TWEN );
	
	//FQUAD_DEBUG_LOG(( "TWSR: %d, TWBR: %d, TWCR: %d\n", TWSR, TWBR, TWCR ));
	
	exit:
	return status;
}


static PlatformStatus _PlatformI2C_GetClockPrescalerBitsAndBitRateValues( uint32_t inCPUFreq, uint8_t *const outPrescalerBits, uint8_t *const outBitRateVal )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	uint32_t currentPrescalerUpperSpeed;
	uint32_t currentPrescalerLowerSpeed;
	uint8_t  prescalerIndex = 0;
	
	bool prescalerFound = false;
	
	require_quiet( outPrescalerBits,  exit );
	require_quiet( outBitRateVal, exit );
	
	for ( uint8_t prescalerIndex = 0; prescalerIndex < sizeof( kPlatformI2CBitRatePrescalers ); prescalerIndex++ )
	{
		// The below formula is taken from Section 21.5.2 of the ATmega328p datasheet
		currentPrescalerUpperSpeed = inCPUFreq / ( 16 + ( 2 * PLATFORM_I2C_TWBR_MIN * kPlatformI2CBitRatePrescalers[ prescalerIndex ] ));
		currentPrescalerLowerSpeed = inCPUFreq / ( 16 + ( 2 * PLATFORM_I2C_TWBR_MAX * kPlatformI2CBitRatePrescalers[ prescalerIndex ] ));
		
		//FQUAD_DEBUG_LOG(( "Max Clock: %lu, Min: %lu\n", currentPrescalerUpperSpeed, currentPrescalerLowerSpeed ));
		
		// If the current prescaler can accomodate the 400kHz of fast mode, break and use it
		if (( currentPrescalerUpperSpeed >= PLATFORM_I2C_FAST_MODE_HZ ) || ( PLATFORM_I2C_FAST_MODE_HZ >= currentPrescalerLowerSpeed ))
		{
			prescalerFound = true;
			break;
		}
	}
	
	require_quiet( prescalerFound, exit );
	
	// Set the returned prescaler bits as the index 0-3 into the array kPlatformI2CBitRatePrescalers
	*outPrescalerBits = prescalerIndex;
	
	// The below formula is rearranged from Section 21.5.2 from the Atmega328p datasheet
	*outBitRateVal    = (( inCPUFreq / PLATFORM_I2C_FAST_MODE_HZ ) - 16 ) / ( 2 * kPlatformI2CBitRatePrescalers[ prescalerIndex ] );
	
	status = PlatformStatus_Success;
	exit:
	return status;
}

PlatformStatus PlatformI2C_Deinit( void )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	// Disable the I2C peripheral
	TWCR &= ~( 1 << TWEN );
	
	// Disable power to the peripheral
	status = PlatformPowerSave_PowerOffPeripheral( PlatformPowerSavePeripheral_I2C );
	require_noerr_quiet( status, exit );
	
exit:
	return status;
}