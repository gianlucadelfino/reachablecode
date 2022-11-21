set term png
set output 'histogram.png'

set title "TSC deltas"

binwidth=0.2
bin(x,width)=width*floor(x/width)
set style fill solid
set xrange [0:500]
# set yrange [0:60000]
plot 'deltas.dat' using (bin($1,binwidth)):(1.0) smooth freq with boxes
