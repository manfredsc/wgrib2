/** @file
 * @brief Projection support routines.
 * 
 * PURPOSE: The following functions are included in REPORT.C
 *
 *    INIT: Initializes the output device for error messages and
 *          report headings.
 *
 *    P_ERROR: Reports errors to the terminal, a specified file, or
 *             both.
 *
 *    PTITLE, RADIUS, RADIUS2, CENLON, CENLONMER, CENLAT, ORIGIN,
 *    STANPARL, STPARL1, OFFSET, GENRPT, GENRPT_LONG, PBLANK:
 *          Reports projection parameters to the terminal,
 *          specified file, or both.
 * @author D. Steinwand, EROS @date July, 1991
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 7/1991 | D. Steinwand | Initial development.
 * 3/1993 | T. Mittan    | Adapted code to new "C" version of GCTP library.
 * 6/1993 | S. Nelson    | Added inline code. 
 *                         Added error messages if no filename was specified.
 * 1/1998 | S. Nelson    | Returned OK instead of 0.
 */

#include <stdio.h>
#include <string.h>
#include "cproj.h"

#define TRUE 1       /**< TRUE boolean value */
#define FALSE 0      /**< FALSE boolean value */

static long terminal_p;		/**< flag for printing parameters to terminal */
static long terminal_e;		/**< flag for printing errors to terminal */
static long file_p;		/**< flag for printing parameters to file */
static long file_e;		/**< flag for printing errors to file */
static FILE  *fptr_p;      /**< file pointer for parameters file */
static FILE  *fptr_e;      /**< file pointer for errors file */
static char parm_file[256];   /**< name of parameters file */
static char err_file[256];    /**< name of errors file */

/** 
 * Initialize output device for error messages and report headings.
 * 
 * @param ipr Flag for printing errors (0, 1, or 2)
 * @param jpr Flag for printing parameters (0, 1, or 2)
 * @param efile Name of error file
 * @param pfile Name of parameter file
 * 
 * @return
 * - 0 :: Success
 * - 6 :: File name not specified.
 * 
 * @author D. Steinwand, EROS @date July, 1991
 */
long init(long ipr, long jpr, char *efile, char *pfile) {
//long init(ipr,jpr,efile,pfile)
//
//long ipr;		/* flag for printing errors (0,1,or 2)		*/
//long jpr;		/* flag for printing parameters (0,1,or 2)	*/
//char *efile;		/* name of error file				*/
//char *pfile;		/* name of parameter file			*/
//
//{
if (ipr == 0)
   {
   terminal_e = TRUE;
   file_e = FALSE;
   }
else
if (ipr == 1)
   {
   terminal_e = FALSE;
   if (strlen(efile) == 0)
      {
      return(6);
      }
   file_e = TRUE;
   strcpy(err_file,efile);
   }
else
if (ipr == 2)
   {
   terminal_e = TRUE;
   if (strlen(efile) == 0)
      {
      file_e = FALSE;
      p_error("Output file name not specified","report-file");
      return(6);
      }
   file_e = TRUE;
   strcpy(err_file,efile);
   }
else
   {
   terminal_e = FALSE;
   file_e = FALSE;
   }
if (jpr == 0)
   {
   terminal_p = TRUE;
   file_p = FALSE;
   }
else
if (jpr == 1)
   {
   terminal_p = FALSE;
   if (strlen(pfile) == 0)
      {
      return(6);
      }
   file_p = TRUE;
   strcpy(parm_file,pfile);
   }
else
if (jpr == 2)
   {
   terminal_p = TRUE;
   if (strlen(pfile) == 0)
      {
      file_p = FALSE;
      p_error("Output file name not specified","report-file");
      return(6);
      }
   file_p = TRUE;
   strcpy(parm_file,pfile);
   }
else
   {
   terminal_p = FALSE;
   file_p = FALSE;
   }
return(OK);
}

/**
 * Close output files.
 * 
 * @author D. Steinwand, EROS @date July, 1991
 */
void close_file()
{
if (fptr_e != NULL) {
   fclose(fptr_e);
   fptr_e = NULL;
}
if (fptr_p != NULL) {
   fclose(fptr_p);
   fptr_p = NULL;
}
}

/* Functions to report projection parameters */

/**
 * Report the title for the projection parameters to the terminal, output 
 * file, or both.
 * 
 * @param A Pointer to projection title string
 * 
 * @author D. Steinwand, EROS @date July, 1991
 */
