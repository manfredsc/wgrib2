/** @file
 * @brief Space Oblique Mercator (SOM) - Inverse Transformation
 *
 * PURPOSE: The first method of Transforming input Easting and
 *          Northing to longitude and latitude for the SOM projection.
 *          The Easting and Northing must be in meters.  The
 *          longitude and latitude values will be returned in radians.
 * @author D. Steinwand @date July, 1992
 * 
 * ### Algorithm References
 * 1. Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
 *    Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
 *    State Government Printing Office, Washington D.C., 1987.
 *
 * 2. "Software Documentation for GCTP General Cartographic Transformation
 *    Package", U.S. Geological Survey National Mapping Division, May 1982.
 * 
 * ### Program History Log
 * Date | Programmer | Comments
 * -----|------------|---------
 * 7/1992 | D. Steinwand | Initial implementation
 * 3/1993 | T. Mittan    | Update
 */

#include "cproj.h"
#define LANDSAT_RATIO 0.5201613   /**< Landsat ratio (for resampling) */

static double lon_center; /**< Center longitude (projection center) */
static double a; /**< Semi-major axis */
static double b; /**< Semi-minor axis */
static double a2; /**< SOM series coefficient */
static double a4; /**< SOM series coefficient */
static double c1; /**< SOM series coefficient */
static double c3; /**< SOM series coefficient */
static double q;  /**< For SOM series calculation */
static double t;  /**< For SOM series calculation */
static double u;  /**< For SOM series calculation */
static double w;  /**< For SOM series calculation */
static double xj; /**< For SOM series calculation */
static double p21; /**< For SOM series calculation */
static double sa; /**< Sine of inclination angle. */
static double ca; /**< Cosine of inclination angle. */
static double es; /**< Eccentricity squared */
static double s;  /**< For SOM series calculation */
static double start; /**< Where SOM starts, beginning or end. */
static double false_easting; /**< X offset in meters */
static double false_northing; /**< Y offset in meters */
static void som_series(double *fb, double *fa2, double *fa4, double *fc1,
        double *fc3,double *dlam);

/**
 * Initialize the Space Oblique Mercator projection for inverse transformation.
 *
 * @param r_major Major axis radius.
 * @param r_minor Minor axis radius.
 * @param satnum Landsat satellite number (1,2,3,4,5).
 * @param path Landsat path number.
 * @param alf_in Inclination of orbit.
 * @param lon Center longitude.
 * @param false_east X offset in meters.
 * @param false_north Y offset in meters.
 * @param time Time.
 * @param start1 Start time.
 * @param flag Initialization method (A or B)
 * 
 * @return Always returns 0
 *
 * @author D. Steinwand @date July, 1992
 */
