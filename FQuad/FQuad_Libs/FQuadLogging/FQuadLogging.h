/*
 * FQuadLogging.h
 *
 * Created: 2016-09-12 2:33:25 PM
 *  Author: Felix
 */ 


#ifndef FQUADLOGGING_H_
#define FQUADLOGGING_H_

#define ENABLE_DEBUG_LOGGING 1  // Since this shares UART with XBee, this should be set to 0 when the XBee is used.

#define FQUAD_LOGGING_MAX_BUFFER_SIZE ( 64 )

#define REMOVE_BRACES( ... ) __VA_ARGS__

#if ENABLE_DEBUG_LOGGING

#include "PlatformUART.h"
#include <string.h>
#include <stdio.h>

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
#else // ENABLE_DEBUG_LOGGING
#define FQUAD_DEBUG_LOG( X )
#endif // ENABLE_DEBUG_LOGGING

#endif /* FQUADLOGGING_H_ */