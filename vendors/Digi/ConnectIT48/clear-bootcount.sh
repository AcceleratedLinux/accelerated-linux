#!/bin/sh
# reset boot counter (ie., we have booted successfully)
# 0 = bootcntr, 89 = BCD (59 - bootcntr)

echo 0 > /sys/class/rtc/rtc0/device/alarm_seconds
echo 89 > /sys/class/rtc/rtc0/device/alarm_minutes

