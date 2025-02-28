#include <stdio.h>
#include <stdlib.h>
#include "wgrib2.h"
#include "fnlist_extra.h"

struct extraftn extraftns[] = {
    {"mysql",f_mysql, output, 5, "H=[host] U=[user] P=[password] D=[db] T=[table]", 100},
    {"mysql_dump",f_mysql_dump, output, 7, "H=[host] U=[user] P=[password] D=[db] T=[table] W=[western_lons:0|1] PV=[remove unlikely:0|1]", 100},
    {"mysql_speed",f_mysql_speed, output, 7, "H=[host] U=[user] P=[password] D=[db] T=[table] W=[western_lons:0|1] PV=[remove unlikely:0|1]", 100},
};