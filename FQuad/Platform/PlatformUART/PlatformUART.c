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

#define PLATFORM_UART_RX_FIFO_SIZE      ( 3 )

//==================================//
//    Static Structs & Variables    //
//==================================//

static bool                mUARTIsInitialized;
static PlatformRingBuffer* mRXRingBuffer;

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
		mRXRingBuffer = inOptionalRingBuffer;
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
	
	// If there is a ring buffer, then we will the RX Complete ISR to read the data into the ring buffer
	if ( inOptionalRingBuffer )
	{
		UCSR0B |= ( 1 << RXCIE0 );
	}
	
	mUARTIsInitialized = true;
	
	status = PlatformStatus_Success;
exit:
	return status;
}

PlatformStatus PlatformUART_Transmit( uint8_t* inBuffer, size_t inBufferLen )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	require_quiet( mUARTIsInitialized, exit );
	
	// Sanity check that the data register is empty. It should never be full at this point.
	require_quiet( PLATFORM_UART_DATA_REG_EMPTY, exit );
	
	// Send each byte in the buffer
	for ( uint8_t i = 0; i < inBufferLen; i++ )
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

PlatformStatus PlatformUART_ReceiveBuffer( uint8_t* const outBuffer, size_t inRequestedLen, size_t *const outActualLen )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	require_quiet( outBuffer,       exit );
	require_quiet( inRequestedLen, exit );
	require_quiet( outActualLen,   exit );
	
	// If there is a ring buffer, then read data from it
	if ( mRXRingBuffer )
	{
		status = PlatformRingBuffer_ReadBuffer( mRXRingBuffer, outBuffer, inRequestedLen, outActualLen );
	}
	// Otherwise read from the USART RX FIFO
	else
	{
		// Cannot read more than the FIFO can hold
		require_quiet( inRequestedLen <= PLATFORM_UART_RX_FIFO_SIZE, exit );
		
		*outActualLen = 0;
		
		// Get each byte from the Data Register, which will be updated with the next byte in the FIFO after read complete
		for ( uint8_t i = 0; i < inRequestedLen; i++ )
		{
			// Make sure there is something to be read
			require_quiet(( UCSR0A & ( 1 << RXC0 )), exit );
			
			// Copy it to the output buffer
			outBuffer[i] = UDR0;
			
			( *outActualLen )++;
		}
	}
	
	status = PlatformStatus_Success;
exit:
	return status;
}