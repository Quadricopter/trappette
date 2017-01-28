#include <stdio.h>
#include <stdlib.h>
#include "filter.h"

/*
 * http://en.wikipedia.org/wiki/Low-pass_filter#Simple_infinite_impulse_response_filter
 * α := dt / (RC + dt)
 * y[i] := α * x[i] + (1-α) * y[i-1]
 * y[i] := y[i-1] + α * (x[i] - y[i-1])
 */

void    low_pass_filter_q16(uint8_t a, q16_t *lp, q16_t u)
{
    *lp = *lp + (( u - *lp ) >> a);
}


/*
 * http://en.wikipedia.org/wiki/High-pass_filter#Algorithmic_implementation
 * α := RC / (RC + dt)
 * y[i] := α * y[i-1] + α * (x[i] - x[i-1])
 */ 

void    high_pass_filter_q16(uint8_t a, q16_t *hp, q16_t u, q16_t *u_mem)
{
    low_pass_filter_q16(a, u_mem, u);
    *hp = u - *u_mem;
}
