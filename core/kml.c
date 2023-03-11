#include <stdio.h>
#include <assert.h>
#include "kml.h"

void Kml_writeHeader(FILE *f)
{
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<kml xmlns=\"http://earth.google.com/kml/2.0\">\n");
    fprintf(f, "<Document>\n");
    fprintf(f, " <Style id=\"track\">\n");
    fprintf(f, "  <LineStyle>\n");
    fprintf(f, "   <color>ffff00ff</color>\n");
    fprintf(f, "  </LineStyle>\n");
    fprintf(f, "  <PolyStyle>\n");
    fprintf(f, "   <color>3fff00ff</color>\n");
    fprintf(f, "  </PolyStyle>\n");
    fprintf(f, " </Style>\n");
    fprintf(f, " <Placemark>\n");
    fprintf(f, "  <styleUrl>#track</styleUrl>\n");
    fprintf(f, "  <LineString>\n");
    fprintf(f, "   <extrude>1</extrude>\n");
    fprintf(f, "   <altitudeMode>absolute</altitudeMode>\n");
    fprintf(f, "   <coordinates>\n");
}

void Kml_writeEntry(FILE *f, const decoded_position_t *position)
{
    assert(f);
    assert(position);

    // TODO: Oups, little decode bug..
    if (position->dLatitude >= -90.f && position->dLatitude <= 90.f)
        fprintf(f, "   %f,%f,%.0f.\n", position->dLongitude, position->dLatitude, position->dAltitude);
}

void Kml_writeTail(FILE *f)
{
    fprintf(f, "   </coordinates>\n");
    fprintf(f, "  </LineString>\n");
    fprintf(f, " </Placemark>\n");
    fprintf(f, "</Document>\n");
    fprintf(f, "</kml>\n");
}
