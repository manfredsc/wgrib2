/** @file
 * @brief Gnomonic - Forward Transformation
 * 
 * PURPOSE: Transforms input longitude and latitude to Easting and Northing
 *          for the Gnomonic projection. The longitude and latitude must be
 *          in radians. The Easting and Northing values will be returned in
 *          meters.
 * @author T. Mittan @date Mar, 1993
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

#include "cproj.h"

/* Variables common to all subroutines in this code file */
static double lon_center;	/**< Center longitude (projection center) */
static double lat_center;	/**< Center latitude (projection center) 	*/
static double R;		/**< Radius of the earth (sphere)	 	*/
static double sin_p13;		/**< Sine of the center latitude 		*/
static double cos_p13;		/**< Cosine of the center latitude 	*/
static double false_easting;	/**< x offset in meters			*/
static double false_northing;	/**< y offset in meters			*/

/** Initialize the Gnomonic projection for forward transformation
 *
 * @param r Radius of the earth (sphere)
 * @param center_long Center longitude
 * @param center_lat Center latitude
 * @param false_east x offset in meters
 * @param false_north y offset in meters
 * 
 * @return Always returns 0
 *
 * @author T. Mittan @date Mar, 1993
 */
long gnomforint(double r, double center_long, double center_lat,
        double false_east, double false_north) {
//long gnomforint(r, center_long, center_lat,false_east,false_north) 
//
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
sincos(center_lat, &sin_p13, &cos_p13);

/* Report parameters to the user
  -----------------------------*/
ptitle("GNOMONIC"); 
radius(r);
cenlon(center_long);
cenlat(center_lat);
offsetp(false_easting,false_northing);
return(OK);
}


/** Gnomonic forward equations--mapping lat,long to x,y 
 * 
 * @param lon Longitude
 * @param lat Latitude
 * @param x Pointer to X projection coordinate
 * @param y Pointer to Y projection coordinate
 * 
 * @return 
 * - 0 :: Success
 * - 133 :: Point projects into infinity
 * 
 * @author T. Mittan @date Mar, 1993
 */
long gnomfor(double lon, double lat, double *x, double *y) {
//long gnomfor(lon, lat, x, y)
//double lon;			/* (I) Longitude */
//double lat;			/* (I) Latitude */
//double *x;			/* (O) X projection coordinate */
//double *y;			/* (O) Y projection coordinate */
//
//{
double dlon;
double sinphi,cosphi;
double coslon;
double g;
double ksp;


/* Forward equations
  -----------------*/
dlon = adjust_lon(lon - lon_center);
sincos(lat,&sinphi,&cosphi);
coslon = cos(dlon);
g = sin_p13 * sinphi + cos_p13 * cosphi * coslon;
if (g <= 0.0)
   {
   p_error("Point projects into infinity","gnomfor-conv");
   return(133);
   }
ksp = 1.0 / g;
*x = false_easting + R * ksp * cosphi * sin(dlon);
*y = false_northing + R * ksp * (cos_p13 * sinphi - sin_p13 * cosphi * 
		coslon);

return(OK);
}
