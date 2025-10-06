/** @file
 * @brief Interface routines to the Proj.4 library
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 6/2012 | D. Jovic | Initial implementation
 * 8/2014 | W. Ebisuzaki | latlon, lambert conformal, ncep rotated latlon B grid
 * 10/2015 | W. Ebisuzaki | lambert azimuthal equal area
 * @author Public Domain: Dusan Jovic, Wesley Ebisuzaki @date 6/2012
 */
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

#ifdef USE_PROJ4

#include <proj_api.h>
#include "proj4_wgrib2.h"

/** Flag to indicate lat-lon grid processing. */
extern int latlon;

/** Current output order type. */
extern enum output_order_type output_order;

/** Flag to use Proj4 library for transformations. */
extern int use_proj4;


#ifndef M_PI
#define M_PI           3.14159265358979323846  /**< pi */
#endif
#ifndef M_PI_2
#define M_PI_2         1.57079632679489661923  /**< pi/2 */
#endif
#ifndef M_PI_4
#define M_PI_4         0.78539816339744830962  /**< pi/4 */
#endif

/** Pointer to array of latitude values. */
extern double *lat;

/** Pointer to array of longitude values. */
extern double *lon;

/** Grid spacing in x direction. */
static double dx;

/** Grid spacing in y direction. */
static double dy;

/** Origin x coordinate. */
static double x_0;

/** Origin y coordinate. */
static double y_0;

/** x coordinate of the first grid point. */
static double x00;

/** x coordinate of the last grid point. */
static double xN;

/** Grid Definition Template (GDT) number. */
static unsigned int gdt;

/** Number of grid points in x direction. */
static int nx;

/** Number of grid points in y direction. */
static int ny;  

/** Number of grid points in x direction (unsigned). */
static unsigned int nx_;

/** Number of grid points in y direction (unsigned). */
static unsigned int ny_;

/** Proj4 grid parameters. */
static projPJ pj_grid;

/** Proj4 latlon parameters. */
static projPJ pj_latlon;

/**
 * Initialize the Proj4 library for grid transformations.
 *
 * @param sec Pointer to the GRIB2 sections.
 * @param grid_lon Pointer to the grid longitude values.
 * @param grid_lat Pointer to the grid latitude values.
 * 
 * @return 0 on success, error code otherwise
 *
 * @author Dusan Jovic @date 6/2012
 */
