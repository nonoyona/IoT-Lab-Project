#ifndef _GPSINTERFACE_H_
#define _GPSINTERFACE_H_

#include <ti/drivers/UART.h>
#include "Board.h"
#include <stdint.h>

void GPSInit(void);

void GPSRead(uint32_t *latitude, uint32_t *longitude);


#endif // _GPSINTERFACE_H_
