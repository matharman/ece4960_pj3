#!/bin/bash

BIN=vis
DATA=circle_data.txt

if [ ! -e $BIN ]
then
    make gui
fi

./$BIN >> $DATA

if [ ! -e $DATA ]
then
    echo "Program did not produce data"
fi

#    set term x11 0
#    set title "Centroid Position"
#    set xlabel "Time (20ms)"
#    set ylabel "Position (pixels)"
#    plot for [col=1:2] 'circle_data.txt' using col with lines title columnheader

#    plot '' using 3 with lines title columnheader
gnuplot -p <<-EOFMARKER
    set datafile separator ","

    set term x11 0
    set title "Centroid Velocity"
    set xlabel "Time (ms)"
    set ylabel "Velocity (pixels / sec)"
    plot for [col=1:2] 'circle_data.txt' using col with lines title columnheader
EOFMARKER
