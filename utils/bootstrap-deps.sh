#!/bin/bash

app=`basename $0`

# Todo: Autodetect system
system=ubuntu

toolchains=(gcc clang)
modules=(android cam qcam raspberrypi tracing gstreamer hotplug documentation tracing)

usage() {
	echo -n "$app [all]"
	for t in ${toolchains[@]}; do
		echo -n " [$t]"
	done
	for m in ${modules[@]}; do
		echo -n " [$m]"
	done
	echo ""
}

contains() {
    [[ $1 =~ (^|[[:space:]])$2($|[[:space:]]) ]]
}

declare -A core
declare -A android
declare -A cam
declare -A qcam
declare -A raspberrypi
declare -A tracing
declare -A gstreamer
declare -A documentation
declare -A hotplug

core["ubuntu"]="ninja-build pkg-config python3-yaml python3-ply python3-jinja2 libgnutls28-dev openssl"
gcc["ubuntu"]="gcc g++"
clang["ubuntu"]="clang"
android["ubuntu"]="cmake libexif-dev libjpeg-dev libyaml-dev"
cam["ubuntu"]="libevent-dev"
qcam["ubuntu"]="qtbase5-dev libqt5core5a libqt5gui5 libqt5widgets5 qttools5-dev-tools libtiff-dev"
raspberrypi["ubuntu"]="libboost-dev"
tracing["ubuntu"]="liblttng-ust-dev python3-jinja2"
gstreamer["ubuntu"]="libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev"
documentation["ubuntu"]="python3-sphinx doxygen graphviz"
hotplug["ubuntu"]="libudev-dev"

# Default to all modules if nothing is given
choices=${@:-${modules[@]}}

if [[ "x$1" == x"all" ]];
then
	choices=${modules[@]}
fi


# Core requirements are always met
selected="${core[$system]} "

for c in ${choices[@]}; do
	case $c in
		android) selected+="${android[$system]} ";;
		cam) selected+="${cam[$system]} ";;
		qcam) selected+="${qcam[$system]} ";;
		raspberrypi) selected+="${raspberrypi[$system]} ";;
		tracing) selected+="${tracing[$system]} ";;
		gstreamer) selected+="${gstreamer[$system]} ";;
		hotplug) selected+="${hotplug[$system]} ";;
		documentation) selected+="${documentation[$system]} ";;
		gcc) selected+="${gcc[$system]} ";;
		clang) selected+="${clang[$system]} ";;
		*)
			usage
			exit
			;;
	esac
done

# Todo: Support other package managers
apt-get install $selected