int proj4_init(unsigned char **sec, double *grid_lon, double *grid_lat) {

    unsigned char *gds;

    double r_maj;                           /* major axis                   */
    double r_min;                           /* minor axis                   */
    double latsp1;                          /* first standard parallel      */
    double latsp2;                          /* second standard parallel     */
    double c_lon;                           /* center longitude             */
    double c_lat;                           /* center latitude              */
    double lon1;
//    double lon2;
    double lat1;
//    double lat2;

    int nres, nscan,has_np, center;
    unsigned int npnts;
    char proj4_def[1000];

    if (grid_lat == NULL || grid_lon == NULL) return 1;
    gdt = code_table_3_1(sec);
    gds = sec[3];
    center = GB2_Center(sec);
//fprintf(stderr,"proj4_init started gdt=%d\n", gdt);

    get_nxny(sec, &nx, &ny, &npnts, &nres, &nscan);
    get_nxny_(sec, &nx_, &ny_, &npnts, &nres, &nscan);
    if (nx_ < 1 || ny_ < 1 || nx_*ny_ != npnts)   return 1;

    /* only process certain grids */
    pj_grid = NULL;
    pj_latlon = NULL;

    x_0 = y_0 = x00 = xN = 0.0;

    if (gdt == 10 && (GDS_Mercator_ori_angle(gds) == 0.0) ) {            // mercator no rotation
        /* get earth axis */
        axes_earth(sec, &r_maj, &r_min, NULL);
        dx = fabs(GDS_Mercator_dx(gds));
        dy = fabs(GDS_Mercator_dy(gds));

        /* central point */
        c_lon = 0.0;
        c_lat = GDS_Mercator_latD(gds);

        sprintf(proj4_def,"+proj=merc +lat_ts=%lf +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +a=%lf +b=%lf", 
	    c_lat, r_maj, r_min);

        if ((pj_grid = pj_init_plus(proj4_def)) == NULL) fatal_error("Proj4 GDT=10 a: pj_init_plus %s failed", proj4_def);

        sprintf(proj4_def,"+proj=latlong +a=%lf +b=%lf",r_maj, r_min);
        if ( (pj_latlon = pj_init_plus(proj4_def)) == NULL) fatal_error("Proj4 GDT=10 b: pj_init_plus %s failed", proj4_def);

        /* longitude, latitude of first grid point */
        lat1 = GDS_Mercator_lat1(gds);
        lon1 = GDS_Mercator_lon1(gds);

        x_0 = lon1 * DEG_TO_RAD;
        y_0 = lat1 * DEG_TO_RAD;

        if ( pj_transform(pj_latlon, pj_grid, 1, 1, &x_0, &y_0, NULL) != 0 ) 
                fatal_error("Proj4 GDT=10 c","");
    }
    else if (gdt == 20) {            // polar stereographic

        /* get earth axis */
        axes_earth(sec, &r_maj, &r_min, NULL);
        dy      = fabs(GDS_Polar_dy(gds));
        dx      = fabs(GDS_Polar_dx(gds));

        /* central point */
        c_lon = GDS_Polar_lov(gds);
        c_lat = GDS_Polar_lad(gds);

        /* strange but np/sp flag is used by proj4 but not gctpc */
        has_np = ((flag_table_3_5(sec) & 128) == 0);

        sprintf(proj4_def,"+proj=stere +lat_ts=%lf +lat_0=%s +lon_0=%lf +k_0=1 +x_0=0 +y_0=0 +a=%lf +b=%lf", 
                c_lat, has_np ? "90" : "-90", c_lon, r_maj,r_min);

        if ((pj_grid = pj_init_plus(proj4_def)) == NULL) fatal_error("Proj4 GDT=20 a: pj_init_plus %s failed", proj4_def);

        sprintf(proj4_def,"+proj=latlong +a=%lf +b=%lf",r_maj, r_min);
        if ( (pj_latlon = pj_init_plus(proj4_def)) == NULL) fatal_error("Proj4 GDT=20 b: pj_init_plus %s failed", proj4_def);

        /* longitude, latitude of first grid point */
        lon1 = GDS_Polar_lon1(gds);
        lat1 = GDS_Polar_lat1(gds);

        x_0 = lon1 * DEG_TO_RAD;
        y_0 = lat1 * DEG_TO_RAD;

        if ( pj_transform(pj_latlon, pj_grid, 1, 1, &x_0, &y_0, NULL) != 0 ) 
                fatal_error("Proj4 GDT=20 c","");
    }
    else if (gdt == 30) {            // lambert conformal conic

        /* get earth axis */
        axes_earth(sec, &r_maj, &r_min, NULL);
        dx      = fabs(GDS_Lambert_dx(gds));
        dy      = fabs(GDS_Lambert_dy(gds));

        /* latitudes of tangent/intersection */
        latsp1 = GDS_Lambert_Latin1(gds);
        latsp2 = GDS_Lambert_Latin2(gds);

        /* central point */
        c_lon = GDS_Lambert_Lov(gds);
        c_lat = GDS_Lambert_LatD(gds);

        sprintf(proj4_def,"+proj=lcc +lon_0=%lf +lat_0=%lf +lat_1=%lf +lat_2=%lf +a=%lf +b=%lf",c_lon,
                   c_lat,latsp1,latsp2,r_maj,r_min);

        if ((pj_grid = pj_init_plus(proj4_def)) == NULL) fatal_error("Proj4 GDT=30 a: pj_init_plus %s failed", proj4_def);

 
        sprintf(proj4_def,"+proj=latlong +a=%lf +b=%lf",r_maj, r_min);
        if ((pj_latlon = pj_init_plus(proj4_def)) == NULL) fatal_error("Proj4 GDT=30 b: pj_init_plus %s failed", proj4_def);

        /* longitude, latitude of first grid point */
        lon1 = GDS_Lambert_Lo1(gds);
        lat1 = GDS_Lambert_La1(gds);

        x_0 = lon1 * DEG_TO_RAD;
        y_0 = lat1 * DEG_TO_RAD;
        if ( pj_transform(pj_latlon, pj_grid, 1, 1, &x_0, &y_0, NULL) != 0 ) fatal_error("Proj4 GDT=30 c","");
    }
    else if (gdt == 140) {            // lambert azimuthal equal area
        /* get earth axis */
        axes_earth(sec, &r_maj, &r_min, NULL);
        dx      = fabs(GDS_Lambert_Az_dx(gds));
        dy      = fabs(GDS_Lambert_Az_dy(gds));

        /* central point */
        c_lon = GDS_Lambert_Az_Cen_Lon(gds);
        c_lat = GDS_Lambert_Az_Std_Par(gds);

        sprintf(proj4_def,"+proj=laea +lon_0=%lf +lat_0=%lf +a=%lf +b=%lf",c_lon,c_lat,r_maj,r_min);
        if ((pj_grid = pj_init_plus(proj4_def)) == NULL) fatal_error("Proj4 GDT=140 a: pj_init_plus %s failed", proj4_def);

        sprintf(proj4_def,"+proj=latlong +a=%lf +b=%lf",r_maj, r_min);
        if ((pj_latlon = pj_init_plus(proj4_def)) == NULL) fatal_error("Proj4 GDT=140 b: pj_init_plus %s failed", proj4_def);

        /* longitude, latitude of first grid point */
        lon1 = GDS_Lambert_Az_Lo1(gds);
        lat1 = GDS_Lambert_Az_La1(gds);

        x_0 = lon1 * DEG_TO_RAD;
        y_0 = lat1 * DEG_TO_RAD;
        if ( pj_transform(pj_latlon, pj_grid, 1, 1, &x_0, &y_0, NULL) != 0 ) fatal_error("Proj4 GDT=140 c","");
    }
    else if (center == NCEP && gdt == 32769) {         // ncep rotated latlon Non-E 

        /* get earth axis */ 
        axes_earth(sec, &r_maj, &r_min, NULL);

        /* dx, dy */
        dx = fabs(GDS_NCEP_B_LatLon_dlon(gds) * 0.000001);
        dy = fabs(GDS_NCEP_B_LatLon_dlat(gds) * 0.000001);
        dx *= DEG_TO_RAD;
        dy *= DEG_TO_RAD;

        /* central point */
        c_lon = GDS_NCEP_B_LatLon_tlm0d(gds) * 0.000001;
        c_lat = GDS_NCEP_B_LatLon_tph0d(gds) * 0.000001;

        lon1 = GDS_NCEP_B_LatLon_lon1(gds) * 0.000001;
        lat1 = GDS_NCEP_B_LatLon_lat1(gds) * 0.000001;

        sprintf(proj4_def,"+proj=ob_tran +o_proj=latlon +o_lon_p=%f +o_lat_p=%f",c_lon,90.0+c_lat);
        if ((pj_latlon = pj_init_plus(proj4_def)) == NULL) fatal_error("Proj4 GDT=32769 a: pj_init_plus %s failed", proj4_def);

        sprintf(proj4_def,"+proj=latlon");
        if ((pj_grid = pj_init_plus(proj4_def)) == NULL ) fatal_error("Proj4: GDT=32769 c:pj_init_plus %s failed", proj4_def);

        x_0 = lon1 * DEG_TO_RAD;
        y_0 = lat1 * DEG_TO_RAD;
        if ( pj_transform(pj_latlon, pj_grid, 1, 1, &x_0, &y_0, NULL) != 0 ) fatal_error("Proj4 GDT=32689 c","");
 
    }
    else {
       return 1;
    }
    return 0;
}

