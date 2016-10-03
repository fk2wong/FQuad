/*
 * PlatformI2C.h
 *
 * Created: 2016-09-26 11:39:46 PM
 *  Author: Felix
 */ 


#ifndef PLATFORMI2C_H_
#define PLATFORMI2C_H_

#include "PlatformStatus.h"
#include <stdint.h>
#include <stddef.h>

PlatformStatus PlatformI2C_Init( void );

PlatformStatus PlatformI2C_WriteByte( const uint8_t inDeviceAddr, const uint8_t inRegisterAddress, const uint8_t inDataByte );

PlatformStatus PlatformI2C_Write( const uint8_t inDeviceAddr, const uint8_t inRegisterAddress, const uint8_t *const inData, const size_t inDataLen );

PlatformStatus PlatformI2C_Read( const uint8_t inDeviceAddr, const uint8_t inRegisterAddress, uint8_t *const outData, const size_t inDataLen );

PlatformStatus PlatformI2C_Deinit( void );

#endif /* PLATFORMI2C_H_ */