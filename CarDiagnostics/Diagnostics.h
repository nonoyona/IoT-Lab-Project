#ifndef _DIAGNOSTICS_H_
#define _DIAGNOSTICS_H_

#include <stdint.h>

struct DiagnosticsData
{
    int32_t latitude;
    int32_t longitude;
    uint32_t vibration;
    bool hasConnection;
    uint16_t bufferLevel;
};

enum DiagnosticsStatus
{
    DIAGNOSTICS_STATUS_OK,
    DIAGNOSTICS_STATUS_ERROR
};

// Receives diagnostics data from the CarSensor via RF
enum DiagnosticsStatus DiagnosticsReceiveData(struct DiagnosticsData *data);

// Sends diagnostics data to the Serial Monitor
void DiagnosticsSendData(struct DiagnosticsData *data);

#endif // _DIAGNOSTICS_H_
