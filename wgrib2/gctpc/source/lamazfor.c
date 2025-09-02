/** @file
 * @brief Lambert Azimuthal Equal-Area - Forward Transformation
 * 
 * PURPOSE: Transforms input longitude and latitude to Easting and
 *          Northing for the Lambert Azimuthal Equal-Area projection.
 *          The longitude and latitude must be in radians.  The Easting
 *          and Northing values will be returned in meters.
 * 
 * This function was adapted from the Lambert Azimuthal Equal Area projection
 * (FORTRAN) in the General Cartographic Transformation Package software
 * which is available from the U.S. Geological Survey National Mapping Division.
 * @author D. Steinwand, EROS @date March, 1991
 * 
 * ### Algorithm References
 * 1. "New Equal-Area Map Projections for Noncircular Regions", John P. Snyder,
 *    The American Cartographer, Vol 15, No. 4, October 1988, pp. 341-355.
 * 
 * 2. Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
 *    Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
 *    State Government Printing Office, Washington D.C., 1987.
 * 
 * 3. "Software Documentation for GCTP General Cartographic Transformation
 *    Package", U.S. Geological Survey National Mapping Division, May 1982.
 */

#include <stdio.h>
#include "cproj.h"

/* Variables common to all subroutines in this code file */
static double lon_center;	/**< Center longitude (projection center) */
static double lat_center;	/**< Center latitude (projection center) */
static double R;		/**< Radius of the earth (sphere) */
static double sin_lat_o;	/**< Sine of the center latitude */
static double cos_lat_o;	/**< Cosine of the center latitude */
static double false_easting;	/**< x offset in meters */
static double false_northing;	/**< y offset in meters */

/** 
 * Initialize the Lambert Azimuthal Equal Area projection for forward 
 * transformation.
 * 
 * @param r Radius of the earth (sphere)
 * @param center_long Center longitude (projection center)
 * @param center_lat Center latitude (projection center)
 * @param false_east x offset in meters
 * @param false_north y offset in meters
 * 
 * @return Always returns 0
 * @author D. Steinwand, EROS @date March, 1991
 */
long lamazforint(double r, double center_long, double center_lat,
        double false_east, double false_north) {
//long lamazforint(r, center_long, center_lat,false_east,false_north) 
//double r; 			/* (I) Radius of the earth (sphere) 	*/
//double center_long;		/* (I) Center longitude 		*/
//double center_lat;		/* (I) Center latitude 			*/
//double false_east;		/* x offset in meters			*/
//double false_north;		/* y offset in meters			*/
//{
/* Place parameters in static storage for common use
  -------------------------------------------------*/
R = r;
lon_center = center_long;
lat_center = center_lat;
false_easting = false_east;
false_northing = false_north;
sincos(center_lat, &sin_lat_o, &cos_lat_o);

/* Report parameters to the user
  -----------------------------*/
ptitle("LAMBERT AZIMUTHAL EQUAL-AREA"); 
radius(r);
cenlon(center_long);
cenlat(center_lat);
offsetp(false_easting,false_northing);
return(OK);
}



/** 
 * Lambert Azimuthal Equal Area forward equations--mapping lat,long to x,y 
 * 
 * @param lon Longitude in radians
 * @param lat Latitude in radians
 * @param x Pointer to store the X projection coordinate
 * @param y Pointer to store the Y projection coordinate
 * 
 * @return 
 * - 0 :: Success
 * - 113 :: Failure
 * @author D. Steinwand, EROS @date March, 1991
 */
long lamazfor(double lon, double lat, double *x, double *y) {
//long lamazfor(lon, lat, x, y)
//double lon;			/* (I) Longitude */
//double lat;			/* (I) Latitude */
//double *x;			/* (O) X projection coordinate */
//double *y;			/* (O) Y projection coordinate */
//{
double delta_lon;	/* Delta longitude (Given longitude - center 	*/
double sin_delta_lon;	/* Sine of the delta longitude 			*/
double cos_delta_lon;	/* Cosine of the delta longitude 		*/
double sin_lat;		/* Sine of the given latitude 			*/
double cos_lat;		/* Cosine of the given latitude 		*/
double g;		/* temporary varialbe				*/
double ksp;		/* heigth above elipsiod			*/
char mess[60];

/* Forward equations
  -----------------*/
delta_lon = adjust_lon(lon - lon_center);
sincos(lat, &sin_lat, &cos_lat);
sincos(delta_lon, &sin_delta_lon, &cos_delta_lon);
g = sin_lat_o * sin_lat + cos_lat_o * cos_lat * cos_delta_lon;
if (g == -1.0) 
   {
   sprintf(mess, "Point projects to a circle of radius = %lf\n", 2.0 * R);
   p_error(mess, "lamaz-forward");
   return(113);
   }
ksp = R * sqrt(2.0 / (1.0 + g));
*x = ksp * cos_lat * sin_delta_lon + false_easting;
*y = ksp * (cos_lat_o * sin_lat - sin_lat_o * cos_lat * cos_delta_lon) + 
	false_northing;
return(OK);
}