long sominvint(double  r_major, double  r_minor, long satnum, long path,
        double alf_in, double lon, double false_east, double false_north,
        double time, long start1, long flag) {
//long sominvint(r_major,r_minor,satnum,path,alf_in,lon,false_east,false_north,
//	time, start1,flag)
//
//double	r_major;		/* major axis				*/
//double	r_minor;		/* minor axis				*/
//long satnum;			/* Landsat satellite number (1,2,3,4,5) */
//long path;			/* Landsat path number */
//double alf_in;
//double lon;
//double false_east;		/* x offset in meters			*/
//double false_north;		/* y offset in meters			*/
//double time;
//long start1;
//long flag;
//{
long i;
double alf,e2c,e2s,one_es;
double dlam,fb,fa2,fa4,fc1,fc3,suma2,suma4,sumc1,sumc3,sumb;

/* Place parameters in static storage for common use
  -------------------------------------------------*/
false_easting = false_east;
false_northing = false_north;
a = r_major;
b = r_minor;
es = 1.0 - SQUARE(r_minor/r_major);

if (flag != 0)
  {
  alf = alf_in;
  lon_center = lon;
  p21 = time/1440.0;
  start = start1;
  }
else
  {
  if (satnum < 4)
    {
    alf = 99.092 * D2R;
    p21=103.2669323/1440.0;
    lon_center = (128.87 - (360.0/251.0 * path)) * D2R;
    }
  else
    {
    alf = 98.2 * D2R;
    p21=98.8841202/1440.0;
    lon_center = (129.30 - (360.0/233.0 * path)) * D2R;
    /*
    lon_center = (129.30557714 - (360.0/233.0 * path)) * D2R;
    */
    }
  start=0.0;
  }

/* Report parameters to the user (to device set up prior to this call)
  -------------------------------------------------------------------*/
ptitle("SPACE OBLIQUE MERCATOR");
radius2(a,b);
genrpt_long(path,      "Path Number:    ");
genrpt_long(satnum,    "Satellite Number:    ");
genrpt(alf*R2D,        "Inclination of Orbit:    ");
genrpt(lon_center*R2D, "Longitude of Ascending Orbit:    ");
offsetp(false_easting,false_northing);
genrpt(LANDSAT_RATIO,  "Landsat Ratio:    ");

ca=cos(alf);
if (fabs(ca)<1.e-9) ca=1.e-9;
sa=sin(alf);
e2c=es*ca*ca;
e2s=es*sa*sa;
w=(1.0-e2c)/(1.0-es);
w=w*w-1.0;
one_es=1.0-es;
q = e2s / one_es;
t = (e2s*(2.0-es)) / (one_es*one_es);
u= e2c / one_es;
xj = one_es*one_es*one_es;
dlam=0.0;
som_series(&fb,&fa2,&fa4,&fc1,&fc3,&dlam);
suma2=fa2;
suma4=fa4;
sumb=fb;
sumc1=fc1;
sumc3=fc3;
for(i=9;i<=81;i+=18)
   {
   dlam=i;
   som_series(&fb,&fa2,&fa4,&fc1,&fc3,&dlam);
   suma2=suma2+4.0*fa2;
   suma4=suma4+4.0*fa4;
   sumb=sumb+4.0*fb;
   sumc1=sumc1+4.0*fc1;
   sumc3=sumc3+4.0*fc3;
   }
for(i=18; i<=72; i+=18)
   {
   dlam=i;
   som_series(&fb,&fa2,&fa4,&fc1,&fc3,&dlam);
   suma2=suma2+2.0*fa2;
   suma4=suma4+2.0*fa4;
   sumb=sumb+2.0*fb;
   sumc1=sumc1+2.0*fc1;
   sumc3=sumc3+2.0*fc3;
   }

dlam=90.0;
som_series(&fb,&fa2,&fa4,&fc1,&fc3,&dlam);
suma2=suma2+fa2;
suma4=suma4+fa4;
sumb=sumb+fb;
sumc1=sumc1+fc1;
sumc3=sumc3+fc3;
a2=suma2/30.0;
a4=suma4/60.0;
b=sumb/30.0;
c1=sumc1/15.0;
c3=sumc3/45.0;
return(OK);
}

/**
 * Space Oblique Mercator inverse equations--mapping x,y to lat,long
 * 
 * @param x X projection coordinate.
 * @param y Y projection coordinate.
 * @param lon Pointer to store longitude.
 * @param lat Pointer to store latitude.
 *
 * @return 
 * - 0 :: Success
 * - 214 :: Convergence failure
 *
 * @author D. Steinwand @date July, 1992
 */
