/** @file
 * @brief Lambert Azimuthal Equal-Area - Inverse Transformation
 * 
 * PURPOSE: Transforms input Easting and Northing to longitude and
 *          latitude for the Lambert Azimuthal Equal-Area projection.
 *          The Easting and Northing must be in meters.  The longitude
 *          and latitude values will be returned in radians.
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
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 3/1991 | D. Steinwand, EROS | Initial implementation
 * 12/1993 | S. Nelson, EROS | Changed asin() to asinz() because NaN resulted 
 *                             expecting poles.
 */

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
 * Initialize the Lambert Azimuthal Equal Area projection for inverse
 * transformation.
 * 
 * @param r Radius of the earth (sphere)
 * @param center_long Center longitude 
 * @param center_lat Center latitude 
 * @param false_east x offset in meters
 * @param false_north y offset in meters
 * 
 * @return Always returns 0
 * 
 * @author D. Steinwand, EROS @date March, 1991
 */
long lamazinvint(double r, double center_long, double center_lat,
        double false_east, double false_north) {
//long lamazinvint(r, center_long, center_lat,false_east,false_north) 
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
 * Lambert Azimuthal Equal Area inverse equations--mapping x,y to lat,long
 * 
 * @param x X projection coordinate
 * @param y Y projection coordinate
 * @param lon Pointer to store the longitude
 * @param lat Pointer to store the latitude
 * 
 * @return 
 * - 0 :: Success
 * - 115 :: Input data error
 * 
 * @author D. Steinwand, EROS @date March, 1991
 */
long lamazinv(double x, double y, double *lon, double *lat) {
//long lamazinv(x, y, lon, lat)
//double x;		/* (I) X projection coordinate */
//double y;		/* (I) Y projection coordinate */
//double *lon;		/* (O) Longitude */
//double *lat;		/* (O) Latitude */
//{
double Rh;
double z;		/* Great circle dist from proj center to given point */
double sin_z;		/* Sine of z */
double cos_z;		/* Cosine of z */
double temp;		/* Re-used temporary variable */


/* Inverse equations
  -----------------*/
x -= false_easting;
y -= false_northing;
Rh = sqrt(x * x + y * y);
temp = Rh / (2.0 * R);
if (temp > 1) 
   {
   p_error("Input data error", "lamaz-inverse");
   return(115);
   }
z = 2.0 * asinz(temp);
sincos(z, &sin_z, &cos_z);
*lon = lon_center;
if (fabs(Rh) > EPSLN)
   {
   *lat = asinz(sin_lat_o * cos_z + cos_lat_o * sin_z * y / Rh);
   temp = fabs(lat_center) - HALF_PI;
   if (fabs(temp) > EPSLN)
      {
      temp = cos_z - sin_lat_o * sin(*lat);
      if(temp!=0.0)*lon=adjust_lon(lon_center+atan2(x*sin_z*cos_lat_o,temp*Rh));
      }
   else if (lat_center < 0.0) *lon = adjust_lon(lon_center - atan2(-x, y));
   else *lon = adjust_lon(lon_center + atan2(x, -y));
   }
else *lat = lat_center;
return(OK);
}
