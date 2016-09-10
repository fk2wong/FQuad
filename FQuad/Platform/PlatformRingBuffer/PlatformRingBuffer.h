/*
 * PlatformRingBuffer.h
 *
 * Created: 2016-09-08 7:57:11 AM
 *  Author: Felix
 */ 


#ifndef PLATFORMRINGBUFFER_H_
#define PLATFORMRINGBUFFER_H_

#include "PlatformStatus.h"
#include <avr/io.h>
#include <stddef.h>

#define PLATFORM_RING_BUFFER_MAX_SIZE ( 512 )

typedef struct PlatformRingBufferStruct PlatformRingBuffer;


PlatformRingBuffer * PlatformRingBuffer_Create( size_t inBufferSize );

PlatformStatus PlatformRingBuffer_WriteBuffer( PlatformRingBuffer *const inRingBuffer, 
											   const uint8_t *const      inData, 
											   size_t                    inDataLen );
										 
PlatformStatus PlatformRingBuffer_WriteByte( PlatformRingBuffer* const inRingBuffer, const uint8_t inData );

PlatformStatus PlatformRingBuffer_ReadBuffer( PlatformRingBuffer *const inRingBuffer,
											  uint8_t *const            outData,
											  const size_t              inRequestedLen );

#endif /* PLATFORMRINGBUFFER_H_ */