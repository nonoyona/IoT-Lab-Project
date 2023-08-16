#include "Diagnostics.h"
#include "RadioDriver.h"
#include "Utils.h"

#define DIAGNOSTICS_PACKET_TYPE 0xDA

void deserializePacket(struct DiagnosticsData *data, struct RDPacket *packet)
{
    data->latitude = packet->data[0] << 24 | packet->data[1] << 16 | packet->data[2] << 8 | packet->data[3];
    data->longitude = packet->data[4] << 24 | packet->data[5] << 16 | packet->data[6] << 8 | packet->data[7];
    data->vibration = packet->data[8] << 24 | packet->data[9] << 16 | packet->data[10] << 8 | packet->data[11];
    data->hasConnection = packet->data[12];
    data->bufferLevel = packet->data[13] << 8 | packet->data[14];
}

enum DiagnosticsStatus DiagnosticsReceiveData(struct DiagnosticsData *data)
{
    struct RDPacket packet;
    enum RDStatus status = RDreceivePacket(&packet);
    if (status != RD_OK || packet.type != DIAGNOSTICS_PACKET_TYPE)
    {
        return DIAGNOSTICS_STATUS_ERROR;
    }
    else
    {
        deserializePacket(data, &packet);
        return DIAGNOSTICS_STATUS_OK;
    }
}

void DiagnosticsSendData(struct DiagnosticsData *data)
{
    Display_printf(display, 0, 0, "latitude:%d,longitude:%d,vibration:%d,hasConnection:%d,bufferLevel:%d", data->latitude, data->longitude, data->vibration, data->hasConnection, data->bufferLevel);
}