/**
 * Convert (lon/lat) to (x/y) coordinates using Proj4.
 *
 * @param n Number of points.
 * @param lon Pointer to array of longitudes.
 * @param lat Pointer to array of latitudes.
 * @param x Pointer to array to store x coordinates.
 * @param y Pointer to array to store y coordinates.
 *
 * @return 0 on success, error code otherwise
 * 
 * @author Dusan Jovic @date 6/2012
 */
int Proj4_ll2xy(int n, double *lon, double *lat, double *x, double *y) {

    int i;
    double rlon, rlat, inv_dx, inv_dy;

    inv_dx = 1.0 / dx;
    inv_dy = 1.0 / dy;

#ifdef USE_OPENMP
#pragma omp parallel for schedule(static) private(i,rlon,rlat)
#endif
    for (i = 0; i < n; i++) {
        rlon = lon[i] * DEG_TO_RAD;
        rlat = lat[i] * DEG_TO_RAD;

        if ( pj_transform(pj_latlon, pj_grid, 1, 1, &rlon, &rlat, NULL) != 0 ) {
            x[i] = y[i] = UNDEFINED;
        }
        else {
            x[i] = (rlon - x_0)*inv_dx;
            y[i] = (rlat - y_0)*inv_dy;
        }
    }
    return 0;
}

