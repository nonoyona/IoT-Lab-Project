#include "C2XInterface.h"
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Semaphore.h>
#include "RadioDriver.h"
#include "Utils.h"

#define C2X_TASK_STACK_SIZE 1024
#define C2X_TASK_PRIORITY 2
#define MS_TO_US 1000
#define C2X_DATA_BUFFER_SIZE 32

#define C2X_PACKET_MISS_THRESHOLD 10

static Semaphore_Handle bufferSemaphore;

Task_Struct txTask;
static Task_Params txTaskParams;
static uint8_t txTaskStack[C2X_TASK_STACK_SIZE];

static struct C2XData dataBuffer[C2X_DATA_BUFFER_SIZE];
static uint16_t dataBufferIndex = 0;

bool hasConnection = false;
uint16_t packetMisses = 0;

void deserializeData(struct C2XData *data, uint8_t *payload)
{
    data->latitude = payload[0] << 24 | payload[1] << 16 | payload[2] << 8 | payload[3];
    data->longitude = payload[4] << 24 | payload[5] << 16 | payload[6] << 8 | payload[7];
    data->vibration = payload[8] << 24 | payload[9] << 16 | payload[10] << 8 | payload[11];
}

void handlePacket()
{
    struct RDPacket packet;
    enum RDStatus status = RDreceivePacket(&packet);
    if (status == RD_OK)
    {
        packetMisses = 0;
        struct RDPacket response = {0};
        struct C2XData data = {0};
        switch (packet.type)
        {
        case 0xC0:

            response.type = 0xAC;
            response.destinatonAddress = packet.sourceAddress;
            RDsendPacketAck(&response, 5);
            hasConnection = true;
            Display_printf(display, 0, 0, "//Connection established by %d", packet.sourceAddress);
            break;
        case 0xDA:

            deserializeData(&data, packet.data);
            Display_printf(display, 0, 0, "//Data sent by %d is %d", packet.sourceAddress, data.vibration);
            Semaphore_pend(bufferSemaphore, BIOS_WAIT_FOREVER);
            if (dataBufferIndex < C2X_DATA_BUFFER_SIZE)
            {
                dataBuffer[dataBufferIndex] = data;
                dataBufferIndex++;
            }
            else
            {
                // Data is discarded silently
            }
            Semaphore_post(bufferSemaphore);
            hasConnection = true; // Probably a connection if we are receiving data
            break;
        default:
            break;
        }
    }
    else
    {
        packetMisses++;
        if (packetMisses > C2X_PACKET_MISS_THRESHOLD)
        {
            hasConnection = false;
        }
    }
}

static void taskC2XFnx(UArg arg0, UArg arg1)
{
    while (1)
    {
        if (!hasConnection)
        {
            // If there is no connection, wait 10ms before checking for packets

            Task_sleep((10 * MS_TO_US) / Clock_tickPeriod);
        }
        handlePacket();
        Task_yield();
    }
}

void C2Xinit(void)
{
    Task_Params_init(&txTaskParams);
    txTaskParams.stackSize = C2X_TASK_STACK_SIZE;
    txTaskParams.priority = C2X_TASK_PRIORITY;
    txTaskParams.stack = &txTaskStack;
    txTaskParams.arg0 = (UInt)1000000;

    Semaphore_Params semaphore_params;
    Semaphore_Params_init(&semaphore_params);
    Error_Block eb;
    Error_init(&eb);

    bufferSemaphore = Semaphore_create(1, &semaphore_params, &eb);
    if (bufferSemaphore == NULL)
    {
        System_abort("Semaphore creation failed");
    }

    Task_construct(&txTask, taskC2XFnx, &txTaskParams, NULL);
}

uint16_t C2XgetNumData()
{
    return dataBufferIndex;
}

uint16_t C2XgetData(struct C2XData *buffer, uint16_t len)
{
    Semaphore_pend(bufferSemaphore, BIOS_WAIT_FOREVER);
    uint16_t i;
    for (i = 0; i < len && i < dataBufferIndex; i++)
    {
        buffer[i] = dataBuffer[i];
    }
    dataBufferIndex = 0;
    Semaphore_post(bufferSemaphore);
    return i;
}
