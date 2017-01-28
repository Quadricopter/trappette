#ifndef __FILTER_H__
#define __FILTER_H__

#include <stdint.h>

/*
 * Q16.16 Fixed point
 */

#define q16_t   int32_t

void    low_pass_filter_q16(uint8_t a, q16_t *lp, q16_t u);
void    high_pass_filter_q16(uint8_t a, q16_t *hp, q16_t u, q16_t *u_mem);


#endif /*__FILTER_H__*/
