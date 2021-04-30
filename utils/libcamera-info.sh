#!/bin/sh

# SPDX-License-Identifier: GPL-2.0-or-later
# Investigates the current media state of the system to ease reporting

# Identify all V4L2 devices
for v in /sys/class/video4linux/{v4l,video}*;
  do
    vn=`basename $v`
    name=`cat $v/name`
    echo $vn: $name
  done | sort -V;

# Identify all Media Devices
for m in /sys/bus/media/devices/media*;
  do
    mn=`basename $m`
    model=`cat $m/model`
    echo $mn: $model
  done | sort -V;

# Print all media graphs
for d in /dev/media*
  do
    dev=`basename $d`
    echo "$d:"
    media-ctl -p -d $d
    echo ""
    media-ctl -d $d --print-dot | dot -Tpng > $dev-graph.png
  done
