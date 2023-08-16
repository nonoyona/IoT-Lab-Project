#include "GPSInterface.h"
#include <string.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <xdc/runtime/Error.h>
#include <stdbool.h>

UART_Handle uart;


#define GPS_TASK_STACK_SIZE 400
#define GPS_TASK_PRIORITY 1

Task_Struct gpsTask;
static Task_Params gpsTaskParams;
static uint8_t gpsTaskStack[GPS_TASK_STACK_SIZE];

int32_t latitude;
int32_t longitude;
bool read = false;

static Semaphore_Handle valueSemaphore;

double dmsToDecimal(int degrees, int minutes) {
    return degrees + (double)minutes / 6000;
}

void uartCallback(UART_Handle handle, void *buf, size_t count){
    char prefix[] = "$GPRMC";
    read = true;
    if(strncmp(buf, prefix, sizeof(prefix)) == 0){
            char *token = strtok(buffer, ",");
            int i = 0;
            Semaphore_pend(valueSemaphore, BIOS_WAIT_FOREVER);
            while(token != NULL){
                if(i == 3){
                    degrees = (token[0] - '0') * 10;
                    degrees += (token[1] - '0');
                    minutes = (token[2] - '0') * 1000;
                    minutes += (token[3] - '0') *  100;
                    minutes = (token[5] - '0') * 10;
                    minutes += (token[6] - '0');
                    latitude = (int32_t)(dmsToDecimal(degrees, minutes) * 10000000L);
                }
                if(i == 5){
                    degrees = (token[0] - '0') * 100;
                    degrees += (token[1] - '0') * 10;
                    degrees += (token[2] - '0');
                    minutes = (token[3] - '0') * 1000;
                    minutes += (token[4] - '0') *  100;
                    minutes = (token[6] - '0') * 10;
                    minutes += (token[7] - '0');
                    longitude = (int32_t)(dmsToDecimal(degrees, minutes) * 10000000L);
                }
                token = strtok(NULL, ",");
                i++;
            }
            Semaphore_post(valueSemaphore);
        }

}

void taskGPSfnx(UArg a0, UArg a1)
{
    uint8_t buffer[256];
    int degrees = 0;
    int minutes = 0;
    while (1)
    {
        read = false;
        UART_read(uart, &buffer, 256);
        Task_sleep(100);
        if(!read){
            UART_readCancel(uart);
        }
    }
}

void GPSInit(void)
{
    UART_Params uartParams;
    UART_Params_init(&uartParams);
    uartParams.baudRate = 9600;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.readReturnMode = UART_RETURN_NEWLINE;
    uartParams.readMode = UART_MODE_CALLBACK;
    uart = UART_open(Board_UART0, &uartParams);

    Task_Params_init(&gpsTaskParams);
    gpsTaskParams.stackSize = GPS_TASK_STACK_SIZE;
    gpsTaskParams.priority = GPS_TASK_PRIORITY;
    gpsTaskParams.stack = &gpsTaskStack;

    Semaphore_Params semaphore_params;
    Semaphore_Params_init(&semaphore_params);
    Error_Block eb;
    Error_init(&eb);

    valueSemaphore = Semaphore_create(1, &semaphore_params, &eb);
    if (valueSemaphore == NULL)
    {
        System_abort("Semaphore creation failed");
    }

    Task_construct(&gpsTask, taskGPSfnx, &gpsTaskParams, NULL);

}

void GPSRead(uint32_t *lat, uint32_t *lon)
{
    Semaphore_pend(valueSemaphore, BIOS_WAIT_FOREVER);
    *lat = latitude;
    *lon = longitude;
    Semaphore_post(valueSemaphore);
}
