#ifndef __KML_H__
#define __KML_H__

#include <stdio.h>
#include "trappette_sdk.h"

/*
 *
 */

void Kml_writeHeader(FILE *f);
void Kml_writeEntry(FILE *f, const decoded_position_t *position);
void Kml_writeTail(FILE *f);


#endif /*__KML_H__*/
