/** @file
 * @brief Interface routines to the gctpc library.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 2/2012 | W. Ebisuzaki | Initial
 * 5/2014 | W. Ebisuzaki | Added staggering
 * 10/2015 | W. Ebisuzaki | Added Lambert Azimuthal Equal Area Projection
 * 
 * @author Public Domain: Wesley Ebisuzaki @date 2/2012
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cproj.h"
#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"

/* Gctpc.c  interface routines to the gctpc library
   2/2012 Public Domain Wesley Ebisuzaki

  gctpc_get_latlon: fill grid with lat/lon values

  mercator 10
  polar stereographic 20
  lambert conformal 30
  Albers equal area 31

  5/2014 added staggering
  10/2015 added Lambert Azimuthal Equal Area Projection
*/

/* M_PI, M_PI_2, M_PI_4, and M_SQRT2 are not ANSI C but are commonly defined */
/* values from GNU C library version of math.h copyright Free Software Foundation, Inc. */

#ifndef M_PI
#define M_PI           3.14159265358979323846  /**< pi */
#endif
#ifndef M_PI_2
#define M_PI_2         1.57079632679489661923  /**< pi/2 */
#endif
#ifndef M_PI_4
#define M_PI_4         0.78539816339744830962  /**< pi/4 */
#endif
#ifndef M_SQRT2
#define M_SQRT2        1.41421356237309504880  /**< sqrt(2) */
#endif

/** Flag indicating whether to use the gctpc library */
extern int use_gctpc;

/** Flag to indicate lat-lon grid processing.  */
extern int latlon;

/** Pointer to longitude values */
extern double *lon;

/** Pointer to latitude values */
extern double *lat;

/** Current output order type. */
extern enum output_order_type output_order;


/*
 * HEADER:100:gctpc:misc:1: X=0,1 use gctpc library  (default=1)
 */

/**
 * Set flag to use gctpc library.
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
 * -gctpc <1|0>
 * 0 - do not use gctpc for geolocation
 * 1 - enabled gctpc for geolocation (default)
 * 
 * @param ARG1 List of function arguments set by wgrib2's main() function (see @ref ARG1). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 *
 * @return 0 for success
 * 
 * @note Some grids are only supported by the internal routines.
 * 
 * @author Wesley Ebisuzaki @date 2/2012
 */
int f_gctpc(ARG1) {
   use_gctpc = (strcmp(arg1,"1") == 0);
   return 0;
}


/* get lat-lon for grid
 *
 * step 1: initialize to center point
 * step 2: find (x,y) of lon1/lat1 (1st grid point)  (x0,y0)
 * step 3  find (x,y) of grid
 * step 4  find lat-lon of (x,y)
 */

/**
 * Get lat-lon for grid.
 *
 * Step 1: initialize to center point
 * Step 2: find (x,y) of lon1/lat1 (1st grid point)  (x0,y0)
 * Step 3: find (x,y) of grid
 * Step 4: find lat-lon of (x,y)
 *
 * @param sec Pointer to grib sections of grid.
 * @param lon Pointer to longitude array.
 * @param lat Pointer to latitude array.
 *
 * @return 0 for success, error code otherwise
 *
 * @author Wesley Ebisuzaki @date 2/2012
 */
