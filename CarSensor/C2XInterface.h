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

void C2XputData(struct C2XData data);

#endif // _C2XINTERFACE_H_
