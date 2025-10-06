/** @file
 * @brief NetCDF support.
 * @author Sergey Varlamov, Kristian Nilssen, Wesley Ebisuzaki 
 * @date 2/18/2008
 */

/*

vsm: test compilation with undefined USE_NETCDF...

 This file is part of wgrib2 and is distributed under terms of the GNU General Public License
 For details see, Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 Boston, MA  02110-1301  USA

 Edition 2008.02.18

 Sergey Varlamov
 Kristian Nilssen
 Wesley Ebisuzaki
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
//#include "grb2.h"
#include "wgrib2.h"
#include "fnlist.h"
#include "wgrib2nc.h"

//#define DEBUG_NC

#if defined USE_NETCDF

/**
 * Create time string (UTC) from time
 *
 * @param utime Unix time in seconds
 * @param date_str Pointer to a string buffer to hold the formatted date
 * 
 * @return Pointer to the date string
 *
 * @author Sergey Varlamov @date 2/18/2008
 */
char * get_unixdate(double utime, char * date_str)
{
  struct tm *gmt;
  time_t gmt_t;

  gmt_t=(time_t)utime;
  gmt = gmtime(&gmt_t);
  sprintf(date_str,"%.4d.%.2d.%.2d %.2d:%.2d:%.2d UTC",
  gmt->tm_year+1900,gmt->tm_mon+1,gmt->tm_mday,gmt->tm_hour,gmt->tm_min,gmt->tm_sec);
  return date_str;
}


/**
 * Check if strings match.
 *
 * @param s Pointer to the input string
 * @param match Pointer to the string to match against
 * 
 * @return 1 if the strings match, 0 otherwise
 * 
 * @author Sergey Varlamov @date 2/18/2008
 */
int match_str(const char *s, const char *match) {
   while (*match) {
      if (*match++ != *s++) return 0;
   }
   return 1;
}
 
/**
 * Makes units COARDS compliant: C -> Celsius, g -> gram, prob -> 1, gpm -> m
 * 
 * @param s Pointer to the units string
 * @param n Length of the units string
 *
 * @author Sergey Varlamov @date 2/18/2008
 */
void fix_units(char *s, int n)
{
// seems to be risky incrementing the pointer s...
// modify to more safe having time. vsm
  char tmp[n+2], *p;

  tmp[0] = '+';
  strncpy(tmp+1, s, n);
  p = tmp+1;
  *s = 0;

  while (*p)
  {
    if (toupper((unsigned char) *p) == 'C' && !isalpha((unsigned char) p[1]) && !isalpha((unsigned char) p[-1]))
    {
      *s++ = 'C';
      *s++ = 'e';
      *s++ = 'l';
      *s++ = 's';
      *s++ = 'i';
      *s++ = 'u';
      *s++ = 's';
      p++;
    }
    else if (toupper((unsigned char) *p) == 'G' && !isalpha((unsigned char) p[1]) && !isalpha( (unsigned char) p[-1]))
    {
      *s++ = 'g';
      *s++ = 'r';
      *s++ = 'a';
      *s++ = 'm';
      p++;
    }
    else if (toupper((unsigned char) *p) == '%') 
    {
      *s++ = 'p';
      *s++ = 'e';
      *s++ = 'r';
      *s++ = 'c';
      *s++ = 'e';
      *s++ = 'n';
      *s++ = 't';
      p++;
    }
    else if (toupper((unsigned char) *p) == 'P' && toupper((unsigned char) p[1]) == 'R' && toupper((unsigned char) p[2]) == 'O' && toupper((unsigned char) p[3]) == 'B')
    {
      *s++ = '1';
      p += 4;
    }
    else if (toupper((unsigned char) *p) == 'G' && toupper((unsigned char) p[1]) == 'P' && toupper((unsigned char) p[2]) == 'M' && !isalpha((unsigned char) p[-1]) && !isalpha((unsigned char) p[3]) )
    {
      *s++ = 'm';
      p += 3;
    }
    else
    {
      *s++ = *p++;
    }
  }
  *s = 0;
}

/**
 * Get the netCDF conversion table index for a given variable name and level.
 *
 * @param name Pointer to the variable name string
 * @param level Pointer to the variable level string
 * @param nc_table Pointer to the netCDF conversion table
 *
 * @return The index of the variable in the netCDF conversion table, or -1 if not found
 *
 * @author Sergey Varlamov @date 2/18/2008
 */
int get_nc_conv_table(const char * name, const char * level,
                      const g2nc_table * nc_table)
{
  int i;
  if ( nc_table )
    if ( nc_table->nvc )
      for (i=0; i < nc_table->nvc; i++)
        if ( strcmp(nc_table->vc[i].wgrib2_name, name) == 0 &&
             strcmp(nc_table->vc[i].wgrib2_level, level) == 0 ) return i;
  return -1;
}

/**
 * Free the netCDF conversion table.
 *
 * @param nc_table Pointer to the netCDF conversion table
 *
 * @return 0 on success, error code otherwise
 *
 * @author Sergey Varlamov @date 2/18/2008
 */
int free_nc_table( g2nc_table * nc_table )
{
  // cleanup
  int i;
  if ( nc_table == NULL ) return 0;
  if ( nc_table->nvc > 0 && nc_table->vc )
  {
    for (i=0; i < nc_table->nvc; i++)
    {
      free(nc_table->vc[i].wgrib2_name);
      free(nc_table->vc[i].wgrib2_level);
      free(nc_table->vc[i].nc_name);
    }
  }
  free(nc_table->vc);
  free(nc_table->lt);
  free(nc_table->lv);
  free(nc_table);
  nc_table = NULL;
#ifdef DEBUG_NC
fprintf(stderr,"nc_table: cleaned-up...\n");
#endif
  return 0;
}

/** 4D level types for netCDF */
g2nc_4Dlt nc_4Dlt[G2NC_NUM_4DLT] = {
  { 20,"klevel","K level","K",1.,},             //"%g K level",
  {100,"plevel","pressure level","mb",0.01,},   //"%g hPa",
  {104,"slevel","sigma level","level",1.,},     //"%g sigma level",
  {105,"hlevel","hybrid level","level",1.,},    //"%g hybrid level",
  {107,"kilevel","K isentropic level","K",1.,}, //"%g K isentropic level",
  {160,"depth","ocean depth","m",1.,},          //"%g m below sea level",
};

/** NetCDF conversion table */
g2nc_table * nc_table = NULL; /* table undefined */

