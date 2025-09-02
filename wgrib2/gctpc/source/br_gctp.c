/** @file
 * @brief Fortran bridge routine to call gctp() for UNIX.
 * @author Wesley Ebisuzaki @date 2020
 */

/**
 * Fortran bridge routine to call gctp() for UNIX.
 *
 * @param incoor Input coordinates
 * @param insys Input projection code
 * @param inzone Input zone number
 * @param inparm Input projection parameter array	
 * @param inunit Input units
 * @param inspheroid Input spheroid
 * @param ipr Printout flag for error messages. 0=yes, 1=no
 * @param efile Error file name
 * @param jpr Printout flag for projection parameters. 0=yes, 1=no
 * @param pfile Projection file name
 * @param outcoor Output coordinates
 * @param outsys Output projection code
 * @param outzone Output zone number
 * @param outparm Output projection parameter array
 * @param outunit Output units
 * @param outspheroid Output spheroid
 * @param fn27 Filename for NAD 1927 parameter file
 * @param fn83 Filename for NAD 1983 parameter file
 * @param iflg Error flag
 *
 * @author Wesley Ebisuzaki @date 2020
 */
#ifdef unix
#include "cproj.h"
/*  Fortran bridge routine for the UNIX */

void gctp_(incoor,insys,inzone,inparm,inunit,inspheroid,ipr,efile,jpr,pfile,
            outcoor,outsys,outzone,outparm,outunit,outspheroid,fn27,fn83,iflg)

double *incoor;
long *insys;
long *inzone;
double *inparm;
long *inunit;
long *inspheroid;
long *ipr;        /* printout flag for error messages. 0=yes, 1=no*/
char *efile;
long *jpr;        /* printout flag for projection parameters 0=yes, 1=no*/
char *pfile;
double *outcoor;
long *outsys;
long *outzone;
double *outparm;
long *outunit;
long *outspheroid;
long *iflg;
/* 2/2024 Wesley Ebisuzaki added 2 lines */
char *fn27;
char *fn83;
{
    gctp(incoor,insys,inzone,inparm,inunit,inspheroid,ipr,efile,jpr,pfile,outcoor,
        outsys,outzone,outparm,outunit,outspheroid,fn27,fn83,iflg);
    return;
}
#endif
