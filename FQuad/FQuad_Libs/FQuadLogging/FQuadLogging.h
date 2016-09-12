/*
 * FQuadLogging.h
 *
 * Created: 2016-09-12 2:33:25 PM
 *  Author: Felix
 */ 


#ifndef FQUADLOGGING_H_
#define FQUADLOGGING_H_

#include "PlatformUART.h"
#include <string.h>
#include <stdio.h>

#define USING_UART_AS_LOGGING 1 // Set to 0 when using UART for XBee

#define FQUAD_LOGGING_MAX_BUFFER_SIZE ( 64 )

#define REMOVE_BRACES( ... ) __VA_ARGS__

#if USING_UART_AS_LOGGING

void FQuadLogging_Print( const char* str, ... );

#define FQUAD_DEBUG_LOG( X )                                             \
	do                                                                   \
	{                                                                    \
		char str[ FQUAD_LOGGING_MAX_BUFFER_SIZE ];                       \
		size_t strLen;                                                   \
	                                                                     \
		snprintf( str, FQUAD_LOGGING_MAX_BUFFER_SIZE, REMOVE_BRACES X ); \
		strLen = strlen( str );                                          \
		PlatformUART_Transmit(( uint8_t* )str, strLen );                 \
	} while ( 0 )
#else // USING_UART_AS_LOGGING
#define FQUAD_DEBUG_LOG( X )
#endif // USING_UART_AS_LOGGING

#endif /* FQUADLOGGING_H_ */