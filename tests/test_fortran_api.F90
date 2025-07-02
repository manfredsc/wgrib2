program test_fortran_api
   use wgrib2api
   implicit none

   character(*) :: GRB2_FILE
   parameter(GRB2_FILE = 'data/gdaswave.t00z.wcoast.0p16.f000.grib2')
   character(*) :: GRB2_INV
   parameter(GRB2_INV = 'junk_ftn_api.inv')
   character(*) :: EXP_GRB2_INV
   parameter(EXP_GRB2_INV = 'data/ref_gdaswave.t00z.wcoast.0p16.f000.grib2.inv')

   integer :: iret
   
   iret = grb2_mk_inv(GRB2_FILE, GRB2_INV, .false.)
   if (iret .ne. 0) stop 2
   
   print *, 'SUCCESS!'
end program test_fortran_api