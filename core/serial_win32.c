#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "serial.h"

/*serial_t *new_serial(void)
{
    serial_t *pSerial;

    pSerial = malloc( sizeof(serial_t) );
    memset( pSerial, 0, sizeof(serial_t) );

    return pSerial;
}*/

ESerial serial_init(serial_t *pSerial)
{
    memset(pSerial, 0, sizeof(serial_t));
    return SERIAL_OK;
}

ESerial serial_open(serial_t *pSerial, const char *szDevice, int speed)
{
    char buff[16];

    // Check szDevice string syntax
    if (strncmp(szDevice, "com", 3) && strncmp(szDevice, "COM", 3) && ((szDevice[3] < '1') || (szDevice[3] > '9'))) {
        return SERIAL_OPEN_FAILED;
    }
    strcpy(buff, szDevice);

    if (szDevice[4] != 0) {
        if ((szDevice[4] < '0') || (szDevice[4] > '9')) {
            return SERIAL_OPEN_FAILED;
        }
        sprintf(buff, "\\\\.\\%s", szDevice);
    }

    pSerial->hPort = CreateFileA(buff, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
    if (pSerial->hPort == INVALID_HANDLE_VALUE) {
        pSerial->hPort = NULL;
        return SERIAL_OPEN_FAILED;
    }

    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
    if (!GetCommState(pSerial->hPort, &dcbSerialParams)) {
        //could not get the state of the comport
        serial_close(pSerial);
        return SERIAL_GETSTATE_FAILED;
    }
    dcbSerialParams.BaudRate = speed;
    dcbSerialParams.ByteSize=8;
    dcbSerialParams.StopBits=ONESTOPBIT;
    dcbSerialParams.Parity=NOPARITY;
    if(!SetCommState(pSerial->hPort, &dcbSerialParams)){
        //analyse error
        serial_close(pSerial);
        return SERIAL_SETSTATE_FAILED;
    }

    COMMTIMEOUTS timeouts={0};
    timeouts.ReadIntervalTimeout=50;
    timeouts.ReadTotalTimeoutConstant=50;
    timeouts.ReadTotalTimeoutMultiplier=10;
    timeouts.WriteTotalTimeoutConstant=50;
    timeouts.WriteTotalTimeoutMultiplier=10;
    if(!SetCommTimeouts(pSerial->hPort, &timeouts)){
        //handle error
        serial_close(pSerial);
        return SERIAL_SETTIMEOUT_FAILED;
    }

    return SERIAL_OK;
}

ESerial serial_close(serial_t *pSerial)
{
    if (pSerial) {
        if (pSerial->hPort) {
            CloseHandle(pSerial->hPort);
        }
    }

    return SERIAL_OK;
}

int	serial_read(serial_t *pSerial, char *buff, int maxsize)
{
    DWORD dwNumberOfBytesRead;
    if (ReadFile(pSerial->hPort, buff, maxsize, &dwNumberOfBytesRead, NULL) == FALSE) {
//        printLastError();
    }
    return dwNumberOfBytesRead;
}

int	serial_write(serial_t *pSerial, const char *buff, size_t len)
{
    DWORD dwBytesRead = 0;
    if(!WriteFile(pSerial->hPort, (char*)buff, strlen(buff), &dwBytesRead, NULL)) {
//        printLastError();
   }

    return dwBytesRead;
}
