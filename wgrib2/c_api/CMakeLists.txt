# This is the CMake file for the c_api directory in the wgrib2
# project. This will only build if BUILD_LIB=ON
#
# Alyson Stahl

set(src c_wgrib2api.h grb2_cmd.c grb2_inq.c grb2_mk_inv.c grb2_wrt.c)

add_library(wgrib2_c_api OBJECT ${src})
set_property(TARGET wgrib2_c_api PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(wgrib2_c_api PRIVATE ${CMAKE_BINARY_DIR}/wgrib2)
target_link_libraries(wgrib2_c_api PUBLIC wgrib2_lib) 
target_include_directories(wgrib2_c_api PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_SOURCE_DIR}>"
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>)

install(
  TARGETS wgrib2_c_api
  EXPORT wgrib2_exports
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