/*
 * HEADER:100:nc_grads:setup:0:require netcdf file to be grads v1.9b4 compatible (fixed time step only)
 */

/** NetCDF compatibility flag. */
int nc_grads_compatible = 0;

/**
 * Activates some tests for the created netcdf file to be GrADS (version 1.9b4) compatible, 
 * or to be open by sdfopen in gradsnc or gradsdods. The GrADS support only COARDS convention 
 * netcdf files; it do not support non-constant data time stepping and silently generates 
 * wrong time stamps for such netcdf files. Packing to byte also is not directly recognized 
 * by GrADS v1.9b4. With this option an error is raised and processing stops if any of 
 * criterias above is determined. 
 * 
 * ## Usage
 * -nc_grads
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return Always returns 0
 * 
 * @author Sergey Varlamov @date 2/18/2008
 */
int f_nc_grads(ARG0)
{
  if (mode == -1) nc_grads_compatible = 1;
  return 0;
}

/*
 * HEADER:100:no_nc_grads:setup:0:netcdf file may be not grads v1.9b4 compatible, variable time step
 */
/**
 * NetCDF file may not be grads v1.9b4 compatible, variable time step.
 *
 * Opposite of -nc_grads option.
 *
 * ## Usage
 * -no_nc_grads
 *
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 *
 * @return Always returns 0
 * 
 * @author Sergey Varlamov @date 2/18/2008
 */
int f_no_nc_grads(ARG0)
{
  if (mode == -1) nc_grads_compatible = 0;
  return 0;
}

/*
 * HEADER:100:nc_pack:setup:1:pack/check limits of all NEW input variables, X=min:max[:byte|short|float]
 *
 * NEW means that if some variable was already defined in the netcdf file and now is appended to it (-append mode)
 * initially defined packing parameters are used. min and max are used to estimate appropriate offset and scaling,
 * if both are zero - automatic scaling goes.
 */

/** NetCDF packing flag. */
int nc_pack = 0;

/** NetCDF packing offset. */
float nc_pack_offset = 0.;

/** NetCDF packing scale. */
float nc_pack_scale = 1.;

/** NetCDF valid range minimum. */
float nc_valid_min = 0.;    

/** NetCDF valid range maximum. */
float nc_valid_max = 0.;

/**
 * Pack/check limits of all NEW input variables. 
 * 
 * NEW means that if some variable was already defined in the netcdf file and now is appended 
 * to it (-append mode) initially defined packing parameters are used. min and max are used 
 * to estimate appropriate offset and scaling, if both are zero - automatic scaling goes.
 * 
 * The -nc_pack option makes it possible to limit the range of defined values and pack data 
 * in the netcdf file specifying data range and packing type as min:max[:float|byte|short]. 
 * Type float here is default and do not assume data packing, only data range is checked. 
 * Data outside of specified range are replaced by missing value code for all types of packing. 
 * This replacement is done in the local copy of unpacked data, so main data array stays unchanged. 
 * Values of min and max could be any signed float. Packing is applied to all new variables in 
 * the input stream. If some variable was already defined in the netcdf file and now is appended 
 * to it in the -append mode the initially defined and fixed in the netcdf file valid range and 
 * packing parameters are used. 
 * 
 * The packing is possible to the short (2 bytes) or byte (1 byte) values with potential loss of 
 * precision. Both zero values of min and max packing parameters for the short or byte packing 
 * activate 'auto' packing when min and max values are defined from the first entered field. 
 * 
 * PS: packing in 'byte' is not directly accepted by GrADS v1.9b4 if netcdf file is open with 
 * 'sdfopen' command. But by some reasons using the GrADS data description file (catalog) for 
 * the same netcdf file helps in this situation. 
 * 
 * ## Usage
 * -nc_pack X
 *
 * X = min:max[:byte|short|float]
 * 
 * @param ARG1 ???
 * 
 * @return 0 for success, error code otherwise
 *
 * @author Sergey Varlamov @date 2/18/2008
 */
int f_nc_pack(ARG1)
{
  char * pack_to=NULL;
  float range;
  int i;
  if (mode == -1)
  {
//fprintf(stderr,"nc_pack: get arg: %s\n", arg1);
    pack_to = (char*) malloc((strlen(arg1)+12)*sizeof(char));
    if (pack_to == NULL) fatal_error("nc_pack: error allocating tmp string","");
    i = sscanf(arg1,"%g:%g:%s", &nc_valid_min, &nc_valid_max, pack_to);
    if ( i < 2)
    {
      fatal_error("nc_pack: bad value, expect min:max[:byte|short|float(default)], found %s", arg1);
//    if (sscanf(arg1,"%g:%g:%s", &nc_pack_offset, &nc_pack_scale, pack_to) != 3){
//      fatal_error("netcdf: bad nc_pack, expect offset:scale:[byte|short], found %s", arg1);
    }
    range = (nc_valid_max - nc_valid_min);
    if ( i == 2 ) strcpy(pack_to,"float");

    if (strcmp(pack_to,"byte")==0 || strcmp(pack_to,"BYTE")==0 || strcmp(pack_to,"Byte")==0)
      nc_pack = G2NC_PACK_BYTE;
    else if (strcmp(pack_to,"short")==0 || strcmp(pack_to,"SHORT")==0 || strcmp(pack_to,"Short")==0)
      nc_pack = G2NC_PACK_SHORT;
    else if (strcmp(pack_to,"float")==0 || strcmp(pack_to,"FLOAT")==0 || strcmp(pack_to,"Float")==0)
      nc_pack = G2NC_PACK_FLOAT; /* will check valid_range */
    else
      fatal_error("nc_pack: bad value, expect min:max:byte|short|float, found %s", arg1);
    if ( nc_pack == G2NC_PACK_FLOAT && fabs(range) < 1e-20 ) fatal_error(
      "nc_pack: small valid_range specified, expected min:max[:float], min<max, found %s", arg1);

    /* Center near 0 as signed char or short are used. */
    if (nc_pack != G2NC_PACK_FLOAT)
    {
      nc_pack_offset = (float) ((nc_valid_max+nc_valid_min)*0.5);
      if ( nc_pack == G2NC_PACK_BYTE )
        nc_pack_scale = (float) ((range/(G2NC_FILL_VALUE_BYTE - 2))*0.5);
      else if ( nc_pack == G2NC_PACK_SHORT )
        nc_pack_scale = (float) ((range/(G2NC_FILL_VALUE_SHORT - 2))*0.5);
    }
    free(pack_to);

//fprintf(stderr,"nc_pack: nc_pack=%d\n", nc_pack);

  }
  return 0;
}

