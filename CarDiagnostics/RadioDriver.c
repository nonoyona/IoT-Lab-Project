#include "RadioDriver.h"

static uint16_t sequenceNumber = 0;

static EasyLink_RxPacket latestRxPacket;
static EasyLink_Status latestRxStatus;

Event_Struct rxEvent;
static Event_Handle rxEventHandle;
#define RX_EVENT_RECEIVED 0x1
#define RX_TIMEOUT_MS 1000
#define RX_ACK_TIMEOUT_MS 100

static Semaphore_Handle radioSemaphore;

void RDserializePacket(EasyLink_TxPacket *txPacket, struct RDPacket *rdPacket)
{
    txPacket->payload[0] = rdPacket->sequenceNumber >> 8;
    txPacket->payload[1] = rdPacket->sequenceNumber & 0xFF;
    txPacket->payload[2] = (rdPacket->requestAck & 0x01) | ((rdPacket->isAck & 0x01) << 1);
    txPacket->payload[3] = rdPacket->type;
    txPacket->payload[4] = rdPacket->sourceAddress;
    txPacket->payload[5] = rdPacket->destinatonAddress;
    txPacket->dstAddr[0] = 0xAA;
    uint16_t i;
    for (i = 0; i < RD_DATA_LENGTH; i++)
    {
        txPacket->payload[6 + i] = rdPacket->data[i];
    }
    txPacket->len = 6 + RD_DATA_LENGTH;
}

void RDdeserializePacket(EasyLink_RxPacket *rxPacket, struct RDPacket *rdPacket)
{
    rdPacket->sequenceNumber = (rxPacket->payload[0] << 8) | rxPacket->payload[1];
    rdPacket->requestAck = (rxPacket->payload[2] & 0x01);
    rdPacket->isAck = (rxPacket->payload[2] & 0x02) >> 1;
    rdPacket->type = rxPacket->payload[3];
    rdPacket->sourceAddress = rxPacket->payload[4];
    rdPacket->destinatonAddress = rxPacket->payload[5];
    uint16_t i;
    for (i = 0; i < RD_DATA_LENGTH; i++)
    {
        rdPacket->data[i] = rxPacket->payload[6 + i];
    }
}

void rxDoneCallback(EasyLink_RxPacket *rxPacket, EasyLink_Status status)
{
    latestRxPacket = *rxPacket;
    latestRxStatus = status;
    Event_post(rxEventHandle, RX_EVENT_RECEIVED);
}

enum RDStatus sendPacketInternally(struct RDPacket *packet, uint8_t maxRetries)
{
    EasyLink_TxPacket txPacket = {{0}, 0, 0, {0}};
    RDserializePacket(&txPacket, packet);

    EasyLink_Status result = EasyLink_transmit(&txPacket);

    if (result != EasyLink_Status_Success)
    {
        return RD_ERROR;
    }

    uint8_t retries = 0;

    while (retries < maxRetries)
    {
        EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut,
                         EasyLink_ms_To_RadioTime(RX_ACK_TIMEOUT_MS));
        EasyLink_receiveAsync(&rxDoneCallback, 0);
        uint32_t events = Event_pend(rxEventHandle, 0, RX_EVENT_RECEIVED, BIOS_WAIT_FOREVER);

        if (latestRxStatus == EasyLink_Status_Success)
        {
            struct RDPacket receivedPacket = {0};
            RDdeserializePacket(&latestRxPacket, &receivedPacket);

            if (receivedPacket.sequenceNumber == packet->sequenceNumber && receivedPacket.isAck == 1 && receivedPacket.destinatonAddress == RD_ADDRESS)
            {

                return RD_OK;
            }
            else
            {
                EasyLink_Status result = EasyLink_transmit(&txPacket);

                if (result != EasyLink_Status_Success)
                {
                    return RD_ERROR;
                }
            }
        }
        else
        {
            EasyLink_Status result = EasyLink_transmit(&txPacket);

            if (result != EasyLink_Status_Success)
            {
                return RD_ERROR;
            }
        }
        retries++;
    }
    if (maxRetries == 0)
    {
        return RD_OK;
    }

    return RD_TIMEOUT;
}