/**
 * Convert longitude/latitude to (i).
 *
 * @param n Number of points.
 * @param lon Pointer to array of longitudes.
 * @param lat Pointer to array of latitudes.
 * @param ipnt Pointer to array to store grid point indices.
 *
 * @return 0 on success, error code otherwise
 *
 * @author Dusan Jovic @date 6/2012
 */
int Proj4_ll2i(int n, double *lon, double *lat, unsigned int *ipnt) {
    int error;
    unsigned int i;
    double rlon, rlat, inv_dx, inv_dy, x, y; 

    inv_dx = 1.0 / dx;
    inv_dy = 1.0 / dy;

    error = 0;

    for (i = 0; i < n; i++) {
        rlon = lon[i] * DEG_TO_RAD;
        rlat = lat[i] * DEG_TO_RAD;

        if ( pj_transform(pj_latlon, pj_grid, 1, 1, &rlon, &rlat, NULL) != 0 ) error = 1;

        x = floor((rlon - x_0)*inv_dx + 0.5);
        y = floor((rlat - y_0)*inv_dy + 0.5);

        if (x < 0 || x >= nx || y < 0 || y >= ny) {
            ipnt[i] = 0;
        }
        else {
            ipnt[i] = (unsigned int) x + nx*(unsigned int) y + 1;
        }
    }
    return error;
}

/*
 * HEADER:100:proj4_ll2ij:inv:2:x=lon y=lat, converts lon-lat (i,j) using proj.4 (experimental)
 */

/**
 * Convert (lon/lat) to (i,j) coordinates using Proj4. Experimental.
 * 
 * ## Usage
 * -proj4_ll2ij lon lat
 *
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 *
 * @return 0 on success, error code otherwise
 *
 * @author Dusan Jovic @date 6/2012
 */
int f_proj4_ll2ij(ARG2) {

    double x[1], y[1], to_lat[1], to_lon[1];
    int i;

    if (mode == -1) {
        latlon = 1;
    }
    if (mode >= 0) {
        if (output_order != wesn)  return 1;
        to_lon[0] = atof(arg1);
        to_lat[0] = atof(arg2);
        i = proj4_init(sec, lon, lat);
        if (i == 0)  {
            i = Proj4_ll2xy(1, to_lon, to_lat, x , y);
            if (i) x[0] = y[0] = -1.0;
            sprintf(inv_out,"%lf %lf -> (%lf,%lf)",to_lon[0], to_lat[0], x[0]+1.0, y[0]+1.0);
        }
    }
    return 0;
}

/**
 * Convert (i,j) to (lon/lat) coordinates using Proj4.
 *
 * @param sec Pointer to the section.
 * @param n Number of points.
 * @param x Pointer to array of x coordinates.
 * @param y Pointer to array of y coordinates.
 * @param lon Pointer to array to store longitudes.
 * @param lat Pointer to array to store latitudes.
 *
 * @return 0 on success, error code otherwise
 *
 * @author Dusan Jovic @date 6/2012
 */