/*
 * HEADER:100:no_nc_pack:setup:0:no packing in netcdf for NEW variables
 *
 */

/**
 * No packing in netcdf for NEW variables.
 * 
 * ## Usage
 * -no_nc_pack
 *
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 *
 * @author Sergey Varlamov @date 2/18/2008
 */
int f_no_nc_pack(ARG0)
{
  if (mode == -1)
  {
    nc_pack = 0;
    nc_pack_scale = 1.;
    nc_pack_offset = 0.;
    nc_valid_min = 0.;
    nc_valid_max = 0.;
  }
  return 0;
}

/*
 * HEADER:100:nc_nlev:setup:1:netcdf, X = max LEV dimension for {TIME,LEV,LAT,LON} data
 */

/** NetCDF maximum LEV dimension. */
int nc_nlev = 0;

/**
 * Set the maximum LEV dimension for {TIME,LEV,LAT,LON} data in netCDF files.
 * 
 * This option is needed to export data to netcdf as 4D data defined in {TIME,LEV,LAT,LON} 
 * space. It must be followed by integer max_number_of_vertical_levels which defines the 
 * vertical dimension size of 4D data exported to the netcdf file. This value can not be found 
 * from each single grib2 message or sub-message, usually it has to be scanned the entire grib2 
 * file or even number of files. It is the reason why the max_number_of_vertical_levels value 
 * must be provided by user. 
 * 
 * Grib2 types of vertical levels eligible for export to the netcdf as 4D data are defined in 
 * the wgrib2 internal table and now include next types of GRIB2 levels: 
 * 
 * 20	:	K level
 * 100	:	pressure level
 * 104	:	sigma level
 * 105	:	hybrid level
 * 107	:	K isentropic level
 * 160	:	ocean depth below sea level
 * 
 * Data for the first found eligible level type are treated as 4D and vertical level information 
 * is not added to the variable names. Error is generetad if data for other then first found 
 * eligible level type are met in the input stream. Data on the non-eligible levels are treated 
 * as 3D data defined in the {TIME,LAT,LON} space with the possible level information included 
 * into the variable names or with the user-specified names (see -nc_table option description 
 * below for how to do it). 
 * 
 * The -nc_nlev option has not -no_... option, please use instead the max_number_of_vertical_levels 
 * equal to 0. In this case all data from the input stream will be threated as 3D. 
 * 
 * When existing netcdf file is updated in the -append mode the value of max_number_of_vertical_levels 
 * must not exceed initial value provided when the netcdf file was first created (defined). By 
 * default when first creating the netcdf file, vertical level values are not fixed (are undefined) 
 * and these are defined one-by-one when data at new level are added to the netcdf file, up to the 
 * max_number_of_vertical_levels. To overcome this feature please use the -nc_table option. 
 * 
 * ## Usage
 * 
 * -nc_lev LEVEL
 * 
 * @param ARG1 ???
 * 
 * @return 0 for success, error code otherwise
 *
 * @author Sergey Varlamov @date 2/18/2008
 */
int f_nc_nlev(ARG1)
{
  if (mode == -1)
  {
    if (sscanf(arg1,"%d", &nc_nlev) != 1)
    {
      fatal_error("netcdf: bad nc_nlev %s", arg1);
    }
    if (nc_nlev > G2NC_MAX_NLEV)
    {
      fatal_error("nc_nlev: value exceeds G2NC_MAX_NLEV, %s", arg1);
    }
  }
  return 0;
}

/*
 * HEADER:100:nc_table:setup:1:X is conversion_to_netcdf_table file name
 */

