#ifndef _RADIODRIVER_H_
#define _RADIODRIVER_H_

#include <stdint.h>
#include "easylink/EasyLink.h"
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Task.h>

#define RD_DATA_LENGTH 32
#define RD_BROADCAST_ADDRESS 0xFF
// The address of this node
#define RD_ADDRESS 0x10

struct RDPacket
{
    uint8_t type;
    uint8_t requestAck;
    uint8_t isAck;
    uint8_t destinatonAddress;
    uint8_t sourceAddress;
    uint8_t sequenceNumber;
    uint8_t data[RD_DATA_LENGTH];
};

enum RDStatus
{
    RD_OK,
    RD_ERROR,
    RD_TIMEOUT,
    RD_INVALID_PACKET,
};

enum RDStatus RDsendPacketAck(struct RDPacket *packet, uint8_t maxRetries);

enum RDStatus RDsendPacket(struct RDPacket *packet);

void RDinitialize();

enum RDStatus RDreceivePacket(struct RDPacket *packet);

#endif
