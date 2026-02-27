/** @file
 * @brief Header file for wgrib2. Contains definitions for grid identification.
 * @author Public Domain: Wesley Ebisuzaki @date 10/2024
 */

/** Grid projection types. */
enum projection_type { p_unknown, p_latlon, p_lambert_conic, p_polar_stereographic, 
   p_mercator, p_rotated_latlon, p_stretched_latlon, p_rotated_streched_latlon, p_lambert_azimuthal };

/** Grid arrangement types. */
enum grid_arrangement_type { rectangular, thinned_rectangular_regional, thinned_rectangular_global, gaussian, thinned_gaussian, staggered_B, list };

/** Maximum number of projection arguments. */
#define N_proj_args 20

/** Structure for grid type definition. */
struct grid_type { 
    double x0; /**< X coordinate of the grid's origin */
    double y0; /**< Y coordinate of the grid's origin */
    int x0_y0_in_m; /**< Flag indicating if x0 and y0 are in meters */
    int nx; /**< Number of grid points in the x direction */
    int ny; /**< Number of grid points in the y direction */
    int n; /**< Total number of grid points */
    int valid_dx_dy; /**< Flag indicating if dx and dy are valid */
    double dx; /**< Grid spacing in the x direction */
    double dy; /**< Grid spacing in the y direction */
    int dx_dy_in_m; /**< Flag indicating if dx and dy are in meters */
    int valid_xn_yn; /**< Flag indicating if xn and yn are valid */
    double xn; /**< X coordinate of the grid's endpoint */
    double yn; /**< Y coordinate of the grid's endpoint */
    int xn_yn_in_m; /**< Flag indicating if xn and yn are in meters */
    enum grid_arrangement_type grid_arrangement; /**< Grid arrangement type */
    int valid_xy_list; /**< Flag indicating if xlist and ylist are valid */
    double *xlist; /**< List of x coordinates */
    double *ylist; /**< List of y coordinates */
    int xy_list_in_m; /**< Flag indicating if xlist and ylist are in meters */
};

/** Macro for function arguments used by grid_id(). */
#define GRID_ID_ARGS unsigned char **sec, double *r_major, double *r_minor, enum projection_type *proj_id, double *proj_args, int n_proj_args, struct grid_type *grid_defn

int grid_id(GRID_ID_ARGS);


