#ifndef __TSIP_H__
#define __TSIP_H__

#include <stdint.h>
#include "trappette_sdk.h"

#define TSIP_PACKET_SIZE    101

/*
 *
 */

int TSIP_stream2Struct(decoded_position_t *, const uint8_t *pStream, uint8_t size);

#endif /*__TSIP_H__*/
