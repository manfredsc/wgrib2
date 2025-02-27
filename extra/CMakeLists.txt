# This is the CMake build file for the wgrib2 project, extra 
# subdirectory.
#
# It optionally builds all user-contributed (non-operational) 
# codes if BUILD_EXTRA=ON.

set(src Mysql.c Mysql_dump.c Mysql_speed.c)

add_library(wgrib2_extra OBJECT ${src})

if (BUILD_LIB)
  set_property(TARGET wgrib2_extra PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()