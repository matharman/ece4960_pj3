#!/bin/bash

BIN=gui-vis
DATA=circle_data.txt

if [ ! -e $BIN ]
then
    make gui
fi

./gui-vis

if [ ! -e $DATA ]
then
    echo "Program did not produce data"
fi

gnuplot -p <<-EOFMARKER
    set datafile separator ","
    set multiplot layout 1,2 title "Centroid Motion Data"
    
    #
    set title "Centroid Position"
    unset key
    plot for [col=1:2] 'circle_data.txt' using col with lines title columnheader
    
    #
    set title "Centroid Velocity"
    unset key
    plot 'circle_data.txt' using 3 with lines columnheader
    
    unset multiplot
EOFMARKER
