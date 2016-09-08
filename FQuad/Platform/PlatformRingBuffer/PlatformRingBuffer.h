/*
 * PlatformRingBuffer.h
 *
 * Created: 2016-09-08 7:57:11 AM
 *  Author: Felix
 */ 


#ifndef PLATFORMRINGBUFFER_H_
#define PLATFORMRINGBUFFER_H_

#include <avr/io.h>
#include <stddef.h>

#define PLATFORM_RING_BUFFER_MAX_SIZE ( 512 )

typedef struct PlatformRingBufferStruct PlatformRingBuffer;


PlatformRingBuffer * PlatformRingBuffer_Create( size_t inBufferSize );

#endif /* PLATFORMRINGBUFFER_H_ */