void ptitle(char *A) {
   if (terminal_p)
      printf("\n%s PROJECTION PARAMETERS:\n\n",A); 
   if (file_p)
   {
      fptr_p = (FILE *)fopen(parm_file,"a");
      fprintf(fptr_p,"\n%s PROJECTION PARAMETERS:\n\n",A); 
      fclose(fptr_p);
      fptr_p = NULL;
   }
}

/**
 * Report the radius of the sphere to the terminal, output file, or both.
 * 
 * @param A Sphere radius in meters
 * 
 * @author D. Steinwand, EROS @date July, 1991
 */
void radius(double A)
{
   if (terminal_p)
      printf("   Radius of Sphere:     %lf meters\n",A); 
   if (file_p)
   {
      fptr_p = (FILE *)fopen(parm_file,"a");
      fprintf(fptr_p,"   Radius of Sphere:     %lf meters\n",A); 
      fclose(fptr_p);
      fptr_p = NULL;
   }
}

/**
 * Report the semi-major and semi-minor axes of the ellipsoid to the 
 * terminal, output file, or both.
 * 
 * @param A Semi-major axis in meters
 * @param B Semi-minor axis in meters
 * 
 * @author D. Steinwand, EROS @date July, 1991
 */
void radius2(double A, double B)
{
   if (terminal_p)
   {
      printf("   Semi-Major Axis of Ellipsoid:     %lf meters\n",A);
      printf("   Semi-Minor Axis of Ellipsoid:     %lf meters\n",B);
   }
   if (file_p)
   {
      fptr_p = (FILE *)fopen(parm_file,"a");
      fprintf(fptr_p,"   Semi-Major Axis of Ellipsoid:     %lf meters\n",A);
      fprintf(fptr_p,"   Semi-Minor Axis of Ellipsoid:     %lf meters\n",B); 
      fclose(fptr_p);
      fptr_p = NULL;
   }
}

/**
 * Report the longitude of the center to the terminal, output file, or both.
 * 
 * @param A Longitude of the center in degrees
 * 
 * @author D. Steinwand, EROS @date July, 1991
 */
void cenlon(double A)
{ 
   if (terminal_p)
      printf("   Longitude of Center:     %lf degrees\n",A*R2D);
   if (file_p)
   {
      fptr_p = (FILE *)fopen(parm_file,"a");
      fprintf(fptr_p,"   Longitude of Center:     %lf degrees\n",A*R2D);
      fclose(fptr_p);
      fptr_p = NULL;
   }
}

/**
 * Report the longitude of the central meridian to the terminal, output file, or both.
 *
 * @param A Longitude of the central meridian in degrees
 *
 * @author D. Steinwand, EROS @date July, 1991
 */
void cenlonmer(double A)
{ 
   if (terminal_p)
      printf("   Longitude of Central Meridian:     %lf degrees\n",A*R2D);
   if (file_p)
   {
      fptr_p = (FILE *)fopen(parm_file,"a");
      fprintf(fptr_p,"   Longitude of Central Meridian:     %lf degrees\n",A*R2D);
      fclose(fptr_p);
      fptr_p = NULL;
   }
}

/**
 * Report the latitude of the center to the terminal, output file, or both.
 *
 * @param A Latitude of the center in degrees
 *
 * @author D. Steinwand, EROS @date July, 1991
 */
void cenlat(double A)
{
   if (terminal_p)
      printf("   Latitude  of Center:     %lf degrees\n",A*R2D);
   if (file_p)
   {
      fptr_p = (FILE *)fopen(parm_file,"a");
      fprintf(fptr_p,"   Latitude of Center:     %lf degrees\n",A*R2D);
      fclose(fptr_p);
      fptr_p = NULL;
   }
}

/**
 * Report the latitude of the origin to the terminal, output file, or both.
 *
 * @param A Latitude of the origin in degrees
 *
 * @author D. Steinwand, EROS @date July, 1991
 */
void origin(double A)
{
   if (terminal_p)
      printf("   Latitude of Origin:     %lf degrees\n",A*R2D);
   if (file_p)
   {
      fptr_p = (FILE *)fopen(parm_file,"a");
      fprintf(fptr_p,"   Latitude  of Origin:     %lf degrees\n",A*R2D);
      fclose(fptr_p);
      fptr_p = NULL;
   }
}

