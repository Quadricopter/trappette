#ifndef __CHECKSUM_H__
#define __CHECKSUM_H__

#include <stdint.h>

#define CHECKSUM_LENGTH 2

/*
 *
 */

int isValidM10Checksum(const uint8_t *buff, uint8_t size);


#endif /*__CHECKSUM_H__*/
