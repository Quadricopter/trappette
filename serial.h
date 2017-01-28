#ifndef __SERIAL_H__
#define __SERIAL_H__

#ifdef WIN32

#include <windows.h>

typedef struct  {

    HANDLE hPort;
}   serial_t;

#else

#define READ_CACHE_SIZE 128

typedef struct  {

    int     fd;
    char    *szDevice;
}   serial_t;

#endif /*WIN32*/


/*
 *
 */

typedef enum {

    SERIAL_OK = 0,
    SERIAL_OPEN_FAILED,
    SERIAL_CONFIG_FAILED,
#ifdef WIN32
    SERIAL_SETSTATE_FAILED,
    SERIAL_GETSTATE_FAILED,
    SERIAL_SETTIMEOUT_FAILED,
#endif
    SERIAL_READ_FAILED,
    SERIAL_WRITE_FAILED
} ESerial;

/*
**
*/

#ifdef __cplusplus
extern "C" {
#endif

//serial_t *new_serial(void);
ESerial serial_init(serial_t *pSerial);
ESerial serial_open(serial_t *pSerial, const char *szDevice, int speed);
ESerial serial_close(serial_t *pSerial);
int     serial_read(serial_t *pSerial, char *buff, int maxsize);
int     serial_write(serial_t *pSerial, const char *buff, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* __SERIAL_H__ */