/**
 * Report the 1st and 2nd standard parallels to the terminal, output file, or both.
 *
 * @param A 1st standard parallel in degrees
 * @param B 2nd standard parallel in degrees
 *
 * @author D. Steinwand, EROS @date July, 1991
 */
void stanparl(double A, double B)
{
   if (terminal_p)
   {
      printf("   1st Standard Parallel:     %lf degrees\n",A*R2D);
      printf("   2nd Standard Parallel:     %lf degrees\n",B*R2D);
   }
   if (file_p)
   {
      fptr_p = (FILE *)fopen(parm_file,"a");
      fprintf(fptr_p,"   1st Standard Parallel:     %lf degrees\n",A*R2D);
      fprintf(fptr_p,"   2nd Standard Parallel:     %lf degrees\n",B*R2D);
      fclose(fptr_p);
      fptr_p = NULL;
   }
}

/**
 * Report the standard parallel to the terminal, output file, or both.
 *
 * @param A Standard parallel in degrees
 *
 * @author D. Steinwand, EROS @date July, 1991
 */
void stparl1(double A)
{
   if (terminal_p)
   {
      printf("   Standard Parallel:     %lf degrees\n",A*R2D);
   }
   if (file_p)
   {
      fptr_p = (FILE *)fopen(parm_file,"a");
      fprintf(fptr_p,"   Standard Parallel:     %lf degrees\n",A*R2D);
      fclose(fptr_p);
      fptr_p = NULL;
   }
}

/**
 * Report the false easting and northing to the terminal, output file, or both.
 *
 * @param A False easting in meters
 * @param B False northing in meters
 *
 * @author D. Steinwand, EROS @date July, 1991
 */
void offsetp(double A, double B) 
{
   if (terminal_p)
   {
      printf("   False Easting:      %lf meters \n",A);
      printf("   False Northing:     %lf meters \n",B);
   }
   if (file_p)
   {
      fptr_p = (FILE *)fopen(parm_file,"a");
      if (fptr_p != NULL)
      {
         fprintf(fptr_p,"   False Easting:      %lf meters \n",A);
         fprintf(fptr_p,"   False Northing:     %lf meters \n",B);
         fclose(fptr_p);
         fptr_p = NULL;
      }
      else
      {
         fprintf(stderr,"Could not open %s",parm_file);
      }
   }      
}

/**
 * Report a double-valued parameter to the terminal, output file, or both.
 *
 * @param A Value of the parameter
 * @param S Name of the parameter
 *
 * @author D. Steinwand, EROS @date July, 1991
 */
void genrpt(double A, char *S)
{
   if (terminal_p)
      printf("   %s %lf\n", S, A);
   if (file_p)
   {
      fptr_p = (FILE *)fopen(parm_file,"a");
      fprintf(fptr_p,"   %s %lf\n", S, A);
      fclose(fptr_p);
      fptr_p = NULL;
   }
}

/**
 * Report a long-valued parameter to the terminal, output file, or both.
 *
 * @param A Value of the parameter
 * @param S Name of the parameter
 *
 * @author D. Steinwand, EROS @date July, 1991
 */
void genrpt_long(long A, char *S)
{
   if (terminal_p)
      printf("   %s %ld\n", S, A);
   if (file_p)
   {
      fptr_p = (FILE *)fopen(parm_file,"a");
      fprintf(fptr_p,"   %s %ld\n", S, A);
      fclose(fptr_p);
      fptr_p = NULL;
   }
}

/**
 * Report a blank line to the terminal, output file, or both.
 *
 * @author D. Steinwand, EROS @date July, 1991
 */
void pblank() 
{
   if (terminal_p)
      printf("\n");
   if (file_p)
   {
      fptr_p = (FILE *)fopen(parm_file,"a");
      fprintf(fptr_p,"\n");
      fclose(fptr_p);
      fptr_p = NULL;
   }
}

/* Function to report errors  */

/**
 * Report an error message to the terminal, output file, or both.
 *
 * @param what Error message
 * @param where Location of the error
 *
 * @author D. Steinwand, EROS @date July, 1991
 */
void p_error(char *what, char *where) 
{
   if (terminal_e)
      printf("[%s] %s\n",where,what);
   if (file_e)
   {
      fptr_e = (FILE *)fopen(err_file,"a");
      fprintf(fptr_e,"[%s] %s\n",where,what);
      fclose(fptr_e);
      fptr_e = NULL;
   }
}
