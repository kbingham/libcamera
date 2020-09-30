#!/bin/sh

# SPDX-License-Identifier: GPL-2.0-or-later

TestPass=0
TestFail=255
TestSkip=77

# Initialise success, set for failure.
ret=$TestPass

ok() {
	echo "ok $*"
}

nok() {
	echo "not ok $*"
	ret=$TestFail
}

valgrind=$(command -v valgrind)

if [ x"" = x"$valgrind" ] ; then
	echo "skip 1 - Valgrind unavailable ..."
	exit $TestSkip
fi

# Tests expect to be run from the meson.project_build_root()
cam=${1:-src/cam/cam}

if [ ! -e "$cam" ] ; then
	nok "1 - failed to find cam utility."
	exit $TestFail
fi

# Unfortunately, we don't have a 'machine interface', so we rely on parsing the
# output of cam...
num_cameras=$("$cam" -l | grep -v "Available" | wc -l)

# Enter TAP plan
echo "1..$num_cameras"

for i in $(seq 1 1 "$num_cameras");
do
	"$cam" -c "$i" -C10
	ret=$?
	if [ $ret != 0 ] ; then
		nok "$i - $cam returned $ret"
		continue
	fi

	log_file="valgrind-cam-$i.log"
	"$valgrind" "$cam" -c "$i" -C10 > "$log_file" 2>&1
	ret=$?
	if [ $ret != 0 ] ; then
		nok "$i - $valgrind returned $ret"
		continue
	fi

	# I'd prefer a better way of checking there are no leaks, as well as reporting
	# the different categories from valgrind as distinct tests.
	if ! grep "no leaks are possible" "$log_file" > /dev/null; then
		nok "$i - Valgrind Errors detected"
		cat $log_file > /dev/stderr
		continue
	fi

	ok "$i - Camera $i reports no leaks"
done;

exit $ret
