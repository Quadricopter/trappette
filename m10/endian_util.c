#include "endian_util.h"

/*
 * Endian utils
 */

uint32_t    getu32_be(const uint8_t *data)
{
    return data[0]<<24 | data[1]<<16 | data[2]<<8 | data[3];
}

uint32_t    getu24_be(const uint8_t *data)
{
    return data[0]<<16 | data[1]<<8 | data[2];
}

uint32_t    getu16_be(const uint8_t *data)
{
    return data[0]<<8 | data[1];
}

uint32_t    getu32_le(const uint8_t *data)
{
    return data[3]<<24 | data[2]<<16 | data[1]<<8 | data[0];
}

uint32_t    getu24_le(const uint8_t *data)
{
    return data[2]<<16 | data[1]<<8 | data[0];
}

uint32_t    getu16_le(const uint8_t *data)
{
    return data[1]<<8 | data[0];
}