enum RDStatus RDsendPacketAck(struct RDPacket *packet, uint8_t maxRetries)
{
    Semaphore_pend(radioSemaphore, BIOS_WAIT_FOREVER);
    packet->isAck = 0;
    packet->requestAck = 1;
    packet->sourceAddress = RD_ADDRESS;
    packet->sequenceNumber = sequenceNumber;
    enum RDStatus status = sendPacketInternally(packet, maxRetries);
    sequenceNumber++;
    Semaphore_post(radioSemaphore);
    return status;
}

enum RDStatus RDsendPacket(struct RDPacket *packet)
{
    Semaphore_pend(radioSemaphore, BIOS_WAIT_FOREVER);
    packet->isAck = 0;
    packet->requestAck = 0;
    packet->sourceAddress = RD_ADDRESS;
    packet->sequenceNumber = sequenceNumber;
    enum RDStatus status = sendPacketInternally(packet, 0);
    sequenceNumber++;
    Semaphore_post(radioSemaphore);
    return status;
}

enum RDStatus RDreceivePacket(struct RDPacket *packet)
{

    Semaphore_pend(radioSemaphore, BIOS_WAIT_FOREVER);

    EasyLink_setCtrl(EasyLink_Ctrl_AsyncRx_TimeOut,
                     EasyLink_ms_To_RadioTime(RX_TIMEOUT_MS));
    EasyLink_receiveAsync(&rxDoneCallback, 0);
    uint32_t events = Event_pend(rxEventHandle, 0, RX_EVENT_RECEIVED, BIOS_WAIT_FOREVER);

    if (latestRxStatus == EasyLink_Status_Success)
    {
        struct RDPacket receivedPacket = {0};
        RDdeserializePacket(&latestRxPacket, &receivedPacket);
        if (receivedPacket.isAck == 1)
        {
            // Ignore packet
            Semaphore_post(radioSemaphore);
            return RD_TIMEOUT;
        }
        else if (receivedPacket.requestAck == 1 && receivedPacket.destinatonAddress == RD_ADDRESS)
        {
            memcpy(packet, &receivedPacket, sizeof(struct RDPacket));
            // Send ack
            receivedPacket.isAck = 1;
            receivedPacket.requestAck = 0;
            receivedPacket.destinatonAddress = receivedPacket.sourceAddress;
            receivedPacket.sourceAddress = RD_ADDRESS;
            /* wait for other device ready */
            Task_sleep(2);
            enum RDStatus status = sendPacketInternally(&receivedPacket, 0);
            Semaphore_post(radioSemaphore);
            return status;
        }
        else if (receivedPacket.destinatonAddress == RD_ADDRESS || receivedPacket.destinatonAddress == RD_BROADCAST_ADDRESS)
        {
            memcpy(packet, &receivedPacket, sizeof(struct RDPacket));
            Semaphore_post(radioSemaphore);
            return RD_OK;
        }
    }
    Semaphore_post(radioSemaphore);
    return RD_TIMEOUT;
}

void RDinitialize()
{

    Semaphore_Params semaphore_params;
    Semaphore_Params_init(&semaphore_params);
    Error_Block eb;
    Error_init(&eb);

    radioSemaphore = Semaphore_create(1, &semaphore_params, &eb);
    if (radioSemaphore == NULL)
    {
        System_abort("Semaphore creation failed");
    }

    EasyLink_Params easyLink_params;
    EasyLink_Params_init(&easyLink_params);

    if (EasyLink_init(&easyLink_params) != EasyLink_Status_Success)
    {
        System_abort("EasyLink_init failed");
    }

    Event_Params eventParam;
    Event_Params_init(&eventParam);
    Event_construct(&rxEvent, &eventParam);
    rxEventHandle = Event_handle(&rxEvent);
}
