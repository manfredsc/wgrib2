/** @file
 * @brief Equirectangular - Inverse Transformation
 * 
 * PURPOSE: Transforms input Easting and Northing to longitude and
 *          latitude for the Equirectangular projection. The
 *          Easting and Northing must be in meters. The longitude
 *          and latitude values will be returned in radians.
 * 
 * @author T. Mittan @date Mar, 1993
 *
 * ### Algorithm References
 * 1. Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
 *    Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
 *    State Government Printing Office, Washington D.C., 1987.
 *
 * 2. Snyder, John P. and Voxland, Philip M., "An Album of Map Projections",
 *    U.S. Geological Survey Professional Paper 1453 , United State Government
 *    Printing Office, Washington D.C., 1989.
 */

#include "cproj.h"

/* Variables common to all subroutines in this code file
  -----------------------------------------------------*/
static double r_major;		/**< major axis 				*/
static double lon_center;	/**< Center longitude (projection center) */
static double lat_origin;	/**< center latitude			*/
static double false_northing;	/**< y offset in meters			*/
static double false_easting;	/**< x offset in meters			*/

/** Initialize the Equirectangular projection for inverse transformation.
 * 
 * @param r_maj Major axis
 * @param center_lon Center longitude
 * @param lat1 Latitude of true scale
 * @param false_east X offset in meters
 * @param false_north Y offset in meters
 * 
 * @return Always returns 0
 * 
 * @author T. Mittan @date Mar, 1993
 */
long equiinvint(double r_maj, double center_lon, double lat1,
        double false_east, double false_north) {
//long equiinvint(r_maj,center_lon,lat1,false_east,false_north) 
//
//double r_maj;			/* major axis			*/
//double center_lon;		/* center longitude		*/
//double lat1;			/* latitude of true scale	*/
//double false_east;		/* x offset in meters		*/
//double false_north;		/* y offset in meters		*/
//{

/* Place parameters in static storage for common use
  -------------------------------------------------*/
r_major = r_maj;
lon_center = center_lon;
lat_origin = lat1;
false_northing = false_north;
false_easting = false_east;

/* Report parameters to the user
  -----------------------------*/
ptitle("EQUIRECTANGULAR"); 
radius(r_major);
cenlonmer(lon_center);
origin(lat_origin);
offsetp(false_easting,false_northing);
return(OK);
}


/** Equirectangular inverse equations--mapping x,y to lat/long
 *
 * @param x X projection coordinate
 * @param y Y projection coordinate
 * @param lon Pointer to Longitude
 * @param lat Pointer to Latitude
 * 
 * @return 
 * - 0 :: Success
 * - 174 :: Input data error
 * 
 * @author T. Mittan @date Mar, 1993
 */
long equiinv(double x, double y, double *lon, double *lat) {
//long equiinv(x, y, lon, lat)
//double x;			/* (O) X projection coordinate 	*/
//double y;			/* (O) Y projection coordinate 	*/
//double *lon;			/* (I) Longitude 		*/
//double *lat;			/* (I) Latitude 		*/
//{

/* Inverse equations
  -----------------*/
x -= false_easting;
y -= false_northing;
*lat = y / r_major;
if (fabs(*lat) > HALF_PI)
   {
   p_error("Input data error","equi-inv");
   return(174);
   }
*lon = adjust_lon(lon_center + x / (r_major * cos(lat_origin)));
return(OK);
}
