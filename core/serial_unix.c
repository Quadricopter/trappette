#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "serial.h"

/*serial_t *new_serial(void)
{
    serial_t *pSerial;

    pSerial = malloc(sizeof(serial_t));
    memset(pSerial, 0, sizeof(serial_t));

    return pSerial;
}*/

ESerial serial_init(serial_t *pSerial)
{
    memset(pSerial, 0, sizeof(serial_t));
    return SERIAL_OK;
}

ESerial serial_open(serial_t *pSerial, const char *szDevice, int speed)
{
    unsigned short speed_flag;
    struct termios options;

#ifdef NON_CANONICAL_SERIAL_INPUT
    pSerial->fd = open(szDevice, O_RDWR | O_NOCTTY | O_NDELAY);
#else
    pSerial->fd = open(szDevice, O_RDWR | O_NOCTTY);
#endif
    if (pSerial->fd < 0) {

        perror(szDevice);
        return SERIAL_OPEN_FAILED;
    }
#ifdef NON_CANONICAL_SERIAL_INPUT
    fcntl(pSerial->fd, F_SETFL, 0);
#endif

//    tcgetattr(pSerial->fd, &options);
    bzero(&options, sizeof(struct termios));
    speed_flag = 0;
    switch (speed) {
    case 4800:
        speed_flag = B4800;
        break;
    case 9600:
        speed_flag = B9600;
        break;
    case 19200:
        speed_flag = B19200;
        break;
    case 38400:
        speed_flag = B38400;
        break;
    case 57600:
        speed_flag = B57600;
        break;
    default:
        speed_flag = B115200;
    }
    cfsetispeed(&options, speed_flag);
    cfsetospeed(&options, speed_flag);

#ifdef NON_CANONICAL_SERIAL_INPUT
    /* set raw input, 128 byte min, 1 second timeout */
    options.c_cflag     |= (CS8 | CLOCAL | CREAD);
    options.c_lflag     &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag     &= ~OPOST;
    options.c_cc[VMIN]  = READ_CACHE_SIZE;
    options.c_cc[VTIME] = 10;
#else
    options.c_cflag     |= (CS8 | CLOCAL | CREAD);
    options.c_lflag     = ICANON;
    options.c_oflag     = 0;
    options.c_iflag     = IGNPAR;
//    options.c_cc[VTIME] = 0;
//    options.c_cc[VMIN]  = 255;
#endif

    tcflush(pSerial->fd, TCIFLUSH);
    tcsetattr(pSerial->fd, TCSANOW, &options);

    return SERIAL_OK;
}

ESerial serial_close(serial_t *pSerial)
{
    close(pSerial->fd);

    return SERIAL_OK;
}

int	serial_read(serial_t *pSerial, char *buff, int maxsize)
{
    int	res;

    res = read(pSerial->fd, buff, maxsize);
    buff[res] = 0;

    return res;
}

int	serial_write(serial_t *pSerial, const char *buff, size_t len)
{
    return write(pSerial->fd, buff, len);
}
