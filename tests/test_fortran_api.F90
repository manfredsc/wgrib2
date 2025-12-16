program test_fortran_api

   use wgrib2api
   use, intrinsic :: iso_c_binding, only: c_char, c_int, c_null_char
   implicit none

   character(len=*), parameter :: GRB2_FILE = "data/gdaswave.t00z.wcoast.0p16.f000.grib2"
   character(len=*), parameter :: GRB2_INV = "junk_ftn_api.inv"
   character(len=*), parameter :: EXP_GRB2_INV = "data/ref_gdaswave.t00z.wcoast.0p16.f000.grib2.inv"
   logical :: use_ncep_table = .false.

   logical :: ret
   integer :: iret

   interface 
      function compare_files(fname1, fname2) bind(c)
         use, intrinsic :: iso_c_binding, only: c_char, c_int
         implicit none
         character(kind=c_char), intent(in) :: fname1(*)
         character(kind=c_char), intent(in) :: fname2(*)
         integer(c_int) :: compare_files
      end function compare_files
      function compare_grib2_files(fname1, fname2) bind(c)
         use, intrinsic :: iso_c_binding, only: c_char, c_int
         implicit none
         character(kind=c_char), intent(in) :: fname1(*)
         character(kind=c_char), intent(in) :: fname2(*)
         integer(c_int) :: compare_grib2_files
      end function compare_grib2_files
   end interface
   
   print *, "Testing Fortran API..."

   print *, "Testing grb2_DEFINED_VAL() and grb2_UNDEFINED_VAL()..."

   ret = grb2_DEFINED_VAL(9.998e20)
   if (.not. ret) stop 2

   ret = grb2_DEFINED_VAL(9.99911e20)
   if (.not. ret) stop 3

   ret = grb2_DEFINED_VAL(grb2_UNDEFINED)
   if (ret) stop 4

   ret = grb2_UNDEFINED_VAL(9.998e20)
   if (ret) stop 5

   ret = grb2_UNDEFINED_VAL(9.99911e20)
   if (ret) stop 6

   ret = grb2_UNDEFINED_VAL(grb2_UNDEFINED)
   if (.not. ret) stop 7

   print *, "Testing grb2_mk_inv()..."

   iret = grb2_mk_inv(GRB2_FILE, GRB2_INV, use_ncep_table)
   if (iret .ne. 0) stop 11
   
   iret = compare_files(trim(GRB2_INV)//c_null_char, trim(EXP_GRB2_INV)//c_null_char)
   if (iret .ne. 0) stop 12

   print *, 'SUCCESS!'
end program test_fortran_api