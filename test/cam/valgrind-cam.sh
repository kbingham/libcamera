#!/bin/sh

# SPDX-License-Identifier: GPL-2.0-or-later

TestPass=0
TestFail=255
TestSkip=77

valgrind=$(command -v valgrind)

if [ x"" = x"$valgrind" ] ; then
	echo "Valgrind unavailable ..."
	exit $TestSkip
fi

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

for i in $(seq 1 1 "$num_cameras");
do
	log_file="valgrind-cam-$i.log"
	"$valgrind" \
		--leak-check=full \
		--show-leak-kinds=all \
		--error-exitcode=1 \
		"$cam" -c "$i" -C10 > "$log_file" 2>&1
	ret=$?
	if [ $ret != 0 ] ; then
		echo "$i - $valgrind returned $ret"
		exit $TestFail
	fi

	# I'd prefer a better way of checking there are no leaks, as well as reporting
	# the different categories from valgrind as distinct tests.
	if ! grep "no leaks are possible" "$log_file" > /dev/null; then
		echo "$i - Valgrind Errors detected"
		cat "$log_file" > /dev/stderr
		exit $TestFail
	fi
done;

exit $TestPass
