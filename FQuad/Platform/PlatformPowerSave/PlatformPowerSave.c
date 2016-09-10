/*
 * PlatformPowerSave.c
 *
 * Created: 2016-09-10 7:00:39 PM
 *  Author: Felix
 */ 

#include "PlatformPowerSave.h"
#include "require_macros.h"
#include <avr/io.h>

PlatformStatus PlatformPowerSave_PowerOnAllDomains( void )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	for ( uint8_t i = 0; i < PlatformPowerSaveDomain_MaxDomains; i++ )
	{
		status = PlatformPowerSave_PowerOnDomain( i );
		require_noerr_quiet( status, exit );
	}
	
exit:
	return status;
}

PlatformStatus PlatformPowerSave_PowerOffAllDomains( void )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	for ( uint8_t i = 0; i < PlatformPowerSaveDomain_MaxDomains; i++ )
	{
		status = PlatformPowerSave_PowerOffDomain( i );
		require_noerr_quiet( status, exit );
	}

exit:
	return status;
}

PlatformStatus PlatformPowerSave_PowerOnDomain( PlatformPowerSaveDomain_t inDomain )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	switch ( inDomain )
	{
		case PlatformPowerSaveDomain_ADC:
		{
			PRR &= ~( 1 << PRADC );
			break;
		}
		case PlatformPowerSaveDomain_USART:
		{
			PRR &= ~( 1 << PRUSART0 );
			break;
		}
		case PlatformPowerSaveDomain_SPI:
		{
			PRR &= ~( 1 << PRSPI );
			break;
		}
		case PlatformPowerSaveDomain_I2C:
		{
			PRR &= ~( 1 << PRTWI );
			break;
		}
		case PlatformPowerSaveDomain_Timer0:
		{
			PRR &= ~( 1 << PRTIM0 );
			break;
		}
		case PlatformPowerSaveDomain_Timer1:
		{
			PRR &= ~( 1 << PRTIM1 );
			break;
		}
		case PlatformPowerSaveDomain_Timer2:
		{
			PRR &= ~( 1 << PRTIM2 );
			break;
		}
		default :
		{
			goto exit;
		}
	}
	
	status = PlatformStatus_Success;
exit:
	return status;
}

PlatformStatus PlatformPowerSave_PowerOffDomain( PlatformPowerSaveDomain_t inDomain )
{
	PlatformStatus status = PlatformStatus_Failed;
	
	switch ( inDomain )
	{
		case PlatformPowerSaveDomain_ADC:
		{
			PRR |= 1 << PRADC;
			break;
		}
		case PlatformPowerSaveDomain_USART:
		{
			PRR |= 1 << PRUSART0;
			break;
		}
		case PlatformPowerSaveDomain_SPI:
		{
			PRR |= 1 << PRSPI;
			break;
		}
		case PlatformPowerSaveDomain_I2C:
		{
			PRR |= 1 << PRTWI;
			break;
		}
		case PlatformPowerSaveDomain_Timer0:
		{
			PRR |= 1 << PRTIM0;
			break;
		}
		case PlatformPowerSaveDomain_Timer1:
		{
			PRR |= 1 << PRTIM1;
			break;
		}
		case PlatformPowerSaveDomain_Timer2:
		{
			PRR |= 1 << PRTIM2;
			break;
		}
		default :
		{
			goto exit;
		}
	}
	
	status = PlatformStatus_Success;
exit:
	return status;
}