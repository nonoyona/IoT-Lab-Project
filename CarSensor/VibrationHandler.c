#include "VibrationHandler.h"
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <xdc/runtime/Error.h>
#include "sensors/SensorMpu9250.h"
#include "sensors/SensorUtil.h"
#include "sensors/SensorI2C.h"

#define VH_TASK_STACK_SIZE 512
#define VH_TASK_PRIORITY 2
#define MS_TO_US 1000

Task_Struct vhTask;
static Task_Params vhTaskParams;
static uint8_t vhTaskStack[VH_TASK_STACK_SIZE];

static Semaphore_Handle valueSemaphore;

static uint32_t vibration = 0;

void taskVHfnx(UArg a0, UArg a1)
{
    if (SensorI2C_open())
    {
        SensorMpu9250_init();
        SensorMpu9250_powerOn();
        SensorMpu9250_enable(0x0038);
    }

    while (1)
    {
        Task_sleep(100);
        bool success;
        uint16_t data[3];
        success = SensorMpu9250_accRead(data);
        if (success)
        {
            float z = SensorMpu9250_accConvert(data[2]) + 1;
            if (z < 0)
            {
                z = -z;
            }
            Semaphore_pend(valueSemaphore, BIOS_WAIT_FOREVER);
            vibration *= 1000;
            vibration += (uint32_t)(z * 100000);
            vibration /= 1001;
            Semaphore_post(valueSemaphore);
        }
    }
}

void VHinit()
{
    Task_Params_init(&vhTaskParams);
    vhTaskParams.stackSize = VH_TASK_STACK_SIZE;
    vhTaskParams.priority = VH_TASK_PRIORITY;
    vhTaskParams.stack = &vhTaskStack;

    Semaphore_Params semaphore_params;
    Semaphore_Params_init(&semaphore_params);
    Error_Block eb;
    Error_init(&eb);

    valueSemaphore = Semaphore_create(1, &semaphore_params, &eb);
    if (valueSemaphore == NULL)
    {
        System_abort("Semaphore creation failed");
    }

    Task_construct(&vhTask, taskVHfnx, &vhTaskParams, NULL);
}

uint32_t VHgetVibration()
{
    Semaphore_pend(valueSemaphore, BIOS_WAIT_FOREVER);
    uint32_t value = vibration;
    Semaphore_post(valueSemaphore);
    return value;
}
