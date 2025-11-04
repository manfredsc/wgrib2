#!/bin/sh

## @file
## @brief This script updates wgrib2 with WMO code info.
## @author Public Domain: Manfred Schwarb <schwarb@meteodat.ch>
## @date 6/2025

## @cond

urlbase="https://github.com/wmo-im/CCT"

outfile="CommonCodeTable_12.dat"
if [ -f "$outfile" ]; then mv "$outfile" "$outfile.old"; fi

#---Common Code Table 12: Sub-centres of originating centres
wget -nv "$urlbase/raw/master/C12.csv" -O- | sed '{
    s/, /# /g
    s/,/;/g
    s/# /, /g
    s/"//g
  }' | env LC_ALL=en_US iconv -c -f UTF8 -t ASCII//TRANSLIT \
    | grep -v "Reserved for other centres" | awk -F";" '
  {
    num=$1+0; name=$4; subnum=$3+0
    if (num>0) {  # omit section headers
      printf "case %10d: string=\"%s\"; break;  // centre=%d, subcentre=%d\n",lshift(num,16)+subnum,name,num,subnum
    }
  }' > "$outfile"

exit

## @endcond