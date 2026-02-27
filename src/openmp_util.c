/** @file
 * @brief OpenMP 3.1+ compatible routines.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 8/2015 | W. Ebisuzaki | Initial
 * 6/2016 | W. Ebisuzaki | Added more functions
 * 4/2022 | W. Ebisuzaki | Now requires OpenMP 3.1 like rest of wgrib2.
 * 5/2023 | W. Ebisuzaki | int_min_array(), int_max_array()
 * 
 * @author Public Domain: Wesley Ebisuzaki @date 8/2015
 */
#include <stdio.h>
#include <limits.h>
#include "wgrib2.h"

#ifdef USE_OPENMP
#include <omp.h>
#endif

/*
 * min_max_array()  returns min/max of the array
 *
 * return 0:  if min max found
 * return 1:  if min max not found, min = max = 0
 */

/**
 * Returns min/max of an array.
 * 
 * @param data Pointer to the array.
 * @param n Number of elements in the array.
 * @param min Pointer to store the minimum value.
 * @param max Pointer to store the maximum value.
 *
 * @return 0 if min/max found, 1 if not found (min = max = 0)
 * 
 * @author Wesley Ebisuzaki @date 8/2015
 */
int min_max_array(float *data, unsigned int n, float *min, float *max) {

    unsigned int first, i;
    float mn, mx;

    if (n == 0) {
        *min = *max = 0.0;
        return 1;
    }

    for (first = 0; first < n; first++) {
        if (DEFINED_VAL(data[first])) break;
    }
    if (first >= n) {
        *min = *max = 0.0;
        return 1;
    }

    mn = mx = data[first];
#ifdef USE_OPENMP
#pragma omp parallel for private(i) reduction(min:mn) reduction(max:mx)
#endif
    for (i = first+1; i < n; i++) {
        if (DEFINED_VAL(data[i])) {
            mn = (mn > data[i]) ? data[i] : mn;
            mx = (mx < data[i]) ? data[i] : mx;
        }
    }

    *min = mn;
    *max = mx;
    return 0;
}

/**
 * Returns min/max of an array which has all values defined.
 *
 * @param data Pointer to the array.
 * @param n Number of elements in the array.
 * @param min Pointer to store the minimum value.
 * @param max Pointer to store the maximum value.
 *
 * @return 0 if min/max found, 1 if not found (min = max = 0)
 *
 * @author Wesley Ebisuzaki @date 8/2015
 */
int min_max_array_all_defined(float *data, unsigned int n, float *min, float *max) {

    float mx, mn;
    unsigned int i;

    if (n == 0) {
        *min = *max = 0.0;
        return 1;
    }

    mn = mx = data[0];
#ifdef USE_OPENMP
#pragma omp parallel for private(i) reduction(min:mn) reduction(max:mx)
#endif
    for (i = 1; i < n; i++) {
        mx = data[i] > mx ? data[i] : mx;
        mn = data[i] < mn ? data[i] : mn;
    }
    *min = mn;
    *max = mx;
    return 0;
} 

/**
 * Find min/max of an integer array.
 *
 * @param data Pointer to the array.
 * @param n Number of elements in the array.
 * @param min Pointer to store the minimum value.
 * @param max Pointer to store the maximum value.
 *
 * @return 0 if min/max found, 1 if not found (min = max = 0)
 */
int int_min_max_array(int *data, unsigned int n, int *min, int *max) {

    unsigned int first, i;
    int  mn, mx;

    *min = *max = 0;
    if (n == 0) return 1;

    for (first = 0; first < n; first++) {
        if (data[first] != INT_MAX) break;
    }
    if (first >= n) return 1;

    mn = mx = data[first];

#ifdef USE_OPENMP
#pragma omp parallel for private(i) reduction(min:mn) reduction(max:mx)
#endif
    for (i = first + 1; i < n; i++) {
        if (data[i] != INT_MAX) {
            mx = data[i] > mx ? data[i] : mx;
            mn = data[i] < mn ? data[i] : mn;
        }
    }

    *min = mn;
    *max = mx;
    return 0;
}

/**
 * Find minimum value in an integer array.
 *
 * @param data Pointer to the array.
 * @param n Number of elements in the array.
 *
 * @return Minimum value if found, INT_MAX (the undefined value) if not found.
 *
 * @author Wesley Ebisuzaki @date 5/2023
 */
int int_min_array(int *data, unsigned int n) {

    unsigned int first, i;
    int mn;

    if (n == 0) return INT_MAX;
    for (first = 0; first < n; first++) {
        if (data[first] != INT_MAX) break;
    }
    if (first >= n) return INT_MAX;
    mn = data[first];

#ifdef USE_OPENMP
#pragma omp parallel for private(i) reduction(min:mn)
#endif
    for (i = first+1; i < n; i++) {
        if (data[i] != INT_MAX) {
            mn = data[i] < mn ? data[i] : mn;
        }
    }

    return mn;
}

/*
 * int_max_array()
 * return INT_MAX: max not found
 * return i: max of array
 * INT_MAX is the undefined value
 */

/**
 * Find maximum value in an integer array.
 *
 * @param data Pointer to the array.
 * @param n Number of elements in the array.
 *
 * @return Maximum value if found, INT_MAX (the undefined value) if not found.
 *
 * @author Wesley Ebisuzaki @date 5/2023
 */
int int_max_array(int *data, unsigned int n) {

    unsigned int first, i;
    int mx;

    if (n == 0) return INT_MAX;
    for (first = 0; first < n; first++) {
        if (data[first] != INT_MAX) break;
    }
    if (first >= n) return INT_MAX;
    mx = data[first];

#ifdef USE_OPENMP
#pragma omp parallel for private(i) reduction(max:mx)
#endif
    for (i = first+1; i < n; i++) {
        if (data[i] != INT_MAX) {
            mx = data[i] > mx ? data[i] : mx;
        }
    }

    return mx;
}
