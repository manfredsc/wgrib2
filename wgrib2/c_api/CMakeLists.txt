# This is the CMake file for the c_api directory in the wgrib2
# project. This will only build if BUILD_LIB=ON
#
# Alyson Stahl


set(src grb2_cmd.c grb2_inq.c grb2_mk_inv.c grb2_wrt.c read.c write.c)

if (BUILD_SHARED_LIB)
   add_library(wgrib2_c_api SHARED ${src})
else()
   add_library(wgrib2_c_api STATIC ${src})
   set_property(TARGET wgrib2_c_api PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

target_include_directories(wgrib2_c_api
    PUBLIC $<BUILD_INTERFACE:${module_dir}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(wgrib2_c_api PUBLIC wgrib2_lib)  

if(NOT DEFINED CMAKE_INSTALL_Fortran_MODULES)
    set(CMAKE_INSTALL_Fortran_MODULES ${CMAKE_INSTALL_INCLUDEDIR})
endif()

install(
  TARGETS wgrib2_c_api
  EXPORT wgrib2_exports
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