int Proj4_ij2ll(unsigned char **sec, int n, double *x, double *y, double *lon, double *lat) {

    int i, error;
    double xx, yy;

    error = 0;
    if (gdt == 0) {
#ifdef USE_OPENMP
#pragma omp parallel for schedule(static)
#endif
        for (i = 0; i < n; i++) {
            lon[i] = dx * x[i] + x_0;
            lat[i] = dy * y[i] + y_0;
            if (lon[i] < 0.0) lon[i] += 360.0;
        }
        return error;
    }
    

#ifdef USE_OPENMP
#pragma omp parallel for schedule(static) private(i,xx,yy)
#endif
    for (i = 0; i < n; i++) {
        xx = x[i] + x_0;
        yy = y[i] + y_0;
/* test */
        xx = dx*(x[i] -1.0) + x_0;
        yy = dy*(y[i] -1.0) + y_0;
        if ( pj_transform(pj_grid, pj_latlon, 1, 1, &xx, &yy, NULL) != 0 ) {
            error = 1;
            lon[i] = 999.0;
            lat[i] = 999.0;
        }
        else {
            lon[i] = xx * RAD_TO_DEG;
            lat[i] = yy * RAD_TO_DEG;
            if (lon[i] < 0.0) lon[i] += 360.0;
        }
    }
    return error;

}

/**
 * Convert (x,y) to (lon/lat) coordinates using Proj4.
 *
 * @param n Number of points.
 * @param x Pointer to array of x coordinates.
 * @param y Pointer to array of y coordinates.
 * @param lon Pointer to array to store longitudes.
 * @param lat Pointer to array to store latitudes.
 *
 * @return 0 on success, error code otherwise
 *
 * @author Dusan Jovic @date 6/2012
 */
int Proj4_xy2ll(int n, double *x, double *y, double *lon, double *lat) {

    int i, error;
    double xx, yy;

    if (gdt == 0) {
#ifdef USE_OPENMP
#pragma omp parallel for schedule(static)
#endif
        for (i = 0; i < n; i++) {
            lon[i] = dx * x[i] + x_0;
            lat[i] = dy * y[i] + y_0;
            if (lon[i] < 0.0) lon[i] += 360.0;
        }
        return 0;
    }
    error = 0;
    for (i = 0; i < n; i++) {
        xx = x[i] + x_0;
        yy = y[i] + y_0;
        if ( pj_transform(pj_grid, pj_latlon, 1, 1, &xx, &yy, NULL) != 0 ) error = 1;
        lon[i] = xx * RAD_TO_DEG;
        lat[i] = yy * RAD_TO_DEG;
        if (lon[i] < 0.0) lon[i] += 360.0;
    }
    return error;
}

/*
 * HEADER:100:proj4_ij2ll:inv:2:X=x Y=y, converts to (i,j) to lon-lat using proj.4  (experimental) we:sn
 */

/**
 * Convert (i,j) to (lon/lat) coordinates using Proj4. Experimental.
 *
 * The lat/lon variables are given in we:sn order.
 *
 * ## Usage
 * -proj4_ij2ll i j
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 *
 * @return 0 on success, error code otherwise
 *
 * @author Dusan Jovic @date 6/2012
 */
int f_proj4_ij2ll(ARG2) {
    int i;
    double x, y, rlon, rlat;

    if (mode == -1) {
        latlon = 1;
    }
    else if (mode >= 0) {
        x = atof(arg1);
        y = atof(arg2);
        i = proj4_init(sec, lon, lat);
        if (i == 0)  {
            i = Proj4_ij2ll(sec, 1, &x, &y, &rlon, &rlat);
            if (i == 0) {
                sprintf(inv_out,"x=%lf y=%lf lon=%lf lat=%lf", x, y, rlon, rlat);
            }
        }
    }
    return 0;
}

/*
 * HEADER:100:proj4_ll2i:inv:2:x=lon y=lat, converts to (i) using proj.4  (experimental) 1..ndata
 */

