/*
 * PlatformPWM.c
 *
 * Created: 2016-09-24 3:47:26 PM
 *  Author: Felix
 */ 

#include "PlatformPWM.h"
#include "PlatformClock.h"
#include "PlatformGPIO.h"
#include "PlatformPowerSave.h"
#include "require_macros.h"
#include <avr/io.h>
#include <stddef.h>
#include <stdlib.h>

#include "FQuadLogging.h" //TEMP remove

#define PLATFORM_PWM_NUM_TIMERS      ( 2 )
#define PLATFORM_PWM_TIMER0_REG_BASE ( 0x44 )
#define PLATFORM_PWM_TIMER2_REG_BASE ( 0xB0 )

#define PLATFORM_PWM_TIMER_MAX_VALUE ( 0xFF )
#define PLATFORM_PWM_DUTY_CYCLE_MAX  ( 100.0f )
#define PLATFORM_PWM_DUTY_CYCLE_RESOLUTION ( PLATFORM_PWM_DUTY_CYCLE_MAX / ( PLATFORM_PWM_TIMER_MAX_VALUE + 1 ))

#define PLATFORM_PWM_GET_16_BIT_COMPARE_VALUE_FROM_DUTY_CYCLE( X ) ( uint16_t )(((( float )( PLATFORM_PWM_TIMER_MAX_VALUE + 1 ) * ( X )) / PLATFORM_PWM_DUTY_CYCLE_MAX ) + 0.5f )
#define PLATFORM_PWM_GET_COMPARE_VALUE_FROM_DUTY_CYCLE( X ) ( uint8_t )( PLATFORM_PWM_GET_16_BIT_COMPARE_VALUE_FROM_DUTY_CYCLE( X ) < ( PLATFORM_PWM_TIMER_MAX_VALUE + 1 ) ? PLATFORM_PWM_GET_16_BIT_COMPARE_VALUE_FROM_DUTY_CYCLE( X ) : PLATFORM_PWM_TIMER_MAX_VALUE )

#define IS_PWM_OUTPUT_CHANNEL_A( PWM ) ((( PWM ) % 2 ) == 0 )

typedef struct 
{
	volatile uint8_t TCCRXA;
	volatile uint8_t TCCRXB;
	volatile uint8_t TCNTX;
	volatile uint8_t OCRXA;
	volatile uint8_t OCRXB;
} PlatformPWMTimerRegBase_t;

typedef struct 
{
	uint8_t                       initCount;
	PlatformPWMTimerRegBase_t    *regBase;
	PlatformPowerSavePeripheral_t powerSavePeripheral;
	PlatformGPIO_t                outputA;
	PlatformGPIO_t                outputB;
} PlatformPWMTimerStruct_t;

static PlatformPWMTimerStruct_t mPlatformPWMTimer0Struct = 
{
	.initCount           = 0,
	.regBase             = ( PlatformPWMTimerRegBase_t* ) PLATFORM_PWM_TIMER0_REG_BASE,
	.powerSavePeripheral = PlatformPowerSavePeripheral_Timer0,
	.outputA             = PlatformGPIO_PTD6,
	.outputB             = PlatformGPIO_PTD5,
};

static PlatformPWMTimerStruct_t mPlatformPWMTimer2Struct = 
{
	.initCount           = 0,
	.regBase             = ( PlatformPWMTimerRegBase_t* ) PLATFORM_PWM_TIMER2_REG_BASE,
	.powerSavePeripheral = PlatformPowerSavePeripheral_Timer2,
	.outputA             = PlatformGPIO_PTB3,
	.outputB             = PlatformGPIO_PTD3,
};

static const uint16_t kPlatformPWMTimerPrescalers[]   = { 1, 8, 64, 256, 1024 };
static const uint8_t  kPlatformPWMTimerPrescaleBits[] = { 1, 2, 3, 4, 5 };

static uint8_t mInitializedPWMChannels;

