/*
 * PlatformUART.h
 *
 * Created: 2016-09-07 5:27:49 PM
 *  Author: Felix
 */ 


#ifndef PLATFORMUART_H_
#define PLATFORMUART_H_

#include "Platform.h"
#include "PlatformStatus.h"
#include "PlatformRingBuffer.h"
#include <avr/io.h>

PlatformStatus PlatformUART_Init( uint32_t inBaudRate, PlatformRingBuffer *const inOptionalRingBuffer );

PlatformStatus PlatformUART_Transmit( uint8_t* inBuffer, size_t bufferLen );

PlatformStatus PlatformUART_ReceiveBuffer( uint8_t* const outBuffer, size_t inRequestedLen, size_t *const outActualLen );


#endif /* PLATFORMUART_H_ */