int gctpc_get_latlon(unsigned char **sec, double **lon, double **lat) {

    int gdt;
    unsigned char *gds;

    double r_maj;                           /* major axis                   */
    double r_min;                           /* minor axis                   */
    double lat1;                            /* first standard parallel      */
    double lat2;                            /* second standard parallel     */
    double c_lon;                           /* center longitude             */
    double c_lat;                           /* center latitude              */
    double false_east;                      /* x offset in meters           */
    double false_north;
    double dx, dy;
    double x0, y0;
    long int (*inv_fn)(double, double, double *, double *);
    double *llat, *llon, rlon, rlat;

    int nnx, nny, nres, nscan, is_spherical;
    unsigned int i, nnpnts;
    long g_error;

    gdt = code_table_3_1(sec);
    gds = sec[3];

    /* only process certain grids */

    if (gdt != 10 && gdt != 20 && gdt != 30 && gdt != 31 && gdt != 140) return 1;
    get_nxny(sec, &nnx, &nny, &nnpnts, &nres, &nscan);
    /* get earth axis */
    axes_earth(sec, &r_maj, &r_min, &is_spherical);

    /* potentially staggered */
// 8/2014    if (nnx < 1 || nny < 1) return 1;

    llat = *lat;
    llon = *lon;

    if (llat != NULL) {
        free(llat);
        free(llon);
        *lat = *lon = llat = llon = NULL;
    }

    inv_fn = NULL;
    dx = dy = 0.0;

    if (gdt == 10) {            // mercator

        dy      = GDS_Mercator_dy(gds);
        dx      = GDS_Mercator_dx(gds);

        /* central point */
        c_lon = GDS_Mercator_ori_angle(gds) * (M_PI/180.0);
        c_lat = GDS_Mercator_latD(gds) * (M_PI/180.0);

        /* find the eastling and northing of of the 1st grid point */

        false_east = false_north = 0.0;
        g_error = merforint(r_maj,r_min,c_lon,c_lat,false_east,false_north);
        if (g_error) fatal_error_i("merforint %ld", g_error);

        rlon   = GDS_Mercator_lon1(gds) * (M_PI/180.0);
        rlat   = GDS_Mercator_lat1(gds) * (M_PI/180.0);

        g_error = merfor(rlon, rlat, &x0, &y0);
        if (g_error) fatal_error_i("merfor %ld", g_error);

        /* initialize for 1st grid point */
        x0 = -x0;
        y0 = -y0;
        g_error = merinvint(r_maj,r_min,c_lon,c_lat,x0,y0);
        if (g_error) fatal_error_i("merinvint %ld", g_error);
        inv_fn = &merinv;
    }

    else if (gdt == 20) {            // polar stereographic

        dy      = GDS_Polar_dy(gds);
        dx      = GDS_Polar_dx(gds);

        /* central point */
        c_lon = GDS_Polar_lov(gds) * (M_PI/180.0);
        c_lat = GDS_Polar_lad(gds) * (M_PI/180.0);

        /* find the eastling and northing of of the 1st grid point */

        false_east = false_north = 0.0;
        g_error = psforint(r_maj,r_min,c_lon,c_lat,false_east,false_north);
        if (g_error) fatal_error_i("psforint %ld", g_error);

        rlon   = GDS_Polar_lon1(gds) * (M_PI/180.0);
        rlat   = GDS_Polar_lat1(gds) * (M_PI/180.0);

        g_error = psfor(rlon, rlat, &x0, &y0);
        if (g_error) fatal_error_i("psfor %ld", g_error);

        /* initialize for 1st grid point */
        x0 = -x0;
        y0 = -y0;
        g_error = psinvint(r_maj,r_min,c_lon,c_lat,x0,y0);
        if (g_error) fatal_error_i("psinvint %ld", g_error);
        inv_fn = &psinv;
    }

    else if (gdt == 30) {            // lambert conformal conic

        dy      = GDS_Lambert_dy(gds);
        dx      = GDS_Lambert_dx(gds);
        //printf(">>> gctpc dx %lf, dy %lf\n", dx, dy);
        /* latitudes of tangent/intersection */
        lat1 = GDS_Lambert_Latin1(gds) * (M_PI/180.0);
        lat2 = GDS_Lambert_Latin2(gds) * (M_PI/180.0);

        /* central point */
        c_lon = GDS_Lambert_Lov(gds) * (M_PI/180.0);
        c_lat = GDS_Lambert_LatD(gds) * (M_PI/180.0);

        /* find the eastling and northing of of the 1st grid point */

        false_east = false_north = 0.0;
        g_error = lamccforint(r_maj,r_min,lat1,lat2,c_lon,c_lat,false_east,false_north);
        if (g_error) fatal_error_i("lamccforint %ld", g_error);

        rlon   = GDS_Lambert_Lo1(gds) * (M_PI/180.0);
        rlat   = GDS_Lambert_La1(gds) * (M_PI/180.0);

        g_error = lamccfor(rlon, rlat, &x0, &y0);
        if (g_error) fatal_error_i("lamccfor %ld", g_error);

        /* initialize for 1st grid point */
        x0 = -x0;
        y0 = -y0;
        g_error = lamccinvint(r_maj,r_min,lat1,lat2,c_lon,c_lat,x0,y0);
        if (g_error) fatal_error_i("lamccinvint %ld", g_error);
        inv_fn = &lamccinv;
    }
    else if (gdt == 31) {			// albers equal area
        dy      = GDS_Albers_dy(gds);
        dx      = GDS_Albers_dx(gds);

        /* latitudes of tangent/intersection */
        lat1 = GDS_Albers_Latin1(gds) * (M_PI/180.0);
        lat2 = GDS_Albers_Latin2(gds) * (M_PI/180.0);

        /* central point */
        c_lon = GDS_Albers_Lov(gds) * (M_PI/180.0);
        c_lat = GDS_Albers_LatD(gds) * (M_PI/180.0);

        /* find the eastling and northing of of the 1st grid point */

        false_east = false_north = 0.0;
        g_error = alberforint(r_maj,r_min,lat1,lat2,c_lon,c_lat,false_east,false_north);
        if (g_error) fatal_error_i("alberforint %ld", g_error);

        rlon   = GDS_Albers_Lo1(gds) * (M_PI/180.0);
        rlat   = GDS_Albers_La1(gds) * (M_PI/180.0);

        g_error = alberfor(rlon, rlat, &x0, &y0);
        if (g_error) fatal_error_i("alberfor %ld", g_error);

        /* initialize for 1st grid point */
        x0 = -x0;
        y0 = -y0;
        g_error = alberinvint(r_maj,r_min,lat1,lat2,c_lon,c_lat,x0,y0);
        if (g_error) fatal_error_i("alberinvint %ld", g_error);
        inv_fn = &alberinv;
    }
    else if (gdt == 140 && is_spherical) {            // lambert azimuthal equal area

        dy      = GDS_Lambert_Az_dy(gds);
        dx      = GDS_Lambert_Az_dx(gds);

        /* central point */
        c_lon = GDS_Lambert_Az_Cen_Lon(gds) * (M_PI/180.0);
        c_lat = GDS_Lambert_Az_Std_Par(gds) * (M_PI/180.0);

        /* find the eastling and northing of of the 1st grid point */

        false_east = false_north = 0.0;
        g_error = lamazforint(r_maj,c_lon,c_lat,false_east,false_north);
        if (g_error) fatal_error_i("lamazforint %ld", g_error);

        rlon  = GDS_Lambert_Az_Lo1(gds) * (M_PI/180.0);
        rlat  = GDS_Lambert_Az_La1(gds) * (M_PI/180.0);
        g_error = lamazfor(rlon, rlat, &x0, &y0);
        if (g_error) fatal_error_i("lamazfor %ld", g_error);

        /* initialize for 1st grid point */
        x0 = -x0;
        y0 = -y0;
        g_error = lamazinvint(r_maj,c_lon,c_lat,x0,y0);
        if (g_error) fatal_error_i("lamazinvint %ld", g_error);
        inv_fn = &lamazinv;
    }

    if (inv_fn == NULL)  return 1;

    if ((*lat = llat = (double *) malloc(((size_t) nnpnts) * sizeof(double))) == NULL) {
        fatal_error("gctpc_get_latlon memory allocation failed","");
    }
    if ((*lon = llon = (double *) malloc(((size_t) nnpnts) * sizeof(double))) == NULL) {
        fatal_error("gctpc_get_latlon memory allocation failed","");
    }

    /* put x[] and y[] values in lon and lat */
    if (stagger(sec, nnpnts, llon, llat)) fatal_error("gctpc: stagger problem","");

//    printf(">> stagger gctpc x00 %lf y00 %lf\n",llon[0], llat[0]);
#ifdef USE_OPENMP
#pragma omp parallel for private(i)
#endif
    for (i = 0; i < nnpnts; i++) {
        inv_fn(llon[i]*dx, llat[i]*dy, llon+i, llat+i);
        llat[i] *= (180.0 / M_PI);
        llon[i] *= (180.0 / M_PI);
        if (llon[i] < 0.0) llon[i] += 360.0;
    }
    return 0;
}