static PlatformStatus _PlatformPWM_DisconnectOutput( const PlatformPWM_t inPWM, const PlatformPWMTimerStruct_t* const inPWMTimer );
static PlatformPWMTimerStruct_t * _PlatformPWM_GetPWMTimerStruct( const PlatformPWM_t inPWM );
static uint8_t _PlatformPWM_GetPWMTimerPrescalerBits( const uint32_t inRequestedPWMFrequency );

PlatformStatus PlatformPWM_Init( const PlatformPWM_t inPWM, const uint32_t inRequestedPWMFrequency )
{
	PlatformStatus status = PlatformStatus_Failed;
	PlatformPWMTimerStruct_t *pwmTimer;
	uint8_t timerPrescalerBits;
	
	require_action_quiet(( inPWM != PlatformPWM_1A ) && ( inPWM != PlatformPWM_1B ), exit, status = PlatformStatus_NotSupported );
	
	// Check that this PWM has not yet been initialized
	require_action_quiet(( mInitializedPWMChannels & ( 1 << inPWM )) == 0, exit, status = PlatformStatus_AlreadyInitialized );
	
	// Get this PWM's timer struct
	pwmTimer = _PlatformPWM_GetPWMTimerStruct( inPWM );
	require_quiet( pwmTimer, exit );
	
	// Initialize the timer if it hasn't already been initialized
	if ( pwmTimer->initCount == 0 )
	{	
		// Disable Powersave on this timer
		status = PlatformPowerSave_PowerOnPeripheral( pwmTimer->powerSavePeripheral );
		require_noerr_quiet( status, exit );
		
		// Set the prescaler bits that best match the requested PWM frequency
		timerPrescalerBits = _PlatformPWM_GetPWMTimerPrescalerBits( inRequestedPWMFrequency );
		require_action_quiet( timerPrescalerBits, exit, status = PlatformStatus_Failed );
		
		pwmTimer->regBase->TCCRXB = timerPrescalerBits;
		
		FQUAD_DEBUG_LOG(( "PWM Prescaler: %d\n", timerPrescalerBits ));
		
		// Set the timer to use Fast PWM
		pwmTimer->regBase->TCCRXA |= ( 1 << WGM01 ) | ( 1 << WGM00 );
	}
	
	// Nothing to do for OCRXA/B, these will be set in PlatformPWM_Start()
	
	// Increment this timer's init count
	pwmTimer->initCount++;
	
	// Mark this PWM as initialized
	mInitializedPWMChannels |= ( 1 << inPWM );
	
	FQUAD_DEBUG_LOG(("Registers: %d, %d, %d, %d, %d\n", &pwmTimer->regBase->TCCRXA, &pwmTimer->regBase->TCCRXB, &pwmTimer->regBase->TCNTX, 
	&pwmTimer->regBase->OCRXA, &pwmTimer->regBase->OCRXB ));
	
	FQUAD_DEBUG_LOG(("Registers: %d, %d, %d, %d, %d\n", pwmTimer->regBase->TCCRXA, pwmTimer->regBase->TCCRXB, pwmTimer->regBase->TCNTX,
	pwmTimer->regBase->OCRXA, pwmTimer->regBase->OCRXB ));

	status = PlatformStatus_Success;
exit:
	return status;
}

