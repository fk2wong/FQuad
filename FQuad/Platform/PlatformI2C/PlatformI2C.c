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
#define PLATFORM_I2C_WRITE_BIT    ( 0 )
#define PLATFORM_I2C_READ_BIT     ( 1 )

#define PLATFORM_I2C_TWBR_MAX     ( 0xFF )
#define PLATFORM_I2C_TWBR_MIN     ( 0x00 )

#define GET_TWSR_STATUS_CODE() ( TWSR & (( 1 << TWS7 ) | ( 1 << TWS6 ) | ( 1 << TWS5 ) | ( 1 << TWS4 ) | ( 1 << TWS3 ))

enum TWSRStatus
{
	TWSRStatus_StartConditionTransmitted         = 0x08,
	TWSRStatus_RepeatedStartConditionTransmitted = 0x10,
	TWSRStatus_SLAW_ACKReceived                  = 0x18,
	TWSRStatus_SLAW_NACKReceived                 = 0x20,
	TWSRStatus_DataACKReceived                   = 0x28,
	TWSRStatus_DataNACKReceived                  = 0x30,
	TWSRStatus_ArbitrationLost                   = 0x38,
		
};

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

PlatformStatus PlatformI2C_Write( const uint8_t inDeviceAddr, const uint8_t inRegisterAddress, const uint8_t *const inData, const size_t inDataLen )
{
	PlatformStatus status = PlatformStatus_Failed;
	bool startConditionSent = false;
	
	require_quiet( inData,    exit );
	require_quiet( inDataLen, exit );
	
	// Sanity check that there is no current I2C action in progress, and that the write collision bit is cleared
	require_quiet( !( TWCR & ( 1 << TWINT )), exit );
	require_quiet( !( TWCR & ( 1 << TWWC )),  exit );
	
	// Send Start Condition // TODO does this need to be on different clock cycles?
	TWCR |= ( 1 << TWSTA ) | ( 1 << TWINT );
	
	// Wait for interrupt flag, indicating the start condition was sent
	while ( !( TWCR & ( 1 << TWINT )));
	
	// Check the status of the start condition
	require_quiet( GET_TWSR_STATUS_CODE() = TWSRStatus_StartConditionTransmitted, exit );
	
	// Set the device address and write bit (SLA+W)
	TWDR = ( inDeviceAddr << 1 ) | PLATFORM_I2C_WRITE_BIT;
	
	// Clear the Start Condition and write the interrupt flag in order to send SLA+W
	TWCR = ( TWCR & ~( 1 << TWSTA )) | ( 1 << TWINT );
	
	// Wait for interrupt flag, indicating SLA+W was sent
	while ( !( TWCR & ( 1 << TWINT )));
	
	// Check if ACK was received
	require_quiet( GET_TWSR_STATUS_CODE() = TWSRStatus_SLAW_ACKReceived, exit );
	
	// Send each byte of data
	for ( size_t i = 0; i < inDataLen, i++ )
	{
		// Set the data into the data register
		TWDR = inData[0];
		
		// Set the interrupt flag to send the data
		TWCR |= ( 1 << TWSTA );
		
		// Check if ACK was received
		require_quiet( GET_TWSR_STATUS_CODE() = TWSRStatus_DataACKReceived, exit );
	}
	
	status = PlatformStatus_Success;
	
exit:
	if ( startConditionSent )
	{
		// Clear start condition, write stop condition, set interrupt flag to send stop condition
		TWCR = ( TWCR & ~( 1 << TWSTA )) | ( 1 << TWSTO ) | ( 1 << TWINT );
	}
	
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