/*
 * HEADER:100:ll2ij:inv:2:x=lon y=lat, converts lon-lat to (i,j) using gctpc
 */

/**
 * Converts lon-lat to (i,j) using gctpc (alpha).
 * 
 * The -lon option uses a brute-force method to find the closest grid point to a specified 
 * latitude and longitude. It finds the minimum distance to each grid point. This slow 
 * procedure is more-or-less necessary when your geolocation routines can transfrom from 
 * (X,Y) -> (lon, lat) but not (lon, lat) -> (X, Y). The gctpc/Proj4 geolocation libraries 
 * have both the forward and inverse transformation so improved geolocation routines can be 
 * added to wgrib2 such as a fast -lon option and a bilinear interpolation option. 
 * 
 * Some grids only have an (i,j) -> (lon,lat) transformation. (I.e., find that lat-lon 
 * of the grid points.) Examples include: staggered grids, thinned Gaussian grids and 
 * irregular grids. 
 * 
 * This option takes a given latitude and longitude, finds the grid point that is closest 
 * to that specified latitude and longitude and prints out the ix and iy of the grid point.
 * 
 * ## Usage
 * -ll2ij lon lat
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 *
 * @author Wesley Ebisuzaki @date 2/2012
 */
int f_ll2ij(ARG2) {

    double x[1], y[1], to_lat[1], to_lon[1];
    int i;

    if (mode == -1) {
        latlon = 1;
    }
    if (mode >= 0) {
        if (output_order != wesn) return 1;
        to_lon[0] = atof(arg1);
        to_lat[0] = atof(arg2);
        i = gctpc_ll2xy_init(sec, lon, lat);
        if (i == 0)  {
            i = gctpc_ll2xy(1, to_lon, to_lat, x , y);
            sprintf(inv_out,"%lf %lf -> (%lf,%lf)",to_lon[0], to_lat[0], x[0]+1.0, y[0]+1.0);
        }
    }
    return 0;
}

/*
 * HEADER:100:ll2i:inv:2:x=lon y=lat, converts to (i), 1..ndata
 */

/**
 * Converts lon-lat to (i) using gctpc (alpha).
 * 
 * This option uses the gctpc library and only supports a grids supported by gctpc. 
 * 
 * ## Usage
 * -ll2i lon lat
 * 
 * @param ARG2 List of function arguments set by wgrib2's main() function (see @ref ARG2). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Wesley Ebisuzaki @date 2/2012
 */
int f_ll2i(ARG2) {

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
        i = gctpc_ll2xy_init(sec, lon, lat);
        if (i != 0) {
            iptr = 0;	
        }
        else {
            i = gctpc_ll2i(1, to_lon, to_lat, &iptr);
            if (i != 0) iptr = 0;
        }
        sprintf(inv_out,"%lf %lf -> (%u)",to_lon[0], to_lat[0], iptr);
    }
    return 0;
}
