/** @file
 * @brief Unit Conversion Factors
 *
 * PURPOSE: This function determines the conversion factor between the
 *          input unit type and the output unit type.  Valid types include:
 *          - 0 = Radians
 *          - 1 = U.S. feet
 *          - 2 = Meters
 *          - 3 = Seconds of arc
 *          - 4 = Degrees of arc
 *          - 5 = International feet
 * @author T. Mittan @date March, 1993
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

/** List of conversion factors */
static double factors[6][6] = {
	{1.0, 0.0, 0.0, R2S, R2D, 0.0},
	{0.0, 1.0, .3048006096012192, 0.0, 0.0, 1.000002000004},
	{0.0, 3.280833333333333, 1.0, 0.0, 0.0, 3.280839895013124},
	{S2R, 0.0, 0.0, 1.0, 1.0/3600, 0.0},
	{D2R, 0.0, 0.0, 3600, 1.0, 0.0},
	{0.0, .999998, .3048, 0.0, 0.0, 1.0}};

/**
 * Determines the conversion factor between input and output unit types.
 * 
 * Valid unit types include:
 * - 0 = Radians
 * - 1 = U.S. feet
 * - 2 = Meters
 * - 3 = Seconds of arc
 * - 4 = Degrees of arc
 * - 5 = International feet
 *
 * @param inunit Input unit type (0-5)
 * @param outunit Output unit type (0-5)
 * @param factor Pointer to the conversion factor
 * 
 * @return
 * - 0 :: Success
 * - 5 :: Illegal source or target unit code
 * - 1101 :: Incompatible unit codes
 *
 * @author T. Mittan @date March, 1993
 */
long untfz(long inunit, long outunit, double *factor) {
//long untfz(inunit,outunit,factor)
//
//long inunit;
//long outunit;
//double *factor;
//{
if ((outunit >= 0) && (outunit <= MAXUNIT) && (inunit >= 0)
						&& (inunit <= MAXUNIT))
   {
   *factor = factors[inunit][outunit];

   /* Angle units can not be converted to length units
     ------------------------------------------------*/
   if (*factor == 0.0)
       {
       p_error("Incompatable unit codes","untfz-code");
       return(1101);
       }
   }
else
  {
  p_error("Illegal source or target unit code","untfz-unit");
  return(5);
  }

return(OK);
}
