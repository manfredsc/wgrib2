#!/bin/sh

## @file
## @brief This script updates wgrib2 with WMO code info.
## @author Public Domain: Manfred Schwarb <schwarb@meteodat.ch>
## @date 10/2024

## @cond

urlbase="https://github.com/wmo-im/GRIB2"

outfile="CodeTable_4.7.dat"
if [ -f "$outfile" ]; then mv "$outfile" "$outfile.old"; fi

#---GRIB2 Code Table 4.7: Derived forecast
#---NCEP additions from https://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc/grib2_table4-7.shtml
wget -nv "$urlbase/raw/master/GRIB2_CodeFlag_4_7_CodeTable_en.csv" -O- | sed '{
    s/, /# /g
    s/,/;/g
    s/# /, /g
    s/"//g
  }' | env LC_ALL=en_US iconv -c -f UTF8 -t ASCII//TRANSLIT | awk -F";" '
  {
    num=$3; name=$5
    if (num != "" && num !~ "-" && num !~ "Code") {
      if (num==255) {  # prepend custom NCEP entries before case 255
        print "case   192: if (center == NCEP) string=\"Unweighted Mode of All Members\"; break;"
        print "case   193: if (center == NCEP) string=\"Percentile value (10%) of All Members\"; break;"
        print "case   194: if (center == NCEP) string=\"Percentile value (50%) of All Members\"; break;"
        print "case   195: if (center == NCEP) string=\"Percentile value (90%) of All Members\"; break;"
        print "case   196: if (center == NCEP) string=\"Statistically decided weights for each ensemble member\"; break;"
        print "case   197: if (center == NCEP) string=\"Climate Percentile (percentile values from climate distribution)\"; break;"
        print "case   198: if (center == NCEP) string=\"Deviation of Ensemble Mean from Daily Climatology\"; break;"
        print "case   199: if (center == NCEP) string=\"Extreme Forecast Index\"; break;"
        print "case   200: if (center == NCEP) string=\"Equally Weighted Mean\"; break;"
        print "case   201: if (center == NCEP) string=\"Percentile value (5%) of All Members\"; break;"
        print "case   202: if (center == NCEP) string=\"Percentile value (25%) of All Members\"; break;"
        print "case   203: if (center == NCEP) string=\"Percentile value (75%) of All Members\"; break;"
        print "case   204: if (center == NCEP) string=\"Percentile value (95%) of All Members\"; break;"
      }
      printf "case %5d: string=\"%s\"; break;\n",num,name
    }
  }' > "$outfile"

exit

## @endcond
