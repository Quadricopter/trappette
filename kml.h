#ifndef __KML_H__
#define __KML_H__

#include <stdio.h>
#include "tsip.h"

/*
 *
 */

void Kml_writeHeader(FILE *f);
void Kml_writeEntry(FILE *f, const tsip_t *tsip);
void Kml_writeTail(FILE *f);


#endif /*__KML_H__*/
