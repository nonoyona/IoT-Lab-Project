#ifndef _C2XINTERFACE_H_
#define _C2XINTERFACE_H_

#include <stdint.h>

struct C2XData
{
    int32_t latitude;
    int32_t longitude;
    uint32_t vibration;
};

void C2Xinit(void);

uint16_t C2XgetNumData();

uint16_t C2XgetData(struct C2XData *buffer, uint16_t len);

#endif // _C2XINTERFACE_H_
