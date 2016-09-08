/*
 * PlatformUART.c
 *
 * Created: 2016-09-07 5:27:35 PM
 *  Author: Felix
 */ 


#include "PlatformUART.h"
#include "require_macros.h"
#include <stdbool.h>

//===============//
//    Defines    //
//===============//

#ifndef F_CPU
#define F_CPU 16000000
#warning F_CPU Not defined!
#endif

#define PLATFORM_UART_UBRR0L_MASK       ( 0xFF )
#define PLATFORM_UART_UBRR0H_MASK       ( 0x0F )
#define PLATFORM_UART_BAUD_RATE_REG_MAX (( PLATFORM_UART_UBRR0H_MASK << 8 ) | PLATFORM_UART_UBRR0L_MASK )

#define PLATFORM_UART_DATA_REG_EMPTY    ( UCSR0A & ( 1 << UDRE0 ))

//==================================//
//    Static Structs & Variables    //
//==================================//

static bool                mUARTIsInitialized;
static PlatformRingBuffer* mRXBuffer;

//===================================//
//    Public Function Definitions    //
//===================================//

PlatformStatus PlatformUART_Init( uint32_t inBaudRate, PlatformRingBuffer *const inOptionalRingBuffer )
{
	PlatformStatus status = PlatformStatus_Failed;
	uint16_t baudRateRegVal;
	
	require_quiet( !mUARTIsInitialized, exit );
	
	if ( inOptionalRingBuffer )
	{
		mRXBuffer = inOptionalRingBuffer;
	}
	
	// TODO initialize the clocks/powersave bit for the UART?
	
	// Initialize UCSR0A as non-double transmission speed, single processor mode ( All default )
	
	// Initialize UCSR0B bit UCSZ02 as 0 to set 8-bit width ( Default )
	
	// Initialize UCSR0C as Asynchronous (UART); Parity Disabled; 1 Stop bit; 8-bit width
	UCSR0C = ( 1 << UCSZ00 ) | ( 1 << UCSZ01 );
	
	// Calculate Baud Rate Register Values. Formula reference from ATMega328P datasheet table 24-1, with float to int conversion.
	baudRateRegVal = ( uint16_t )((( float )( F_CPU ) / ( float )( inBaudRate )) + 0.5f ) - 1;
	
	// Verify that the baud rate register value is within range
	require_quiet( baudRateRegVal <= PLATFORM_UART_BAUD_RATE_REG_MAX, exit );
	
	// Set the baud rate high and low registers
	UBRR0L = baudRateRegVal & PLATFORM_UART_UBRR0L_MASK;
	UBRR0H = ( baudRateRegVal >> 8 ) & PLATFORM_UART_UBRR0H_MASK;
	
	// Enable the UART transmitter and receiver
	UCSR0B = ( 1 << TXEN0 ) | ( 1 << RXEN0 );
	
	mUARTIsInitialized = true;
	
	status = PlatformStatus_Success;
exit:
	return status;
}

PlatformStatus PlatformUART_Transmit( uint8_t* inBuffer, size_t bufferLen )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	require_quiet( mUARTIsInitialized, exit );
	
	// Sanity check that the data register is empty. It should never be full at this point.
	require_quiet( PLATFORM_UART_DATA_REG_EMPTY, exit );
	
	// Send each byte in the buffer
	for ( uint8_t i = 0; i < bufferLen; i++ )
	{
		// Wait for the data register to be empty before sending it the next byte
		while( !PLATFORM_UART_DATA_REG_EMPTY );
		
		// Place next byte into I/O data register
		UDR0 = inBuffer[i];
	}
	
	status = PlatformStatus_Success;
exit:
	return status;
}