PlatformStatus PlatformPWM_Deinit( const PlatformPWM_t inPWM )
{
	PlatformStatus status = PlatformStatus_Failed;
	PlatformPWMTimerStruct_t *pwmTimer;
		
	// Check that this PWM has been initialized
	require_action_quiet(( mInitializedPWMChannels & ( 1 << inPWM )) == 0, exit, status = PlatformStatus_NotInitialized );
		
	// Get this PWM's timer struct
	pwmTimer = _PlatformPWM_GetPWMTimerStruct( inPWM );
	require_quiet( pwmTimer, exit );
	
	// Make sure the PWM output has stopped
	status = _PlatformPWM_DisconnectOutput( inPWM, pwmTimer );
	require_noerr_quiet( status, exit );
	
	// If this is the only PWM channel using this timer, then deinit the timer
	if ( pwmTimer->initCount == 1 )
	{
		// Set the timer mode to normal operation, output disconnected
		pwmTimer->regBase->TCCRXA = 0;
		
		// Disable the clock source
		pwmTimer->regBase->TCCRXB = 0;
		
		// Enable PowerSave on this timer
		status = PlatformPowerSave_PowerOffPeripheral( pwmTimer->powerSavePeripheral );
		require_noerr_quiet( status, exit );
	}
	
	// Decrement the init count
	pwmTimer->initCount--;
	
	// Mark this PWM as not init
	mInitializedPWMChannels &= ~( 1 << inPWM );
	
	status = PlatformStatus_Success;
exit:
	return status;
}

PlatformStatus PlatformPWM_Start( const PlatformPWM_t inPWM, const float inDutyCycle )
{
	PlatformStatus status = PlatformStatus_Failed;
	PlatformPWMTimerStruct_t *pwmTimer;
	uint8_t compareRegValue;
	
	require_action_quiet( mInitializedPWMChannels & ( 1 << inPWM ), exit, status = PlatformStatus_NotInitialized );
	require_quiet( inDutyCycle >= 0, exit );
	require_quiet( inDutyCycle <= PLATFORM_PWM_DUTY_CYCLE_MAX, exit );	
	
	// Get this PWM's timer struct
	pwmTimer = _PlatformPWM_GetPWMTimerStruct( inPWM );
	require_quiet( pwmTimer, exit );
	
	FQUAD_DEBUG_LOG(( "Init count: %d\n", pwmTimer->initCount ));
	// Sanity check that this timer has been init
	require_quiet( pwmTimer->initCount > 0, exit );
	
	// Get the Compare Register value that produces this duty cycle
	compareRegValue = PLATFORM_PWM_GET_COMPARE_VALUE_FROM_DUTY_CYCLE( inDutyCycle );
	
	FQUAD_DEBUG_LOG(("Compare Value: %d\n", compareRegValue ));
	
	if ( IS_PWM_OUTPUT_CHANNEL_A( inPWM ))
	{
		// Configure the GPIO as output
		status = PlatformGPIO_Configure( pwmTimer->outputA, PlatformGPIOConfig_Output );
		require_noerr_quiet( status, exit );
		
		// If the compare value is 0, then just disable the PWM output.
		if ( compareRegValue == 0 )
		{
			pwmTimer->regBase->TCCRXA &= ~(( 1 << COM0A1 ) | ( 1 << COM0A0 ));
		}
		// Otherwise set the compare register value and enable non-inverting PWM 
		else 
		{
			pwmTimer->regBase->OCRXA   = compareRegValue;
			pwmTimer->regBase->TCCRXA |= ( 1 << COM0A1 );
		}
	}
	else
	{
		// Configure the GPIO as output
		status = PlatformGPIO_Configure( pwmTimer->outputB, PlatformGPIOConfig_Output );
		require_noerr_quiet( status, exit );
			
		// If the compare value is 0, then just disable the PWM output.
		if ( compareRegValue == 0 )
		{
			pwmTimer->regBase->TCCRXB &= ~(( 1 << COM0B1 ) | ( 1 << COM0B0 ));
		}
		// Otherwise set the compare register value and enable non-inverting PWM
		else
		{
			pwmTimer->regBase->OCRXB   = compareRegValue;
			pwmTimer->regBase->TCCRXB |= ( 1 << COM0B1 );
		}
	}
	
	status = PlatformStatus_Success;
exit:
	return status;
}

