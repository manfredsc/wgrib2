/** @file
 * @brief Mollweide - Inverse Transformation
 *
 * PURPOSE: Transforms input Easting and Northing to longitude and
 *          latitude for the Mollweide projection. The Easting and
 *          Northing must be in meters. The longitude and latitude
 *          values will be returned in radians.
 * @author D. Steinwand, EROS @date May, 1991
 * 
 * ### Algorithm References
 * 1. Snyder, John P. and Voxland, Philip M., "An Album of Map Projections",
 *    U.S. Geological Survey Professional Paper 1453 , United State Government
 *    Printing Office, Washington D.C., 1989.
 *
 * 2. Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
 *    Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
 *    State Government Printing Office, Washington D.C., 1987.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 5/1991 | D. Steinwand | Initial implementation
 * 9/1992 | D. Steinwand | Update
 * 2/1993 | D. Steinwand | Update
 * 11/1993 | S. Nelson | Fixed infinite loop at poles
 */

#include "cproj.h"

/* Variables common to all subroutines in this code file */
static double lon_center;	/**< Center longitude (projection center) */
static double R;		/**< Radius of the earth (sphere) */
static double false_easting;	/**< x offset in meters */
static double false_northing;	/**< y offset in meters */

/** 
 * Initialize the Mollweide projection for inverse transformation.
 *
 * @param r Radius of the earth (sphere)
 * @param center_long Center longitude (projection center)
 * @param false_east x offset in meters
 * @param false_north y offset in meters
 *
 * @return Always returns 0
 * 
 * @author D. Steinwand, EROS @date May, 1991
 */
long molwinvint(double r, double center_long, double false_east,
        double false_north) {
//long molwinvint(r, center_long, false_east, false_north) 
//double r; 			/* (I) Radius of the earth (sphere) */
//double center_long;		/* (I) Center longitude */
//double false_east;		/* x offset in meters			*/
//double false_north;		/* y offset in meters			*/
//{
/* Place parameters in static storage for common use
  -------------------------------------------------*/
false_easting = false_east;
false_northing = false_north;
R = r;
lon_center = center_long;

/* Report parameters to the user
  -----------------------------*/
ptitle("MOLLWEIDE"); 
radius(r);
cenlon(center_long);
offsetp(false_easting,false_northing);
return(OK);
}



/** 
 * Mollweide inverse equations--mapping x,y to lat,long
 *
 * @param x X projection coordinate
 * @param y Y projection coordinate
 * @param lon Pointer to store longitude
 * @param lat Pointer to store latitude
 * 
 * @return Always returns 0
 * 
 * @author D. Steinwand, EROS @date May, 1991 
 */
long molwinv(double x, double y, double *lon, double *lat) {
//long molwinv(x, y, lon, lat)
//double x;		/* (I) X projection coordinate */
//double y;		/* (I) Y projection coordinate */
//double *lon;		/* (O) Longitude */
//double *lat;		/* (O) Latitude */
//{
double theta;
double arg;

/* Inverse equations
  -----------------*/
x -= false_easting;
y -= false_northing;
arg = y /  (1.4142135623731 * R);

/* Because of division by zero problems, 'arg' can not be 1.0.  Therefore
   a number very close to one is used instead.
   -------------------------------------------------------------------*/
if(fabs(arg) > 0.999999999999) arg=0.999999999999;
theta = asin(arg);
*lon = adjust_lon(lon_center + (x / (0.900316316158 * R * cos(theta))));
if(*lon < (-PI)) *lon= -PI;
if(*lon > PI) *lon= PI;
arg = (2.0 * theta + sin(2.0 * theta)) / PI;
if(fabs(arg) > 1.0)arg=1.0;
*lat = asin(arg);
return(OK);
}