/**
 * Set conversion_to_netcdf_table file name.
 * 
 * The -nc_table option followed by the file_name is most usefull for the advanced users as it 
 * specifies the file where user can customize many features of created netcdf file. Next 
 * examples demonstrate which directives and conversion rules could be given in the -nc_table 
 * file. Almost all of them are optional with exception of $nlev for the case when user 
 * explicitly specifies the vertical level values $levs. 
 * 
 * @code{.sh}
 * $lev_type 100:pressure:pressure level:mb:0.01
 * @endcode
 * 
 * This directive explicitly defines the type of vertical level in the grib2 file that becomes 
 * eligible for treating data at these levels as 4D data and some it attributes for netcdf file. 
 * Default, first found level type listed in the wgrib2 build-in table of eligible levels is 
 * selected if no $lev_type directive found. 
 * 
 * Fields here include: 
 * 100	-	the grib2 level type code number for treating data as 4D. In this example it is the 
 * code number for pressure levels, integer
 * pressure	-	short_name for the vertical axis in the netcdf file, string
 * pressure level	-	long_name for the vertical axis in the netcdf file, string, could include 
 * spaces
 * mb	-	vertical axis units as written in the netcdf file, string
 * 0.01	-	decimal scale to convert grib2 level values to the netcdf-stored values, float. In 
 * this example we convert Pa to mb (hPa). It is default conversion scale for the pressure levels 
 * when exporting to the netcdf file
 * 
 * @code{.sh}
 * $nlev X
 * @endcode
 * 
 * This directive is an equivalent of the -nc_nlev command line option but value in the -nc_table 
 * file has precedence over the command line option if both are found. It is required if next 
 * $levs directive is specified. 
 * 
 * @code{.sh}
 * $levs lev(1) lev(2) lev(3)...
 * ...lev(I-1) lev(I) lev(I+1)...
 * ...lev(X-1) lev(X)
 * @endcode
 * 
 * This directive explicitly specifies the vertical level values to be exported to the netcdf 
 * file, in the netcdf units. If this directive exists in the -nc_table file and there is 
 * found the grib2 data eligible for the 4D presentation but defined on other then listed 
 * level - these data are skipped from export and work is silently continued. All data conversion 
 * parameters are checked before the $levs is checked. List of level values could consist of 
 * multiple lines; max line length in the -nc_table file is limited by _MAX_PATH symbols (about 
 * 255 symbols); use space, ',', ';' or ':' as lev(i) fields separator. 
 * 
 * Impact of $nlev and $levs directives is different depending on does the netcdf file is first 
 * created or it is updated. When the netcdf file is first created these values are written to 
 * the netcdf file fixing 4D data vertical structure. When updating an existing netcdf file in 
 * the -append mode these parameters will work like filter limiting possible updates of 4D data 
 * by these that satisfy to the given $levs only and ignoring other. 3D (TIME,LAT,LON) data are 
 * not affected by these directives. 
 * 
 * The $nlev and $levs directives could be necessary if records in the grib2 file have randomly 
 * changing vertical level values that makes it impossible sequentionally define valid order of 
 * vertical levels. 
 * 
 * @code{.sh}
 * $grads 1
 * @endcode
 * 
 * This directive is an equivalent of the -nc_grads command line option if passed value is 1 or 
 * to the -no_nc_grads if value is 0. Directive in the -nc_table file has precedence over the 
 * command line option. 
 * 
 * @code{.sh}
 * wgib2_name:wgrib2_level|*:nc_name|ignore[:ignore|no|float|short|byte[:min:max]]
 * @endcode
 * 
 * All non-empty lines in the -nc_table file not starting from the '$' symbol or from the comment 
 * mark '#' are treated as grib2 to netcdf conversion rules for the specified variables. In 
 * these strings the wgib2_name and wgrib2_level are strings as returned by the wgrib2 inventory; 
 * * used as the wgrib2_level will apply to all levels not given explicitly. The valid range and 
 * packing information is optional but it overwrites common packing rule if such is specified 
 * by the -nc_pack command line option. Absence of packing information means no packing for 
 * this variable. Min and max values are any signed float values. Both could be omitted or put 
 * to zero. Last case means that automatic scaling will be estimated from the first entered 
 * wgib2_name field at the wgrib2_level or at the first level in case of * as level value. 
 * 
 * If the keyword ignore is found as a netcdf variable name or as a packing type value, the 
 * corresponding data are ignored and do not written to the netcdf file. Impact from this keyword 
 * is similar to the wgrib2 -not option or filtering data with the grep utility. 
 * 
 * The ignore keyword is recommended if the data from the same grib2 file are exported in number 
 * of output files (netcdf or other) by the same wgrib2 process. Then the same decoded data could 
 * be passed for the output in other file of any supported type. Doing export to the single netcdf 
 * file it is not recommended to use the ignore keyword as corresponding data are first decoded 
 * and after that skipped from writing to the netcdf file. 
 * 
 * In the next example two lines from the -nc_table file instruct the wgrib2 utility export to 
 * the netcdf file the geopotential height changing name from HGT (wgrib2) to geopotential in 
 * the netcdf file, at all levels except at 975 mb if these data would be found in the grib2 
 * decoded input data stream: 
 * 
 * HGT:*:geopotential
 * HGT:975 mb:ignore
 * 
 * All other variables not listed in the -nc_table file but found in the input stream are 
 * processed as regular 3D data. 
 * 
 * ## Usage
 * -nc_table <FILE_NAME
 * 
 * @param ARG1 ???
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Sergey Varlamov @date 2/18/2008
 */
