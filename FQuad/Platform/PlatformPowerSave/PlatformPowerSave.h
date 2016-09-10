/*
 * PlatformPowerSave.h
 *
 * Created: 2016-09-10 7:00:27 PM
 *  Author: Felix
 */ 


#ifndef PLATFORMPOWERSAVE_H_
#define PLATFORMPOWERSAVE_H_

#include "PlatformStatus.h"
 
typedef enum 
{
	PlatformPowerSaveDomain_ADC = 0,
	PlatformPowerSaveDomain_USART,
	PlatformPowerSaveDomain_SPI,
	PlatformPowerSaveDomain_Timer0,
	PlatformPowerSaveDomain_Timer1,
	PlatformPowerSaveDomain_Timer2,
	PlatformPowerSaveDomain_I2C,
	PlatformPowerSaveDomain_MaxDomains,
} PlatformPowerSaveDomain_t;

PlatformStatus PlatformPowerSave_PowerOnAllDomains( void );

PlatformStatus PlatformPowerSave_PowerOffAllDomains( void );

PlatformStatus PlatformPowerSave_PowerOnDomain( PlatformPowerSaveDomain_t inDomain );

PlatformStatus PlatformPowerSave_PowerOffDomain( PlatformPowerSaveDomain_t inDomain );



#endif /* PLATFORMPOWERSAVE_H_ */