#ifndef __COORDINATE_H__
#define __COORDINATE_H__

/*
 *
 */

#define EARTH_RADIUS_KM     6371.008f

typedef enum {

    COORD_UNIT_DEGREE,
    COORD_UNIT_RADIAN
}   coord_unit_t;

typedef struct  {

    coord_unit_t    unit;
    double          dLatitude;
    double          dLongitude;
    double          dAltitude;
}   wgs84_t;

/*
 *
 */

int     wgs84_setCoordinate(wgs84_t *dst, double dLat, double dLong, double dAlt);
//double wgs84_computeHeadingAndDistance(const wgs84_t *localPos, const wgs84_t *remotePos);
double  wgs84_computeHeadingAndDistance(const wgs84_t *localPos, const wgs84_t *remotePos, double *dHeading, double *dDistance);

double  degToRad(double deg);
double  radToDeg(double rad);

#endif /*__COORDINATE_H__*/
