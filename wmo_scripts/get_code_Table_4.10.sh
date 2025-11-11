#!/bin/sh

## @file
## @brief This script updates wgrib2 with WMO code info.
## @author Public Domain: Manfred Schwarb <schwarb@meteodat.ch>
## @date 10/2024

## @cond

urlbase="https://github.com/wmo-im/GRIB2"

outfile="CodeTable_4.10.dat"
if [ -f "$outfile" ]; then mv "$outfile" "$outfile.old"; fi

#---GRIB2 Code Table 4.10: Type of statistical processing
#---NCEP additions from https://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc/grib2_table4-10.shtml
wget -nv "$urlbase/raw/master/GRIB2_CodeFlag_4_10_CodeTable_en.csv" -O- | sed '{
    s/, /# /g
    s/,/;/g
    s/# /, /g
    s/"//g
  }' | env LC_ALL=en_US iconv -c -f UTF8 -t ASCII//TRANSLIT | awk -F";" '
  {
    num=$3; name=$5
    if (num != "" && num !~ "-" && num !~ "Code") {
      switch (num) {
        case   0: wgrib2name="ave"; break
        case   1: wgrib2name="acc"; break
        case   2: wgrib2name="max"; break
        case   3: wgrib2name="min"; break
        case   4: wgrib2name="last-first"; break
        case   5: wgrib2name="RMS"; break
        case   6: wgrib2name="StdDev"; break
        case   7: wgrib2name="covar"; break
        case   8: wgrib2name="first-last"; break
        case   9: wgrib2name="ratio"; break
        case  10: wgrib2name="standardized anomaly"; break
        case  11: wgrib2name="summation"; break
        case  12: wgrib2name="return period"; break
        case  13: wgrib2name="median"; break
        case 100: wgrib2name="severity"; break
        case 101: wgrib2name="mode"; break
        case 102: wgrib2name="index processing"; break
        case 255: wgrib2name="missing"; break
        default: { print "ERROR: missing switch statement for",num > "/dev/stderr"; exit 1 }
      }
      if (num==255) {  # prepend custom NCEP entries before case 255
        print "case   192: if (center == NCEP) string=\"climatological mean\"; break;"
        print "case   193: if (center == NCEP) string=\"average of forecasts\"; break;"
        print "case   194: if (center == NCEP) string=\"average of uninitialized analyses\"; break;"
        print "case   195: if (center == NCEP) string=\"average of forecast accumulations\"; break;"
        print "case   196: if (center == NCEP) string=\"average of cuccessive forecast accumulations\"; break;"
        print "case   197: if (center == NCEP) string=\"average of forecast averages\"; break;"
        print "case   198: if (center == NCEP) string=\"average of successive forecast averages\"; break;"
        print "case   199: if (center == NCEP) string=\"climatological average of analyses\"; break;"
        print "case   200: if (center == NCEP) string=\"climatological average of forecasts\"; break;"
        print "case   201: if (center == NCEP) string=\"climatological RMS\"; break;"
        print "case   202: if (center == NCEP) string=\"climatological StdDev of forecasts\"; break;"
        print "case   203: if (center == NCEP) string=\"climatological StdDev of analyses\"; break;"
        print "case   204: if (center == NCEP) string=\"average of forecast accumulations in 6h intervals\"; break;"
        print "case   205: if (center == NCEP) string=\"average of forecast averages in 6h intervals\"; break;"
        print "case   206: if (center == NCEP) string=\"average of forecast accumulations in 12h intervals\"; break;"
        print "case   207: if (center == NCEP) string=\"average of forecast averages in 12h intervals\"; break;"
        print "case   208: if (center == NCEP) string=\"variance\"; break;"
        print "case   209: if (center == NCEP) string=\"coefficient\"; break;"
      }
      printf "case %5d: string=\"%s\"; break;  // %s\n",num,wgrib2name,name
    }
  }' > "$outfile"

exit

## @endcond