PlatformStatus PlatformPWM_Stop( const PlatformPWM_t inPWM )
{
	PlatformStatus status = PlatformStatus_Failed;
	PlatformPWMTimerStruct_t *pwmTimer;
	
	require_action_quiet( mInitializedPWMChannels & ( 1 << inPWM ), exit, status = PlatformStatus_NotInitialized );
	
	// Get this PWM's timer struct
	pwmTimer = _PlatformPWM_GetPWMTimerStruct( inPWM );
	require_quiet( pwmTimer, exit );
	
	// Sanity check that this timer has been init
	require_quiet( pwmTimer->initCount > 0, exit );
	
	status = _PlatformPWM_DisconnectOutput( inPWM, pwmTimer );
	require_noerr_quiet( status, exit );

exit:
	return status;
}

static PlatformStatus _PlatformPWM_DisconnectOutput( const PlatformPWM_t inPWM, const PlatformPWMTimerStruct_t* const inPWMTimer )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	require_quiet( inPWMTimer, exit );
	
	if ( IS_PWM_OUTPUT_CHANNEL_A( inPWM ))
	{
		// Disable the PWM output
		inPWMTimer->regBase->TCCRXA &= ~(( 1 << COM0A1 ) | ( 1 << COM0A0 ));
	}
	else
	{
		// Disable the PWM output
		inPWMTimer->regBase->TCCRXB &= ~(( 1 << COM0B1 ) | ( 1 << COM0B0 ));
	}
	
	status = PlatformStatus_Success;
exit:
	return status;
}

static PlatformPWMTimerStruct_t * _PlatformPWM_GetPWMTimerStruct( const PlatformPWM_t inPWM ) 
{
	PlatformPWMTimerStruct_t* pwmTimerStruct;
	
	switch ( inPWM )
	{
		case PlatformPWM_0A:
		case PlatformPWM_0B:
		{
			pwmTimerStruct = &mPlatformPWMTimer0Struct;
			break;
		}
		case PlatformPWM_2A:
		case PlatformPWM_2B:
		{
			pwmTimerStruct = &mPlatformPWMTimer2Struct;
			break;
		}
		default:
		{
			pwmTimerStruct = NULL;
			break;
		}
	}
	
	return pwmTimerStruct;
}

static uint8_t _PlatformPWM_GetPWMTimerPrescalerBits( const uint32_t inRequestedPWMFrequency )
{
	uint8_t prescalerBits = 0;
	uint32_t currentFreq;
	uint32_t nextFreq;
	
	for ( uint8_t i = 0; i < (( sizeof( kPlatformPWMTimerPrescalers ) / sizeof( uint16_t )) - 1 ); i++ )
	{
		currentFreq = F_CPU / kPlatformPWMTimerPrescalers[i] / ( PLATFORM_PWM_TIMER_MAX_VALUE + 1 );   // From ATmega328p datasheet, section 14.7.3
		nextFreq    = F_CPU / kPlatformPWMTimerPrescalers[i+1] / ( PLATFORM_PWM_TIMER_MAX_VALUE + 1 );
		
		FQUAD_DEBUG_LOG(( "Current: %lu, Next: %lu, Req: %lu\n", currentFreq, nextFreq, inRequestedPWMFrequency ));
		
		// If the requested frequency is closer to the current frequency than the next frequency, use this prescaler value.
		if ( labs( inRequestedPWMFrequency - currentFreq ) <= labs( inRequestedPWMFrequency - nextFreq ))
		{
			FQUAD_DEBUG_LOG(("Match!\n"));
			prescalerBits = kPlatformPWMTimerPrescaleBits[i];
			break;
		}
	}
	// If no prescaler value was found, then the requested frequency is too low; set it to the max prescaler.
	if ( prescalerBits == 0 )
	{
		prescalerBits = kPlatformPWMTimerPrescaleBits[ sizeof( kPlatformPWMTimerPrescaleBits ) / sizeof( uint8_t ) - 1 ];
	}
	
	return prescalerBits;
}