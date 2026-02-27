/** @file
 * @brief Header file defining mathematical constants.
 * @author Public Domain: Manfred Schwarb @date 9/2025
 */

#ifndef _WMATH_H_
#define _WMATH_H_

/*
 * math constants usually provided by math.h, define them in any case as not ANSI C.
 * Values are calculated with bc command, correct up to 20 significant digits to
 * accommodate 80bit excess precision and are identical with recent glibc values.
 */
#define M_PI           3.14159265358979323846  /**< pi */
#define M_PI_2         1.57079632679489661923  /**< pi/2 */
#define M_PI_4         0.78539816339744830962  /**< pi/4 */
#define M_SQRT2        1.41421356237309504880  /**< sqrt(2) */

/* constants for conversion from and to degrees and radians */
#define DEG_TO_RAD     0.017453292519943295769 /**< pi/180 */
#define RAD_TO_DEG     57.295779513082320877   /**< 180/pi */

#endif /* _WMATH_H_ */
