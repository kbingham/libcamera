#!/bin/bash

app=`basename $0`

# Todo: Autodetect system
system=ubuntu

distro_codename=$(lsb_release --codename --short)
distro_id=$(lsb_release --id --short)
supported_codenames="(trusty|xenial|bionic|disco|eoan|focal)"
supported_ids="(Debian)"
if [ 0 -eq "${do_unsupported-0}" ] && [ 0 -eq "${do_quick_check-0}" ] ; then
  if [[ ! $distro_codename =~ $supported_codenames &&
        ! $distro_id =~ $supported_ids ]]; then
    echo -e "ERROR: The only supported distros are\n" \
      "\tUbuntu 18.04 LTS (bionic with EoL April 2028)\n" \
      "\tUbuntu 20.04 LTS (focal with Eol April 2030)\n" \
      "\tUbuntu 19.04 (disco)\n" \
      "\tUbuntu 19.10 (eoan)\n" \
      "\tDebian 8 (jessie) or later" >&2
    exit 1
  fi




toolchains=(gcc clang)
modules=(android cam qcam raspberrypi tracing gstreamer hotplug documentation tracing developer)

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
declare -A developer

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
developer["ubuntu"]="codespell pycodestyle clang-format shellcheck"

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
		gcc) selected+="${gcc[$system]} ";;
		clang) selected+="${clang[$system]} ";;
		android) selected+="${android[$system]} ";;
		cam) selected+="${cam[$system]} ";;
		qcam) selected+="${qcam[$system]} ";;
		raspberrypi) selected+="${raspberrypi[$system]} ";;
		tracing) selected+="${tracing[$system]} ";;
		gstreamer) selected+="${gstreamer[$system]} ";;
		hotplug) selected+="${hotplug[$system]} ";;
		documentation) selected+="${documentation[$system]} ";;
		developer) selected+="${developer[$system]} ";;
		*)
			usage
			exit
			;;
	esac
done

# Todo: Support other package managers
apt-get install $selected
