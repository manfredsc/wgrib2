/** @file
 * @brief Hammer -  Forward Transformation
 * 
 * PURPOSE: Transforms input longitude and latitude to Easting and
 *          Northing for the Hammer projection.  The longitude
 *          and latitude must be in radians.  The Easting and
 *          Northing values will be returned in meters.
 * 
 * This function was adapted from the Hammer projection code (FORTRAN)
 * in the General Cartographic Transformation Package software which is
 * available from the U.S. Geological Survey National Mapping Division.
 * 
 * @author T. Mittan @date March, 1993
 * 
 * ### Algorithm References
 * 1.  "New Equal-Area Map Projections for Noncircular Regions", John P. Snyder,
 *     The American Cartographer, Vol 15, No. 4, October 1988, pp. 341-355.
 *
 * 2.  Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
 *     Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
 *     State Government Printing Office, Washington D.C., 1987.
 *
 * 3.  "Software Documentation for GCTP General Cartographic Transformation
 *     Package", U.S. Geological Survey National Mapping Division, May 1982.
 */

#include "cproj.h"

/* Variables common to all subroutines in this code file */
static double lon_center;	/**< Center longitude (projection center) */
static double R;		/**< Radius of the earth (sphere)	 	*/
static double false_easting;	/**< x offset in meters			*/
static double false_northing;	/**< y offset in meters			*/

/**
 * Initialize the HAMMER projection for forward transformation.
 *
 * @param r Radius of the earth (sphere)
 * @param center_long Center longitude
 * @param false_east x offset in meters
 * @param false_north y offset in meters
 * 
 * @return Always returns 0
 *
 * @author T. Mittan @date March, 1993
 */
long hamforint(double r, double center_long, double false_east,
        double false_north) {
//long hamforint(r, center_long,false_east,false_north) 
//
//double r; 			/* (I) Radius of the earth (sphere) 	*/
//double center_long;		/* (I) Center longitude 		*/
//double false_east;		/* x offset in meters			*/
//double false_north;		/* y offset in meters			*/
//{

/* Place parameters in static storage for common use
  -------------------------------------------------*/
R = r;
lon_center = center_long;
false_easting = false_east;
false_northing = false_north;

/* Report parameters to the user
  -----------------------------*/
ptitle("HAMMER"); 
radius(r);
cenlon(center_long);
offsetp(false_easting,false_northing);
return(OK);
}


/**
 * HAMMER forward equations--mapping lat,long to x,y
 * 
 * @param lon Longitude
 * @param lat Latitude
 * @param x Pointer to X projection coordinate
 * @param y Pointer to Y projection coordinate
 * 
 * @return Always returns 0
 * 
 * @author T. Mittan @date March, 1993
 */
long hamfor(double lon, double lat, double *x, double *y) {
//long hamfor(lon, lat, x, y)
//double lon;			/* (I) Longitude */
//double lat;			/* (I) Latitude */
//double *x;			/* (O) X projection coordinate */
//double *y;			/* (O) Y projection coordinate */
//
//{
double dlon;
double fac;

/* Forward equations
  -----------------*/
dlon = adjust_lon(lon - lon_center);

fac  = R * 1.414213562 / sqrt(1.0 + cos(lat) * cos(dlon / 2.0));
*x = false_easting + fac * 2.0 * cos(lat) * sin(dlon / 2.0);
*y = false_northing + fac * sin(lat);

return(OK);
}
