/*
 * PlatformRingBuffer.c
 *
 * Created: 2016-09-08 7:57:40 AM
 *  Author: Felix
 */ 

#include "PlatformRingBuffer.h"
#include "FMemory.h"
#include "require_macros.h"

struct PlatformRingBufferStruct
{
	size_t   bufferSize;
	uint32_t tailIndex;
	uint32_t headIndex;
};

PlatformRingBuffer * PlatformRingBuffer_Create( size_t inBufferSize )
{
	PlatformRingBuffer* newRingBuf = NULL;
	
	require_quiet( inBufferSize <= PLATFORM_RING_BUFFER_MAX_SIZE, exit );
	
	// Allocate new ring buffer
	newRingBuf = FMemoryAlloc( inBufferSize );
	require_quiet( newRingBuf, exit );
	
	// Initialize internal struct
	newRingBuf->bufferSize = inBufferSize;
	newRingBuf->headIndex  = 0;
	newRingBuf->tailIndex  = 0;
	
exit:
	return newRingBuf;
}