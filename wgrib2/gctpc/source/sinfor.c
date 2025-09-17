/** @file
 * @brief Sinusoidal - Forward Transformation
 *
 * PURPOSE: Transforms input longitude and latitude to Easting and Northing for
 * the Sinusoidal projection. The longitude and latitude must be in radians.
 * The Easting and Northing values will be returned in meters.
 *
 * This function was adapted from the Sinusoidal projection code (FORTRAN) in the
 * General Cartographic Transformation Package software which is available from
 * the U.S. Geological Survey National Mapping Division.
 * @author D. Steinwand, EROS @date May, 1991
 *
 * ### Algorithm References
 * 1. Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
 *    Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
 *    State Government Printing Office, Washington D.C., 1987.
 *
 * 2. "Software Documentation for GCTP General Cartographic Transformation
 *    Package", U.S. Geological Survey National Mapping Division, May 1982.
 */

#include "cproj.h"

/* Variables common to all subroutines in this code file */
static double lon_center;      /**< Center longitude (projection center). */
static double R;               /**< Radius of the earth (sphere). */
static double false_easting;   /**< X offset in meters. */
static double false_northing;  /**< Y offset in meters. */

/**
 * Initialize the Sinusoidal projection for forward transformation.
 *
 * @param r Radius of the earth (sphere).
 * @param center_long Center longitude.
 * @param false_east X offset in meters.
 * @param false_north Y offset in meters.
 *
 * @return Always returns 0
 *
 * @author D. Steinwand @date May, 1991
 */
long sinforint(double r, double center_long, double false_east,
        double false_north) {
//long sinforint(r, center_long,false_east,false_north) 
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
ptitle("SINUSOIDAL"); 
radius(r);
cenlon(center_long);
offsetp(false_easting,false_northing);
return(OK);
}

/**
 *  Sinusoidal forward equations--mapping lat,long to x,y
 *
 * @param lon Longitude in radians.
 * @param lat Latitude in radians.
 * @param x Pointer to store X projection coordinate.
 * @param y Pointer to store Y projection coordinate.
 *
 * @return Always returns 0
 *
 * @author D. Steinwand @date May, 1991
 */
long sinfor(double lon, double lat, double *x, double *y) {
//long sinfor(lon, lat, x, y)
//double lon;			/* (I) Longitude */
//double lat;			/* (I) Latitude */
//double *x;			/* (O) X projection coordinate */
//double *y;			/* (O) Y projection coordinate */
//{
double delta_lon;	/* Delta longitude (Given longitude - center */

/* Forward equations
  -----------------*/
delta_lon = adjust_lon(lon - lon_center);
*x = R * delta_lon * cos(lat) + false_easting;
*y = R * lat + false_northing;
return(OK);
}
