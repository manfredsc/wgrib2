
/** @file
 * @brief Oblated Equal-Area - Inverse Transformation
 *
 * PURPOSE: Transforms input Easting and Northing to longitude and
 *          latitude for the Oblated Equal Area projection. The Easting and
 *          Northing must be in meters. The longitude and latitude values will
 *          be returned in radians.
 * @author D. Steinwand, EROS @date May, 1991
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
 * 5/1991 | D. Steinwand | Initial implementation
 * 11/1993 | S. Nelson | Added "double adjust_lon()" function declaration statement
 */

#include "cproj.h"

static double lon_center;     /**< Center longitude (projection center) */
static double lat_o;          /**< Center latitude (projection center) */
static double theta;          /**< Rotation angle (radians) */
static double m;              /**< Shape parameter m */
static double n;              /**< Shape parameter n */
static double R;              /**< Radius of the earth (sphere) */
static double sin_lat_o;      /**< Sine of center latitude */
static double cos_lat_o;      /**< Cosine of center latitude */
static double false_easting;  /**< X offset in meters */
static double false_northing; /**< Y offset in meters */

/**
 * Initialize the Oblated Equal-Area projection for inverse transformation.
 *
 * @param r Radius of the earth (sphere)
 * @param center_long Center longitude 
 * @param center_lat Center latitude 
 * @param shape_m Shape parameter m
 * @param shape_n Shape parameter n
 * @param angle Rotation angle (radians)
 * @param false_east X offset in meters
 * @param false_north Y offset in meters
 * 
 * @return Always returns 0
 *
 * @author D. Steinwand, EROS @date May, 1991
 */
long obleqinvint(double r, double center_long, double center_lat,
        double shape_m, double shape_n, double angle, double false_east,
        double false_north) {
//long obleqinvint(r, center_long, center_lat, shape_m, shape_n, angle,false_east,
//	    false_north)
//double r;
//double center_long;
//double center_lat;
//double shape_m,shape_n;
//double angle;
//double false_east;
//double false_north;
//{
/* Place parameters in static storage for common use
  -------------------------------------------------*/
R = r;
lon_center = center_long;
lat_o = center_lat;
m = shape_m;
n = shape_n;
theta = angle;
false_easting = false_east;
false_northing = false_north;

/* Report parameters to the user (to device set up prior to this call)
  -------------------------------------------------------------------*/
ptitle("OBLATED EQUAL-AREA");
radius(R);
cenlon(lon_center);
cenlat(lat_o);
genrpt(m,"Parameter m:      ");
genrpt(n,"Parameter n:      ");
genrpt(theta,"Theta:      ");
offsetp(false_easting,false_northing);

/* Calculate the sine and cosine of the latitude of the center of the map
   and store in static storage for common use.
  -------------------------------------------*/
sincos(lat_o, &sin_lat_o, &cos_lat_o);
return(OK);
}

/**
 * Oblated Equal-Area inverse equations--mapping x,y to lat,long
 *
 * @param x X projection coordinate
 * @param y Y projection coordinate
 * @param lon Pointer to the longitude
 * @param lat Pointer to the latitude
 *
 * @return Always returns 0
 *
 * @author D. Steinwand, EROS @date May, 1991
 */
long obleqinv(double x, double y, double *lon, double *lat) {
//long obleqinv(x, y, lon, lat)
//
//double x;		/* (I) X projection coordinate */
//double y;		/* (I) Y projection coordinate */
//double *lon;		/* (O) Longitude */
//double *lat;		/* (O) Latitude */
//{
double z;
double sin_z;
double cos_z;
double Az;
double temp;			/* Re-used temporary variable */
double x_prime;
double y_prime;
double M;
double N;
double diff_angle;
double sin_diff_angle;
double cos_diff_angle;

/* Inverse equations
  -----------------*/
x -= false_easting;
y -= false_northing;
N = (n / 2.0) * asin(y / (n * R));
temp = x / (m * R) * cos(2.0 * N / n) / cos(N);
M = (m / 2.0) * asin(temp);
x_prime = 2.0 * sin(M);
y_prime = 2.0 * sin(N) * cos(2.0 * M / m) / cos(M);
temp = sqrt(x_prime * x_prime + y_prime * y_prime) / 2.0;
z = 2.0 * asin(temp);
Az = atan2(x_prime, y_prime);
diff_angle = Az - theta;
sincos(diff_angle, &sin_diff_angle, &cos_diff_angle);
sincos(z, &sin_z, &cos_z);
*lat = asin(sin_lat_o * cos_z + cos_lat_o * sin_z * cos_diff_angle);
*lon = adjust_lon(lon_center + atan2((sin_z * sin_diff_angle), (cos_lat_o *
		 cos_z - sin_lat_o * sin_z * cos_diff_angle)));
return(OK);
}

