#include <stdint.h>

/*
 * Endian utils
 */

uint32_t    getu32_be(const uint8_t *data);
uint32_t    getu24_be(const uint8_t *data);
uint32_t    getu16_be(const uint8_t *data);

uint32_t    getu32_le(const uint8_t *data);
uint32_t    getu24_le(const uint8_t *data);
uint32_t    getu16_le(const uint8_t *data);
