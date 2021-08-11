#!/bin/sh

# SPDX-License-Identifier: GPL-2.0-or-later

TestPass=0
TestFail=255
TestSkip=77

# Tests expect to be run from the meson.project_build_root()
cam=${1:-src/cam/cam}

if [ ! -e "$cam" ] ; then
	echo "Failed to find cam utility."
	exit $TestFail
fi

# Unfortunately, we don't have a 'machine interface', so we rely on parsing the
# output of cam...
num_cameras=$("$cam" -l | grep -cv "Available")

if [ "$num_cameras" -lt 1 ];
then
	echo "Warning: No cameras available"
	exit $TestSkip
fi

## Run for each camera detected
for i in $(seq 1 1 "$num_cameras");
do
	"$cam" -c "$i" -C10
	ret=$?
	if [ $ret != 0 ] ; then
		echo "$i - $cam returned $ret"
		continue
	fi
done;

exit $TestPass