int f_nc_table(ARG1)
{
  char   input[_MAX_PATH], on[_MAX_PATH], lv[_MAX_PATH],
         nn[_MAX_PATH], pk[_MAX_PATH], defl[_MAX_PATH], ctmp[_MAX_PATH];
  char * prd;
  FILE * fl;
  int i, ir, ierr, itmp;
  g2nc_conv * old_nct;
  char *tptr;
  float min, max, range, ftmp;
  int search_lev;

  if (mode == -1)
  {
    /* avoid 'lost memory' */
    if ( nc_table )
      if ( !nc_table->used)
        fatal_error("nc_table: second table name entered when first was not used: %s", arg1);

    fl = fopen(arg1,"r");
    if(fl == NULL)
      fatal_error("nc_table: can not open file for reading: %s", arg1);

    /* Deconnect from allocated memory, it is mapped in some 'local' structure,
     * and allocate new instance
     */
    nc_table = (g2nc_table*)malloc(sizeof(g2nc_table));
    if ( nc_table == NULL )
      fatal_error("nc_table: error allocating new table: %s", arg1);

    nc_table->vc = NULL;
    nc_table->lt = NULL;
    nc_table->lv = NULL;
    nc_table->nvc = 0;
    nc_table->used = 0;
    nc_table->nlev = -1;
    nc_table->grads = -1;
    nc_table->endianness = -1;

    ierr = 0;

    while(!feof(fl))
    {/* read strings including \n symbol */
      prd = fgets(input, _MAX_PATH, fl);
      if(prd == NULL) continue;
      if(strlen(input) < 2)continue;
      /* format:

wgib2_name:wgrib2_level|*:nc_name|ignore[:ignore|no|float|deflate{0-9}|[short|byte:min:max]][:deflate{0-9}]

         or/and special instructions that overcome command-line options
         for the netcdf file they are applied:

           # 4D level type description: grib2_type:user_short_name:user:long_name:user_units:scale_to_user_units
           $lev_type 100:plevel:pressure level:mb:0.01   #JMA MSM model upper layer (p) data; scale: Pa->mb
           $nlev 3   #number of vertical levels for 4D variables
           $levs 1000 500 100  #vertical level values in user_units, at least nlev;
                 50 10
           $endianness native|little-endian|big-endian

         Length of input strings must be less then _MAX_PATH or 256 symbols,
         input on multiple lines is supported.
         The "deflate{0-9}" directive is supported only if netCDF-4 type new file is created;
         deflate0 means "no compression", other values define compression level. Default is 1.
       */
//
//    next do not work as expected, parse string 'manually', it's C...
//    ir = sscanf(input," %[^:]s:%[^:]s%[^:\n]s%[^:\n]s:%g:%g",
//                     on,    lv,    nn,      pk,    &min, &max);
      ir=0;
      min=max=0;
      prd = input;
      i = sscanf(prd," %[^:]s",on);
      if (i < 1) continue;
      if (on[0]=='#' || on[0]=='\n') continue;  /* comment or line of spaces only, pass */
      if (on[0]=='$')
      {
        /* specification of vertical level for 4D variables in grib2 to netcdf conversion */
        if (strncmp(on,"$nlev",5) == 0 || strncmp(nn,"$NLEV",5) == 0)
        {
          itmp = -1;
          i = sscanf(prd," %*s %d",&itmp);
          if ( i < 1 || itmp < 0 || itmp > G2NC_MAX_NLEV)
          {
            fprintf(stderr,"nc_table: large value or error in $nlev definition string:\n%s\n", prd);
            ierr = -1;
            break;
          }
          if (nc_table->nlev >= 0)
          {
            fprintf(stderr,"nc_table: found dublicate $nlev definition, was %d, new:\n%s\n",
            nc_table->nlev,prd);
            ierr = -2;
            break;
          }
          nc_table->nlev=itmp;
        }
        else if (strncmp(on,"$grads",6) == 0 || strncmp(nn,"$GRADS",6) == 0)
        {
          itmp = -1;
          i = sscanf(prd," %*s %d",&itmp);
          if ( i < 1 || itmp < 0 || itmp > 1)
          {
            fprintf(stderr,"nc_table: use 0 or 1 in $grads definition string:\n%s\n", prd);
            ierr = -1;
            break;
          }
          if (nc_table->grads >= 0)
          {
            fprintf(stderr,"nc_table: found dublicate $grads definition, was %d, new:\n%s\n",
            nc_table->grads,prd);
            ierr = -2;
            break;
          }
          nc_table->grads=itmp;
        }
        else if (strncmp(on,"$lev_type",9) == 0 || strncmp(nn,"$LEV_TYPE",9) == 0)
        {
          /* parse input string */
          i = sscanf(prd," %*s %d:",&itmp); /* type */
          if ( i >= 1 )
          {
            ir++;
            i = strcspn( prd, ":");
            if (i > 0 && i < strlen(prd) )
            {
              prd += (i+1);
              i = sscanf(prd," %[^: ]s",nn); /* sname, no spaces */
              if (i >= 1 )
              {
                ir++;
                i = strcspn( prd, ":");
                if (i > 0 && i < strlen(prd) )
                {
                  prd += (i+1);
                  i = sscanf(prd," %[^:]s",lv); /* lname */
                  if (i >= 1 )
                  {
                    ir++;
                    i = strcspn( prd, ":");
                    if (i > 0 && i < strlen(prd) )
                    {
                      prd += (i+1);
                      i = sscanf(prd," %[^:#\n]s",pk); /* units could be last */
                      if (i >= 1 )
                      {
                        ir++;
                        i = strcspn( prd, ":");
                        if (i > 0 && i < strlen(prd) )
                        {
                          prd += (i+1);
                          i = sscanf(prd," %g",&ftmp); /* scale */
                          if (i >= 1 )
                          {
                            ir++;
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
          if ( ir < 4 )
          {
            ierr = -3;
            fprintf(stderr,"nc_table: error %d in $lev_type definition string:\n%s\n", ir,on);
            break;
          }
          if ( nc_table->lt != NULL )
          {
            fprintf(stderr,"nc_table: found dublicate $lev_type definition:\n%s\n",on);
            ierr = -4;
            break;
          }
          nc_table->lt = (g2nc_4Dlt*)malloc(sizeof(g2nc_4Dlt));
          if ( ir > 4 )nc_table->lt->scale = ftmp;
          else         nc_table->lt->scale = 1;
          nc_table->lt->type = itmp;


          tptr = (char *) malloc((strlen(nn)+1)*sizeof(char));
      if (tptr) strcpy(tptr,nn);
          else ierr = 1;
          nc_table->lt->sname = tptr;

/* old version
          nc_table->lt->sname = (char *)malloc((strlen(nn)+1)*sizeof(char));
          if (nc_table->lt->sname) strcpy(nc_table->lt->sname,nn);
          else ierr = 1;
*/

          tptr = (char *) malloc((strlen(lv)+1)*sizeof(char));
          if (tptr) strcpy(tptr,lv);
          else ierr = 2;
          nc_table->lt->lname = lv;

/* old version
          nc_table->lt->lname = (char *)malloc((strlen(lv)+1)*sizeof(char));
          if (nc_table->lt->lname) strcpy(nc_table->lt->lname,lv);
          else ierr = 2;
*/

          tptr = (char *) malloc((strlen(pk)+1)*sizeof(char));
          if (tptr) strcpy(tptr,pk);
          else ierr = 3;
          nc_table->lt->units = tptr;

/* old version
          nc_table->lt->units = (char *)malloc((strlen(pk)+1)*sizeof(char));
          if (nc_table->lt->units) strcpy(nc_table->lt->units,pk);
          else ierr = 3;
 */

          if ( ierr ) break;
        }
        else if (strncmp(on,"$levs",5) == 0 || strncmp(nn,"$LEVS",5) == 0)
        {
          if ( nc_table->lv )
          {
            fprintf(stderr,"nc_table: found dublicate $levs definition:\n%s\n",on);
            ierr = -5;
            break;
          }
          if (nc_table->nlev > 0)
          {
            nc_table->lv = (float *)malloc(nc_table->nlev*sizeof(float));
            if ( nc_table->lv == NULL )
            {
              ierr = 4;
              break;
            }
          }
          else
          {
            fprintf(stderr,"nc_table: found $levs directive for undef $nlev:\n%s\n",prd);
            ierr = -6;
            break;
          }
          i = strcspn( prd, "$");
          prd += (i+4+1);

          for (i=0; i < nc_table->nlev; i++)
          {
            search_lev = 1;
            while (search_lev == 1) {
              // search_lev: ir = strcspn(prd,"+-.0123456789");
              ir = strcspn(prd,"+-.0123456789");
              itmp = strcspn(prd,"#"); /* inline comment */
              if ( ir < 0 || ir >= strlen(prd) || itmp < ir )
              { /* read next line */
                prd = fgets(input, _MAX_PATH, fl);
                if (prd == NULL )
                {
                  fprintf(stderr,"nc_table: error reading multy line $levs \n");
                  ierr = -7;
                  search_lev = 1;
                  continue;
                }
                continue;
              }
              itmp = strcspn(prd,":"); /* second important fields separator before number */
              if (itmp <= ir )
              {
                fprintf(stderr,"nc_table: error entering $levs, check that there are $nlev values defined\n");
                ierr = -8;
                search_lev = 1;
                continue;
              }
              prd += ir;
              ir = sscanf(prd,"%g",&nc_table->lv[i]);
              if ( ir < 1 )
              {
                fprintf(stderr,"nc_table: $levs formatted input error\n");
                ierr = -9;
                search_lev = 1;
                continue;
              }
              ir = strcspn(prd," ,;:\n");
              if(ir <= strlen(prd) ) prd += ir;
              else
              {
                fprintf(stderr,"nc_table: $levs parsing error, do no found fields separator\n");
                ierr = -10;
                search_lev = 1;
                continue;
              }
              search_lev = 0;
            }
            /* end of search lev */
            if (ierr) break;
          }
          if ( ierr ) break;
        }
        else if (strncmp(on,"$endianness",11) == 0 || strncmp(nn,"$ENDIANNESS",11) == 0)
        {
          *ctmp = 0;
          itmp = 0;
          i = sscanf(prd," %*s %[^ #\n]s",ctmp); /* could be last, no spaces */
          if ( i >= 1){
            if (strcmp(ctmp,"native")==0 || strcmp(ctmp,"NATIVE")==0 || strcmp(ctmp,"Native")==0)
            {
              nc_table->endianness = 0;
            }
            else if (strcmp(ctmp,"little-endian")==0 || strcmp(ctmp,"LITTLE-ENDIAN")==0)
            {
              nc_table->endianness = 1;
            }
            else if (strcmp(ctmp,"big-endian")==0 || strcmp(ctmp,"BIG-ENDIAN")==0)
            {
              nc_table->endianness = 2;
            }
            else
            {
              itmp = 1;
            }
          }
          if ( i < 1 || itmp == 1)
          {
            fprintf(stderr,"nc_table: use native, little-endian or big-endian strings in $endianness definition:\n%s\n", prd);
            ierr = -1;
            break;
          }
          /* dublicates do not checked, last found value would be used for endianness */
        }
        else
        {
          fprintf(stderr,"nc_table: found unrecognized directive:\n%s\n", prd);
          ierr = -11;
          break;
        }
        continue;
      }
/********************************************************************
      grib2 to netcdf variable convertion rule string parsing
********************************************************************/
      ir++;
      i = strcspn( prd, ":");
      if (i > 0 && i < strlen(prd) )
      {
        prd += (i+1);
        i = sscanf(prd," %[^:]s",lv);
        if (i >= 1 )
        {
          ir++;
          i = strcspn( prd, ":");
          if (i > 0 && i < strlen(prd) )
          {
            prd += (i+1);
            i = sscanf(prd," %[^: #\n]s",nn); /* could be last, no spaces */
            if (i >= 1 )
            {
              ir++;
              i = strcspn( prd, ":");
              if (i > 0 && i < strlen(prd) )
              {
                prd += (i+1);
                i = sscanf(prd," %[^: #\n]s",pk);  /* could be last, no spaces */
                if (i >= 1 )
                {
                  ir++;
                  i = strcspn( prd, ":");
                  if (i > 0 && i < strlen(prd) )
                  {
                    prd += (i+1);
                    i = strcspn( prd, ":");
                    if (i > 0 && i < strlen(prd) ) //one more separator found, min:max
                    {
//                      i = sscanf(prd," %g: %g",&offset, &scale);
                      i = sscanf(prd," %g: %g",&min, &max);
                      if (i >= 2 )
                      {
                        ir += i;
                        i = strcspn( prd, ":");
                        if (i > 0 && i < strlen(prd) )
                        {
                          prd += (i+1);
                          i = strcspn( prd, ":");
                          if (i > 0 && i < strlen(prd) )
                          {
                            prd += (i+1);
                            i = sscanf(prd," %[^: #\n]s",defl);  /* must be last, no spaces */
                            ir += i;
                          }
                        }
                      }
                    }
                    else /* no separators more, single last input string*/
                    {
                      i = sscanf(prd," %[^: #\n]s",defl);  /* could be last, no spaces */
                      ir += i;
                    }
                  }
                }
              }
            }
          }
        }
      }
      if ( ir < 3 )
      {
        /* not eligible string, issue warning and pass */
//        fprintf(stderr,"nc_table: badly formatted string, ignore: %s",input);
//        continue;
        fprintf(stderr,"nc_table: badly formatted string:\n %s",input);
        ierr = -21;
        break;
      }
      if ( nc_table->nvc > G2NC_MAX_VARS )
      {
        fprintf(stderr,"nc_table: nvc exceed G2NC_MAX_VARS: %d %d\n",
        nc_table->nvc, G2NC_MAX_VARS);
        ierr = -22;
        break;
      }
      i = nc_table->nvc;
      nc_table->nvc++;
      old_nct = nc_table->vc;

      nc_table->vc = (g2nc_conv*) realloc((void*)old_nct, nc_table->nvc*sizeof(g2nc_conv));
      nc_table->vc[i].ignore = 0;
      nc_table->vc[i].nc_pack = 0;
      nc_table->vc[i].nc_offset = 0.;
      nc_table->vc[i].nc_scale = 1.;
      nc_table->vc[i].nc_valid_min = 0.;
      nc_table->vc[i].nc_valid_max = 0.;
      nc_table->vc[i].nc_deflate = 1;
      nc_table->vc[i].wgrib2_name = (char *)malloc((strlen(on)+1)*sizeof(char));
      if (nc_table->vc[i].wgrib2_name)
        strcpy(nc_table->vc[i].wgrib2_name,on);
      else
      {
        ierr = 21;
        break;
      }
      nc_table->vc[i].wgrib2_level = (char *)malloc((strlen(lv)+1)*sizeof(char));
      if (nc_table->vc[i].wgrib2_level)
        strcpy(nc_table->vc[i].wgrib2_level,lv);
      else
      {
        ierr = 22;
        break;
      }
      nc_table->vc[i].nc_name = (char *)malloc((strlen(nn)+1)*sizeof(char));
      if (nc_table->vc[i].nc_name)
      {
        strcpy(nc_table->vc[i].nc_name,nn);
        if (strcmp(nn,"ignore")==0 || strcmp(nn,"IGNORE")==0 || strcmp(nn,"Ignore")==0)
          nc_table->vc[i].ignore = 1;
      }
      else
      {
        ierr = 23;
        break;
      }
      if (ir < 4)continue;

      range = (max - min);
      if (strcmp(pk,"byte")==0 || strcmp(pk,"BYTE")==0 || strcmp(pk,"Byte")==0)
      {
        nc_table->vc[i].nc_pack = G2NC_PACK_BYTE;
        nc_table->vc[i].nc_scale = (float) ((range/(G2NC_FILL_VALUE_BYTE - 2))*0.5);
      }
      else if (strcmp(pk,"short")==0 || strcmp(pk,"SHORT")==0 || strcmp(pk,"Short")==0)
      {
        nc_table->vc[i].nc_pack = G2NC_PACK_SHORT;
        nc_table->vc[i].nc_scale = (float) ((range/(G2NC_FILL_VALUE_SHORT - 2))*0.5);
      }
      else if (strcmp(pk,"float")==0 || strcmp(pk,"FLOAT")==0 || strcmp(pk,"Float")==0)
      {
        nc_table->vc[i].nc_pack = G2NC_PACK_FLOAT;
      }
      else if (strcmp(pk,"ignore")==0 || strcmp(pk,"IGNORE")==0 || strcmp(pk,"Ignore")==0)
      {
        nc_table->vc[i].ignore = 1;
        continue;
      }
      else if (strcmp(pk,"no") == 0 || strcmp(pk,"NO") == 0 )
      {
        continue;
      }
      else if (strncmp(pk,"deflate",7)==0 || strncmp(pk,"DEFLATE",7)==0 || strncmp(pk,"Deflate",7)==0)
      {
        if ( strlen(pk) > 7 && isdigit((unsigned char) pk[7]))
        {
          nc_table->vc[i].nc_deflate = (int)pk[7];
          continue;
        }
        else
        {
          fprintf(stderr,"nc_table: unsupported deflate definition, ignored: %s in %s",pk,input);
          continue;  /* not eligible string, warning and pass */
        }
      }
      else
      {
        fprintf(stderr,"nc_table: unsupported packing type, ignored: %s in %s",pk,input);
        continue;  /* not eligible string, warning and pass */
      }
      if (nc_table->vc[i].nc_pack == G2NC_PACK_FLOAT)
      {
        if (fabs(range) < 1e-20)
        {
          fprintf(stderr,"nc_table: small valid_range specified, expected ...:float:{min}:{max}, min<max: %s",input);
          ierr = 23;
          break;
        }
      }
      else  nc_table->vc[i].nc_offset = (float) ((min+max)*0.5);

      nc_table->vc[i].nc_valid_min=min;
      nc_table->vc[i].nc_valid_max=max;

      if (strncmp(defl,"deflate",7)==0 || strncmp(defl,"DEFLATE",7)==0 || strncmp(defl,"Deflate",7)==0)
      {
        if ( strlen(defl) > 7 && isdigit((unsigned char) defl[7]))
        {
          nc_table->vc[i].nc_deflate = (int)defl[7];
          continue;
        }
        else
        {
          fprintf(stderr,"nc_table: unsupported deflate definition, ignored: %s in %s",defl,input);
          continue;  /* not eligible string, warning and pass */
        }
      }
    }
    fclose(fl);
    if( ierr )
    {
      if ( ierr > 0 ) fprintf(stderr,"nc_table: allocation error\n");
      fatal_error("nc_table: fatal error parsing file %s", arg1);
    }

#ifdef DEBUG_NC
fprintf(stderr, "nc_table: total found: %d entries\n",nc_table->nvc);
for (i=0; i < nc_table->nvc; i++)
{
  fprintf(stderr, "%2d) %s:%s:%s:%d:%d:%f:%f:%f:%f:%d\n",i,
    nc_table->vc[i].wgrib2_name,
    nc_table->vc[i].wgrib2_level,
    nc_table->vc[i].nc_name,
    nc_table->vc[i].ignore,
    nc_table->vc[i].nc_pack,
    nc_table->vc[i].nc_valid_min,
    nc_table->vc[i].nc_valid_max,
    nc_table->vc[i].nc_offset,
    nc_table->vc[i].nc_scale,
    nc_table->vc[i].nc_deflate);
}
fprintf(stderr, "nc_table: $endianness=%d\n",nc_table->endianness);
if (nc_table->nlev >= 0)
{
  fprintf(stderr, "nc_table: $nlev=%d\n",nc_table->nlev);
  if (nc_table->lv)
    for (i=0; i < nc_table->nlev; i++)
      fprintf(stderr, "lev(%d)=%g\n",i,nc_table->lv[i]);
}
if( nc_table->lt )
{
  fprintf(stderr, "nc_table: found $lev_type directive:\n");
  fprintf(stderr, "$lev_type = %d:%s:%s:%s:%g\n",
    nc_table->lt->type,  nc_table->lt->sname,
    nc_table->lt->lname, nc_table->lt->units,
    nc_table->lt->scale);
}
#endif
  }
  return 0;
}

/*
 * HEADER:100:no_nc_table:setup:0:disable previously defined conversion_to_netcdf_table
 */

/**
 * Disable the previously defined conversion to netcdf table.
 * 
 * ## Usage
 * -no_nc_table
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Sergey Varlamov @date 2/18/2008
 */
int f_no_nc_table(ARG0)
{
  if (mode == -1)
  {
    if (nc_table)
    {
      if (nc_table->used)
      { /* disconnect from allocated memory, it is mapped in some 'local' structure */
       nc_table = NULL;
      }
      else fatal_error("no_nc_table: disabling not used -nc_table, is it your intention?","");
    }
  }
  return 0;
}
/*
 * HEADER:100:nc_time:setup:1:netcdf, [[-]yyyymmddhhnnss]:[dt{s[ec]|m[in]|h[our]|d[ay]}], [-] is for time alignment only
 */
//nc_time option [+|-]{yyyymmddhhnn}:{dt}[mn|hr|dy]

/** NetCDF date. 0 for undefined.*/
double nc_date0 = 0;      /* undefined value... */

/** NetCDF date type. 0 for undefined; 1 for absolute, -1 for relative (alignment only) */
int    nc_date0_type = 0; 

/** NetCDF time step. 0 for not initialized; -1 will be used for variable (undefined) step*/
double nc_dt = 0; 

/**
 * NetCDF time setup.
 * 
 * ## Usage
 * -nc_time [+|-]{yyyymmddhhnn}:{dt}[mn|hr|dy]
 *
 * [-] is for time alignment only
 * 
 * @param ARG1 ???
 * 
 * @return 0 for success, error code otherwise
 * 
 * @author Sergey Varlamov @date 2/18/2008
 */
int f_nc_time(ARG1)
{
  char chr;
  const char *p;
  int year,month,day,hour,minute,second,dt_val,dt_conv,err_code;
  char dt_type[25];
  int skip_date;

  if (mode == -1)
  {

    if (strlen(arg1) == 0) return 0;
    /* default initialization, clear any previously defined values */
    nc_date0_type = 0;
    nc_date0 = 0;
    nc_dt = 0;
    skip_date = 0;

    sscanf(arg1,"%c",&chr);

    p=arg1;
    if (chr==':')
    { /* separator symbol is first, time step only, any long string*/
      p++;
      skip_date = 1;
    }
    if (strlen(p) < 14)
    { /* time step only */
      skip_date = 1;
    }
    if (skip_date == 0) {
      /* include date */
      nc_date0_type = 1;    /*absolute */
      if (chr=='-')
      {
        p++;
        nc_date0_type = -1; /* relative*/
      }
      else if (chr == '+')
      {
        p++;
      }
      if (sscanf(p,"%4d%2d%2d%2d%2d%2d",&year,&month,&day,&hour,&minute,&second) != 6)
        fatal_error("nc_time: bad format of yyyymmddhhnnss value in nc_time: %s", arg1);

      if(year<0 || year>9999 || month<1 || month>12 || day<1 || day>31 ||
         hour <0 || hour>23 || minute < 0 || minute >59 || second < 0 || second >59)
         fatal_error("nc_time: bad value of yyyymmddhhnnss value in nc_time: %s", arg1);

#ifdef DEBUG_NC
printf("nc_time: date0_type=%d date0=%d.%d.%d %d:%d:%d\n",nc_date0_type,year,month,day,hour,minute,second);
#endif
      nc_date0 = get_unixtime(year, month, day, hour, minute, second, &err_code);
      if(err_code)
        fatal_error("nc_time: time [sec] is out of range for this OS","");

      p+=14;
      if (strlen(p) <= 1)
      {
        if (nc_date0_type > 0) return 0;
        fatal_error("nc_time: negative yyyymmddhhnnss (relative date) need time step in -nc_time option: %s", arg1);
      }
      p++; /*pass separator symbol ':' or other*/
    }
 
    /* now for the time step */ 
    if (sscanf(p,"%d%s",&dt_val,dt_type) != 2)
      fatal_error("nc_time: bad time step in nc_time: %s", p);

    if( strlen(dt_type) == 0)
      fatal_error("nc_time: undef time step type in nc_time: %s", arg1);

    p = dt_type;
    dt_conv=0;
    if (*p=='s' || *p=='S')
     dt_conv=1;
    else if (*p=='m' || *p=='M')
     dt_conv=60;
    else if (*p=='h' || *p=='H')
     dt_conv=3600;
    else if (*p=='d' || *p=='D')
     dt_conv=86400;
    else
      fatal_error("nc_time: unsupported time step type in nc_time: %s", p);

//    if (align_only && dt_conv > 86400*30) // unclear behavior for monthly data...
//      fatal_error("netcdf: unsupported large time increment in nc_time %s", arg1);
    nc_dt = (double)dt_val*(double)dt_conv;  // convert to seconds

#ifdef DEBUG_NC
printf("nc_time: time_step_value=%d, type=%s, val_sec=%.1lf\n",dt_val,dt_type,nc_dt);
#endif
  }
  return 0;
}

/*
 * HEADER:100:no_nc_time:setup:0:netcdf, disable previously defined initial or relative date and time step
 */

/**
 * Disable previously defined initial or relative date and time step.
 * 
 * ## Usage
 * -no_nc_time
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return Always returns 0
 * 
 * @author Sergey Varlamov @date 2/18/2008
 */
int f_no_nc_time(ARG0)
{
  if (mode == -1)
  {
    nc_date0 = 0;      /* undefined value... */
    nc_date0_type = 0; /* undefined; 1 for absolute, -1 for relative (alignment only) */
    nc_dt = 0;         /* not initialized; -1 will be used for variable (undefined) step */
  }
  return 0;
}
/*
 * HEADER:100:nc4:setup:0:use netcdf4 (compressed, controlled endianness etc)
 */

/** NetCDF version 4 flag */
int nc4 = 0;

/**
 * Use NetCDF version 4 (compressed, controlled endianness etc).
 * 
 * ## Usage
 * -nc4
 * 
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 * 
 * @return Always returns 0
 * 
 * @author Sergey Varlamov @date 2/18/2008
 */
int f_nc4(ARG0)
{
  if (mode == -1) nc4 = 1;
  return 0;
}

/*
 * HEADER:100:nc3:setup:0:use netcdf3 (classic)
 */

/**
 * Use NetCDF version 3 (classic).
 * 
 * ## Usage
 * -nc3
 *
 * @param ARG0 List of function arguments set by wgrib2's main() function (see @ref ARG0). These arguments 
 * won't be relevant to the average wgrib2 user. See the Usage section above for details about any input 
 * parameters.
 *
 * @return Always returns 0
 *
 * @author Sergey Varlamov @date 2/18/2008  
 */
int f_nc3(ARG0)
{
  if (mode == -1) nc4 = 0;
  return 0;
}

#else

int f_nc_nlev(ARG1)
{
  if (mode == -1) {fprintf(stderr,"netcdf package not installed\n"); return 1;}
  return 0;
}
int f_nc_pack(ARG1)
{
  if (mode == -1) {fprintf(stderr,"netcdf package not installed\n"); return 1;}
  return 0;
}
int f_nc_table(ARG1)
{
  if (mode == -1) {fprintf(stderr,"netcdf package not installed\n"); return 1;}
  return 0;
}
int f_no_nc_table(ARG0)
{
  if (mode == -1) {fprintf(stderr,"netcdf package not installed\n"); return 1;}
  return 0;
}
int f_no_nc_pack(ARG0)
{
  if (mode == -1) {fprintf(stderr,"netcdf package not installed\n"); return 1;}
  return 0;
}
int f_nc_grads(ARG0)
{
  if (mode == -1) {fprintf(stderr,"netcdf package not installed\n"); return 1;}
  return 0;
}
int f_no_nc_grads(ARG0)
{
  if (mode == -1) {fprintf(stderr,"netcdf package not installed\n"); return 1;}
  return 0;
}
int f_nc_time(ARG1)
{
  if (mode == -1) {fprintf(stderr,"netcdf package not installed\n"); return 1;}
  return 0;
}
int f_no_nc_time(ARG0)
{
  if (mode == -1) {fprintf(stderr,"netcdf package not installed\n"); return 1;}
  return 0;
}
int f_nc4(ARG0)
{
  if (mode == -1) {fprintf(stderr,"netcdf package not installed\n"); return 1;}
  return 0;
}
int f_nc3(ARG0)
{
  if (mode == -1) {fprintf(stderr,"netcdf package not installed\n"); return 1;}
  return 0;
}

#endif



