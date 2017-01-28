#include <stdio.h>
#include <string.h>
#include <math.h>
#include "wgs84.h"

/*
 * Private
 */

double  degToRad(double deg)
{
    return (deg*M_PI)/180.f;
}

double  radToDeg(double rad)
{
    return (rad*180.f)/M_PI;
}

int wgs84_degreeToRadian(const wgs84_t *src, wgs84_t *dst)
{
    dst->dLatitude  = degToRad(src->dLatitude);
    dst->dLongitude = degToRad(src->dLongitude);
    dst->unit = COORD_UNIT_RADIAN;

    return 0;
}

double  mod(double y, double x)
{
    return y - x*floor(y/x);
}

double  reduceAzimut(double azimut)
{
  return azimut - 2.f*M_PI * floor(azimut / (2.f*M_PI));
}


/*
 * Public
 */

int wgs84_setCoordinate(wgs84_t *dst, double dLat, double dLong, double dAlt)
{
    dst->dLatitude  = dLat;
    dst->dLongitude = dLong;
    dst->dAltitude  = dAlt;
    dst->unit = COORD_UNIT_DEGREE;

    return 0;
}

double  wgs84_computeHeadingAndDistance(const wgs84_t *localPos, const wgs84_t *remotePos, double *dHeading, double *dDistance)
{
//    double  dDistance, dHeading;
    wgs84_t p1, p2;
    double lat1, lat2, lon1, lon2;
    double  d;

#ifdef TRACE_WGS84
    printf("X1 Lat:%f°, Lon:%f°\n", localPos->dLatitude,  localPos->dLongitude );
    printf("X2 Lat:%f°, Lon:%f°\n", remotePos->dLatitude, remotePos->dLongitude);
#endif /*TRACE_WGS84*/

    /*
     * Convert Degree to Radian
     */

    if (localPos->unit == COORD_UNIT_DEGREE)
        wgs84_degreeToRadian(localPos, &p1);
    else
        memcpy(&p1, localPos, sizeof(wgs84_t));

    if (remotePos->unit == COORD_UNIT_DEGREE)
        wgs84_degreeToRadian(remotePos, &p2);
    else
        memcpy(&p2, remotePos, sizeof(wgs84_t));

    lat1 = p1.dLatitude;
    lon1 = p1.dLongitude;
    lat2 = p2.dLatitude;
    lon2 = p2.dLongitude;

    /*
     * Compute distance
     * https://fr.wikipedia.org/wiki/Orthodromie#Distance_orthodromique
     */

    d = acos( sin(lat1)*sin(lat2) + cos(lat1)*cos(lat2)*cos(lon1-lon2) );
    if (dDistance) {

        *dDistance = EARTH_RADIUS_KM * d;
#ifdef TRACE_WGS84
        printf("dist: %f NM / %f km\n", *dDistance/1.852f, *dDistance);
#endif
    }

    /*
     * Compute course
     * http://williams.best.vwh.net/avform.htm
     */

    if (dHeading) {

/*
        lon2 = -lon2;
        *dHeading = radToDeg( mod(atan2(sin(lon1-lon2)*cos(lat2), cos(lat1)*sin(lat2)-sin(lat1)*cos(lat2)*cos(lon1-lon2)), 2*M_PI) );
*/
/*
        if (sin(lon2-lon1) < 0.f)
            *dHeading = radToDeg( acos((sin(lat2)-sin(lat1)*cos(d))/(sin(d)*cos(lat1))) );
        else
            *dHeading = radToDeg( 2*M_PI - acos((sin(lat2)-sin(lat1)*cos(d))/(sin(d)*cos(lat1))) );
*/
        // zyGrib
//        *dHeading = radToDeg( reduceAzimut( atan2(sin(lon2-lon1)*cos(lat2), cos(lat1)*sin(lat2)-sin(lat1)*cos(lat2)*cos(lon2-lon1)) ) );
        *dHeading = radToDeg( mod( atan2(sin(lon2-lon1)*cos(lat2), cos(lat1)*sin(lat2)-sin(lat1)*cos(lat2)*cos(lon2-lon1)), 2*M_PI ) );

#ifdef TRACE_WGS84
        printf("heading: %f°\n\n", *dHeading);
#endif
    }

    return *dDistance;
}
