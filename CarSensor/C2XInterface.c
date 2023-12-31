#include "C2XInterface.h"
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Semaphore.h>
#include "RadioDriver.h"

#define C2X_TASK_STACK_SIZE 700
#define C2X_TASK_PRIORITY 2
#define MS_TO_US 1000
#define C2X_DATA_BUFFER_SIZE 512

#define C2X_PACKET_MISS_THRESHOLD 10

static Semaphore_Handle bufferSemaphore;

Task_Struct c2xTask;
static Task_Params c2xTaskParams;
static uint8_t c2xTaskStack[C2X_TASK_STACK_SIZE];

static struct C2XData dataBuffer[C2X_DATA_BUFFER_SIZE];
static uint16_t dataBufferIndex = 0;

bool hasConnection = false;
uint8_t infrastrucutreAddress;

/**
 * Sends the current vibration to the other board in the car such that it can be displayed on the screen
 *
 */
static void sendDiagnostics(struct C2XData data)
{
    struct RDPacket packet = {0};
    packet.type = 0xDA;
    packet.destinatonAddress = RD_BROADCAST_ADDRESS;
    serializeData(&data, packet.data);
    packet.data[12] = (uint8_t)hasConnection;
    packet.data[13] = dataBufferIndex >> 8;
    packet.data[14] = dataBufferIndex;
    RDsendPacket(&packet);
}

void serializeData(struct C2XData *data, uint8_t *payload)
{
    payload[0] = data->latitude >> 24;
    payload[1] = data->latitude >> 16;
    payload[2] = data->latitude >> 8;
    payload[3] = data->latitude;
    payload[4] = data->longitude >> 24;
    payload[5] = data->longitude >> 16;
    payload[6] = data->longitude >> 8;
    payload[7] = data->longitude;
    payload[8] = data->vibration >> 24;
    payload[9] = data->vibration >> 16;
    payload[10] = data->vibration >> 8;
    payload[11] = data->vibration;
}

void sendConnectionRequest()
{
    struct RDPacket packet = {0};
    packet.type = 0xC0;
    packet.destinatonAddress = 0xFF;
    packet.data[0] = dataBufferIndex / 100 + '0';
    packet.data[1] = ((dataBufferIndex % 100) / 10) + '0';
    packet.data[2] = (dataBufferIndex % 10) + '0';
    RDsendPacket(&packet);
}

void tryReceiveAccept()
{
    struct RDPacket packet;
    enum RDStatus status = RDreceivePacket(&packet);
    if (status == RD_OK)
    {
        if (packet.type == 0xAC)
        {
            hasConnection = true;
            infrastrucutreAddress = packet.sourceAddress;
        }
    }
}

static void taskC2XFnx(UArg arg0, UArg arg1)
{
    while (1)
    {
        if (!hasConnection)
        {
            if (dataBufferIndex > 0)
            {
                // If there is no connection, try to connect every millisecond
                sendConnectionRequest();
                tryReceiveAccept();
                Task_sleep((1 * MS_TO_US) / Clock_tickPeriod);
            }
            else
            {
                Task_sleep((100 * MS_TO_US) / Clock_tickPeriod);
            }
        }
        else
        {
            Semaphore_pend(bufferSemaphore, BIOS_WAIT_FOREVER);
            if (dataBufferIndex > 0)
            {
                struct RDPacket packet = {0};
                packet.type = 0xDA;
                packet.destinatonAddress = infrastrucutreAddress;
                serializeData(&dataBuffer[0], packet.data);
                enum RDStatus status = RDsendPacketAck(&packet, 5);
                if (status == RD_OK)
                {
                    dataBufferIndex--;
                    uint16_t j;
                    for (j = 0; j < dataBufferIndex; j++)
                    {
                        dataBuffer[j] = dataBuffer[j + 1];
                    }
                }
                else
                {
                    hasConnection = false;
                }
            }
            else
            {
                hasConnection = false;
            }
            Semaphore_post(bufferSemaphore);
        }
    }
}

void C2Xinit(void)
{
    Task_Params_init(&c2xTaskParams);
    c2xTaskParams.stackSize = C2X_TASK_STACK_SIZE;
    c2xTaskParams.priority = C2X_TASK_PRIORITY;
    c2xTaskParams.stack = &c2xTaskStack;
    c2xTaskParams.arg0 = (UInt)1000000;

    Semaphore_Params semaphore_params;
    Semaphore_Params_init(&semaphore_params);
    Error_Block eb;
    Error_init(&eb);

    bufferSemaphore = Semaphore_create(1, &semaphore_params, &eb);
    if (bufferSemaphore == NULL)
    {
        System_abort("Semaphore creation failed");
    }

    Task_construct(&c2xTask, taskC2XFnx, &c2xTaskParams, NULL);
}

void C2XputData(struct C2XData data)
{
    Semaphore_pend(bufferSemaphore, BIOS_WAIT_FOREVER);
    if (dataBufferIndex < C2X_DATA_BUFFER_SIZE)
    {
        dataBuffer[dataBufferIndex] = data;
        dataBufferIndex++;
    }
    sendDiagnostics(data);
    Semaphore_post(bufferSemaphore);
}
