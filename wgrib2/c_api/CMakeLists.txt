# This is the CMake file for the c_api directory in the wgrib2
# project. This will only build if BUILD_LIB=ON
#
# Alyson Stahl

set(src grb2_cmd.c grb2_inq.c grb2_mk_inv.c grb2_wrt.c)

if (BUILD_SHARED_LIB)
   add_library(wgrib2_c_api SHARED ${src})
   set_target_properties(wgrib2_c_api PROPERTIES SOVERSION 0)
else()
   add_library(wgrib2_c_api STATIC ${src})
   set_property(TARGET wgrib2_c_api PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

target_include_directories(wgrib2_c_api PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}>)

set_target_properties(wgrib2_c_api PROPERTIES PUBLIC_HEADER c_wgrib2api.h)
target_link_libraries(wgrib2_c_api PUBLIC wgrib2_lib) 

install(
  TARGETS wgrib2_c_api
  EXPORT wgrib2_exports
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})