/**
 * Convert (lon,lat) to (i) coordinates using Proj4. Experimental.
 * 
 * The grid indices i are 1...ndata.
 * 
 * ## Usage
 * -proj4_ll2i lon lat
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 *
 * @return 0 on success, error code otherwise
 *
 * @author Dusan Jovic @date 6/2012
 */
int f_proj4_ll2i(ARG2) {

    double to_lat[1], to_lon[1];
    int i;
    unsigned int iptr;

    if (mode == -1) {
        latlon = 1;
    }
    if (mode >= 0) {
        if (output_order != wesn) return 1;
        to_lon[0] = atof(arg1);
        to_lat[0] = atof(arg2);
        i = proj4_init(sec, lon, lat);
        if (i) iptr = 0;
        else {
            i = Proj4_ll2i(1, to_lon, to_lat, &iptr);
            if (i) iptr = 0;
        }
        sprintf(inv_out,"%lf %lf -> (%u)",to_lon[0], to_lat[0], iptr);
    }
    return 0;
}

/*
 * HEADER:100:proj4:misc:1:X=0,1 use proj4 library for geolocation (testing)
 */

/**
 * Set flag to use Proj4 library.
 *
 * Wgrib2 has 3 sets of geolocation routines. Geolocation routines are used to find the 
 * locations of the grid points by a (i,j) -> (lon,lat) transformation. Some geolocation 
 * routines can calculate (i,j) from (lon,lat). There is no "best" set of routines, so 
 * up to 3 sets can be used. 
 * 
 * Internal Routines
 *      1. Spherical Earth only
 *      2. Very fast and multithreaded, 3.5/12.7 sec on test case 1*
 *      3. Very fast and multithreaded, 3.5/12.7 sec on test case 2*
 *      4. Needed for NWP grids such as Gaussian grids and rotated lat-lon
 *      5. Common projections
 *      6. C, no configure file needed
 *      7. Can transform (i,j) to (lon,lat)
 *      8. Many of the codes trace their origins to operational codes at NCEP
 *
 * GCTPC Library
 *      1. Handles ellipsoidal Earth
 *      2. Slow and multithreaded (OpenMP), 5.5/19.5 sec on test case 1*
 *      3. Slow and multithreaded (OpenMP), 15.7/58.7 sec on test case 2*
 *      4. More projections than internal routines
 *      5. C, no configure file needed
 *      6. Can transform (i,j) to (lon,lat)
 *      7. Can transform (X,Y) to (lon,lat)
 *      8. Can transform (lon,lat) to (X, Y)
 *      9. Library is old but working, no recent development
 *      10. Support for library is unknown
 *
 * Proj4 Library
 *      1. Handles ellipsoidal Earth
 *      2. Slow and no support for OpenMP, 17.8/19.7 sec on test case 1*
 *      3. Slow and no support for OpenMP, 62.8/64.7 sec on test case 2*
 *      4. Proj4 supports pthreads, wgrib2 uses OpenMP
 *      5. More projections than GCTPC, ex. ellipsoidal Lambert Azimuthal Equal Area grid
 *      6. C, requires config script
 *      7. Can transform (i,j) to (lon,lat)
 *      8. Can transform (X,Y) to (lon,lat)
 *      9. Can transform (lon,lat) to (X, Y)
 *      10. Library has active development
 *      11. Support for library is good
 *      12. The first choice by many people.
 *
 * The internal routines are fast, multithreaded but only handle a spherical earth and 
 * cannot transform from (lon,lat) -> (X,Y). The internal routines include grids not 
 * included with GCTPC. 
 * 
 * The GCTPC routines can handle an ellipsoidal Earth. They are 50% slower than the internal 
 * routines for a spherical earth and are acceptable for NOMADs. For an ellipsoidal Earth, 
 * the speed is probably adequate for NOMADS. The lack of support is not a major factor because 
 * the codes are tested and relatively simple. By supporting both GCTPC and Proj4, I am comfortable 
 * that the codes can be adequately tested. 
 * 
 * Proj4 is the Gold standard; it used by many projects, it has good support and updates appear on 
 * a regular basis. Proj4 supports more projections than GCTPC. Unfortunately Proj4 is slow. For a 
 * spherical Earth, Proj4 is probably adequate (slower than gctpc for a spherical earth). For an 
 * ellipsoidal Earth, Proj4 is too slow for Nomads. There have been some efforts to make Proj4 
 * thread safe. Another difficulty with Proj4 is that compiling uses a config script. Config scripts 
 * can cause problems when cross-compiling for compute nodes (some HPC computers). 
 * 
 * Wgrib2 needs the internal routines for grids not supported by GCTPC/Proj4. Wgrib2 needs GCTPC/Proj4 
 * for handling ellipsoidal Earths. The current policy is that Proj4 will be an optional package and 
 * GCTPC will be come the default package in the near future. Support for Proj4 is useful for debugging 
 * and for "keeping the options open". 
 * 
 * ## Usage
 * -proj4 <1|0>
 * 0 - do not use Proj4 for geolocation
 * 1 - enable Proj4 for geolocation
 * 
 * GCTPC is enabled by default.
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 on success
 * 
 * @note Some grids are only supported by the internal routines.
 * 
 * @author Dusan Jovic @date 6/2012
 */
