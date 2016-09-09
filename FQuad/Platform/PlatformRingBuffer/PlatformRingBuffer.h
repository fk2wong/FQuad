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

PlatformStatus PlatformRingBuffer_Write( PlatformRingBuffer *const inRingBuffer, 
										 const uint8_t *const      inData, 
										 size_t                    inDataLen );

PlatformStatus PlatformRingBuffer_Read( PlatformRingBuffer *const inRingBuffer,
										uint8_t **const           outData,
										const size_t              inRequestedLen,
										size_t *const             outActualLen );

#endif /* PLATFORMRINGBUFFER_H_ */