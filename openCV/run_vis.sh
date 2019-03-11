#!/bin/bash

v4l2-ctl -d /dev/video0 -p 60
v4l2-ctl --set-ctrl=vertical_flip=1
v4l2-ctl --set-ctrl=horizontal_flip=1

./vis