long sominv(double x, double y, double *lon, double *lat) {
//long sominv(y, x, lon, lat)
// 
//double x;               /* (I) X projection coordinate */
//double y;               /* (I) Y projection coordinate */
//double *lon;            /* (O) Longitude */
//double *lat;            /* (O) Latitude */
//{
double tlon,conv,sav,sd,sdsq,blon,dif,st,defac,actan,tlat,dd,bigk,bigk2,xlamt;
double sl,scl,dlat,dlon,temp;
long inumb;
 
/* Inverse equations. Begin inverse computation with approximation for tlon. 
   Solve for transformed long.
  ---------------------------*/
temp=y; y=x - false_easting; x= temp - false_northing;
tlon= x/(a*b);
conv=1.e-9;
for(inumb=0;inumb<50;inumb++)
   {
   sav=tlon;
   sd=sin(tlon);
   sdsq=sd*sd;
   s=p21*sa*cos(tlon)*sqrt((1.0+t*sdsq)/((1.0+w*sdsq)*(1.0+q*sdsq)));
   blon=(x/a)+(y/a)*s/xj-a2*sin(2.0*tlon)-a4*sin(4.0*tlon)-(s/xj)*(c1*
          sin(tlon)+c3*sin(3.0*tlon)); 
   tlon=blon/b;
   dif=tlon-sav;
   if(fabs(dif)<conv)break; 
   }
if(inumb>=50)  
   {
   p_error("50 iterations without convergence","som-inverse");
   return(214);
   }

/* Compute transformed lat.
  ------------------------*/
st=sin(tlon);
defac=exp(sqrt(1.0+s*s/xj/xj)*(y/a-c1*st-c3*sin(3.0*tlon)));
actan=atan(defac);
tlat=2.0*(actan-(PI/4.0));

/* Compute geodetic longitude
  --------------------------*/
dd=st*st;
if(fabs(cos(tlon))<1.e-7) tlon=tlon-1.e-7;
bigk=sin(tlat); 
bigk2=bigk*bigk;
xlamt=atan(((1.0-bigk2/(1.0-es))*tan(tlon)*ca-bigk*sa*sqrt((1.0+q*dd)
            *(1.0-bigk2)-bigk2*u)/cos(tlon))/(1.0-bigk2*(1.0+u)));

/* Correct inverse quadrant
  ------------------------*/
if(xlamt>=0.0) sl=1.0;
if(xlamt<0.0) sl= -1.0;
if(cos(tlon)>=0.0) scl=1.0;
if(cos(tlon)<0.0) scl= -1.0;
xlamt=xlamt-((PI/2.0)*(1.0-scl)*sl);
dlon=xlamt-p21*tlon;

/* Compute geodetic latitude
  -------------------------*/
if(fabs(sa)<1.e-7)
  dlat=asin(bigk/sqrt((1.0-es)*(1.0-es)+es*bigk2));
else
  dlat=atan((tan(tlon)*cos(xlamt)-ca*sin(xlamt))/((1.0-es)*sa));
*lon = adjust_lon(dlon+lon_center);
*lat = dlat;
return(OK);
}
 
/**
 * Series to calculate a,b,c coefficients to convert from transform
 * latitude, longitude to Space Oblique Mercator (SOM) rectangular coordinates.
 *
 * Mathematical analysis by John Snyder 6/82
 * 
 * @param fb Pointer to store coefficient fb.
 * @param fa2 Pointer to store coefficient fa2.
 * @param fa4 Pointer to store coefficient fa4.
 * @param fc1 Pointer to store coefficient fc1.
 * @param fc3 Pointer to store coefficient fc3.
 * @param dlam Pointer to store dlam (updated to radians).
 *
 * @author D. Steinwand @date July, 1992
 */
static void som_series(double *fb, double *fa2, double *fa4, double *fc1,
        double *fc3,double *dlam) {
// static void som_series(fb,fa2,fa4,fc1,fc3,dlam)
// double *fb,*fa2,*fa4,*fc1,*fc3,*dlam;
// {
double sd,sdsq,h,sq,fc;

*dlam= *dlam*0.0174532925;		/* Convert dlam to radians */
sd=sin(*dlam); 
sdsq=sd*sd;
s=p21*sa*cos(*dlam)*sqrt((1.0+t*sdsq)/((1.0+w*sdsq)*(1.0+q*sdsq)));
h=sqrt((1.0+q*sdsq)/(1.0+w*sdsq))*(((1.0+w*sdsq)/((1.0+q*sdsq)*(1.0+
     q*sdsq)))-p21*ca);
sq=sqrt(xj*xj+s*s);
*fb=(h*xj-s*s)/sq;
*fa2= *fb*cos(2.0* *dlam);
*fa4= *fb*cos(4.0* *dlam);
fc=s*(h+xj)/sq;
*fc1=fc*cos(*dlam);
*fc3=fc*cos(3.0* *dlam);
}
