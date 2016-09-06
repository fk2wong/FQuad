/**
 * \file
 *
 * \brief User board configuration template
 *
 */
 /**
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#ifndef CONF_BOARD_H
#define CONF_BOARD_H

#include "ioport.h"

// System Clock Frequency
#define F_CPU 16000000

#define FQUAD_TEST_LED IOPORT_CREATE_PIN( PORTB, 5 )

#endif // CONF_BOARD_H