int f_proj4(ARG1) {
    use_proj4 = (strcmp(arg1,"1") == 0);
    return 0;
}

/**
 * Get latitude and longitude values using Proj4.
 * 
 * @param sec Pointer to GRIB sections.
 * @param lon Pointer to longitude values.
 * @param lat Pointer to latitude values.
 * 
 * @return 0 on success, error code otherwise
 * 
 * @author Dusan Jovic @date 6/2012
 */
int proj4_get_latlon(unsigned char **sec, double **lon, double **lat) {

    int nnx, nny, nres, nscan, error;
    unsigned int i, nnpnts;
    double *llat, *llon;

    llat = *lat;
    llon = *lon;
    get_nxny(sec, &nnx, &nny, &nnpnts, &nres, &nscan);

    if (llat != NULL) {
        free(llat);
        free(llon);
        *lat = *lon = llat = llon = NULL;
    }

    if ((*lat = llat = (double *) malloc(sizeof(double) * (size_t) nnpnts)) == NULL) {
        fatal_error("proj4_get_latlon memory allocation failed","");
    }
    if ((*lon = llon = (double *) malloc(sizeof(double) * (size_t) nnpnts)) == NULL) {
        fatal_error("proj4_get_latlon memory allocation failed","");
    }

    if (proj4_init(sec, llon, llat) != 0) {
        return 1;
    }

    /* put x[] and y[] values in lon and lat */
    if (stagger(sec, nnpnts, llon, llat)) fatal_error("proj4: stagger problem","");

    /* proj4 projections */

#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
    for (i = 0; i < nnpnts; i++) {
        llon[i] = llon[i] * dx + x_0;
        llat[i] = llat[i] * dy + y_0;
    }

    error = pj_transform(pj_grid, pj_latlon, (long) nnpnts, (long) 1, llon, llat, NULL);

#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
    for (i = 0; i < nnpnts; i++) {
        llon[i] = llon[i] * RAD_TO_DEG;
        llat[i] = llat[i] * RAD_TO_DEG;
        if (llon[i] < 0.0) llon[i] += 360.0;
    }        
    return error;
}


#else
int f_proj4(ARG1) {
    if (mode == -1) {fprintf(stderr,"Proj4 package not installed\n"); return 1;}
    return 1;
}
int f_proj4_ll2ij(ARG2) {
    if (mode == -1) {fprintf(stderr,"Proj4 package not installed\n"); return 1;}
    return 1;
}
int f_proj4_ij2ll(ARG2) {
    if (mode == -1) {fprintf(stderr,"Proj4 package not installed\n"); return 1;}
    return 1;
}
int f_proj4_ll2i(ARG2) {
    if (mode == -1) {fprintf(stderr,"Proj4 package not installed\n"); return 1;}
    return 1;
}
#endif
