/*
 * PlatformRingBuffer.c
 *
 * Created: 2016-09-08 7:57:40 AM
 *  Author: Felix
 */ 

#include "PlatformRingBuffer.h"
#include "FMemory.h"
#include "require_macros.h"
#include "FUtilities.h"
#include <string.h>

struct PlatformRingBufferStruct
{
	size_t   bufferSize;
	uint32_t tailIndex;
	uint32_t headIndex;
	uint8_t *buffer;
};

static inline size_t _PlatformRingBuffer_GetNumFreeBytes( PlatformRingBuffer *const inRingBuffer );
static inline size_t _PlatformRingBuffer_GetNumUsedBytes( PlatformRingBuffer *const inRingBuffer );
static inline void _PlatformRingBuffer_UpdateHeadIndex(   PlatformRingBuffer *const inRingBuffer, size_t inSizeToIncrease );
static inline void _PlatformRingBuffer_UpdateTailIndex(   PlatformRingBuffer *const inRingBuffer, size_t inSizeToIncrease );

PlatformRingBuffer * PlatformRingBuffer_Create( size_t inBufferSize )
{
	PlatformRingBuffer* newRingBuf    = NULL;
	uint8_t*            newByteBuffer = NULL;
	
	require_quiet( inBufferSize <= PLATFORM_RING_BUFFER_MAX_SIZE, exit );
	
	// Allocate the byte buffer
	newByteBuffer = FMemoryAlloc( inBufferSize );
	require_quiet( newByteBuffer, exit );
	
	// Allocate new ring buffer struct
	newRingBuf = FMemoryAlloc( sizeof( struct PlatformRingBufferStruct ));
	require_quiet( newRingBuf, exit );
	
	// Initialize internal struct
	newRingBuf->bufferSize = inBufferSize;
	newRingBuf->headIndex  = 0;
	newRingBuf->tailIndex  = 0;
	newRingBuf->buffer     = newByteBuffer;
	
exit:
	// If allocation for the struct failed but the byte buffer was still created, free it
	if ( !newRingBuf && newByteBuffer )
	{
		FMemoryFreeAndNULLPtr( &newByteBuffer );
	}
	return newRingBuf;
}


PlatformStatus PlatformRingBuffer_Write( PlatformRingBuffer *const inRingBuffer, const uint8_t *const inData, size_t inDataLen )
{
	PlatformStatus status = PlatformStatus_Failed;
	size_t sizeToCopy;
	size_t sizeCopied;
	
	require_quiet( inRingBuffer, exit );
	require_quiet( inData,       exit );
	require_quiet( inDataLen,    exit );
	
	// Check we have enough room in the buffer
	require_quiet( _PlatformRingBuffer_GetNumFreeBytes( inRingBuffer ) >= inDataLen, exit );
	
	// Disable Global Interrupts, if enabled
	
	// Copy the data, no further than final byte slot in the ring buffer
	sizeToCopy = MAX( inDataLen, inRingBuffer->bufferSize - inRingBuffer->headIndex + 1 );
	
	memcpy( &inRingBuffer->buffer[ inRingBuffer->headIndex ], inData, sizeToCopy );
	
	// If there is more data to copy after that, then wrap the buffer around index 0 and copy the rest.
	if ( sizeToCopy < inDataLen )
	{
		sizeCopied = sizeToCopy;
		sizeToCopy = sizeCopied - inDataLen;
		memcpy( &inRingBuffer->buffer[0], &inData[ sizeCopied ], sizeToCopy );
	}

	// Update the head index
	_PlatformRingBuffer_UpdateHeadIndex( inRingBuffer, inDataLen );
	
	// Enable global interrupts, if we disabled them
	
	status = PlatformStatus_Success;
exit:
	return status;
}

PlatformStatus PlatformRingBuffer_Read( PlatformRingBuffer *const inRingBuffer, 
										uint8_t **const           outData, 
										const size_t              inRequestedLen, 
										size_t *const             outActualLen )
{
	PlatformStatus status = PlatformStatus_Failed;
	size_t sizeToCopy;
	
	require_quiet( inRingBuffer,   exit );
	require_quiet( outData,        exit );
	require_quiet( inRequestedLen, exit );
	require_quiet( outActualLen,   exit );
	
	// Disable global interrupts, if enabled
	
	// Copy the data to the outData buffer, either the requested amount or however much we can provide
	sizeToCopy = MIN( inRequestedLen, _PlatformRingBuffer_GetNumUsedBytes( inRingBuffer ));
	
	memcpy( outData, &inRingBuffer->buffer[ inRingBuffer->tailIndex ], sizeToCopy );
	
	// Update the tail ptr
	_PlatformRingBuffer_UpdateTailIndex( inRingBuffer, sizeToCopy );
	
	*outActualLen = sizeToCopy;
	
	// Return sucess only if the requested length of bytes were available
	if ( sizeToCopy == inRequestedLen )
	{
		status = PlatformStatus_Success;
	}
exit:
	return status;
}

static inline size_t _PlatformRingBuffer_GetNumFreeBytes( PlatformRingBuffer *const inRingBuffer )
{
	size_t numUsedBytes = _PlatformRingBuffer_GetNumUsedBytes( inRingBuffer );
	
	return inRingBuffer->bufferSize - numUsedBytes;
}

static inline size_t _PlatformRingBuffer_GetNumUsedBytes( PlatformRingBuffer *const inRingBuffer )
{		
	return (( inRingBuffer->headIndex - inRingBuffer->tailIndex ) % inRingBuffer->bufferSize ) + 1;
}

static inline void _PlatformRingBuffer_UpdateHeadIndex( PlatformRingBuffer *const inRingBuffer, size_t inSizeToIncrease )
{
	inRingBuffer->headIndex = ( inRingBuffer->headIndex + inSizeToIncrease ) % inRingBuffer->bufferSize;
}

static inline void _PlatformRingBuffer_UpdateTailIndex( PlatformRingBuffer *const inRingBuffer, size_t inSizeToIncrease )
{
	inRingBuffer->tailIndex = ( inRingBuffer->tailIndex + inSizeToIncrease ) % inRingBuffer->bufferSize;
}