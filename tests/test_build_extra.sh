#!/bin/sh
# This test script simply checks that BUILD_EXTRA=ON works correctly.
#
# Alyson Stahl 3/20/25

set -e
echo ""
echo "*** Checking BUILD_EXTRA=ON works correctly"

echo "Checking that fnlist is updated with fnlist_extra..."
n_extra_fns=3
output=$(../wgrib2/wgrib2 -h all 2>&1 | tail -n "$n_extra_fns")
expected_output="
-mysql           out   5 args H=[host] U=[user] P=[password] D=[db] T=[table]
-mysql_dump      out   7 args H=[host] U=[user] P=[password] D=[db] T=[table] W=[western_lons:0|1] PV=[remove unlikely:0|1]
-mysql_speed     out   7 args H=[host] U=[user] P=[password] D=[db] T=[table] W=[western_lons:0|1] PV=[remove unlikely:0|1]"

if [ "$output" != "$expected_output" ]; then
  echo "Test failed: Output does not match expected last $n_extra_fns lines."
  echo "Actual output:"
  echo "$output"
  exit 1
fi

echo "*** SUCCESS!"
exit 0