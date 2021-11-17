#!/bin/sh

SESSION_NAME=libcamera.trace

lttng create $SESSION_NAME
lttng enable-event -u libcamera:\*
lttng start

# run libcamera application
"$@"

lttng stop
lttng view
lttng destroy $SESSION_NAME

