set term png
set output 'parabolic_motion_neural_net.png'

set title "Neural Network parabolic motion estimation"

# Line width of the axes
set border linewidth 1.5
# Line styles
set style line 1 linecolor rgb '#0060ad' linetype 1 linewidth 2
set style line 2 linecolor rgb '#dd181f' linetype 1 linewidth 2

set xlabel 'time(s)'
set ylabel 'y'

set xrange [0:10]
set yrange [0:-600]

# Plot
plot 'parabola.data' using 1:2 with linespoints linestyle 1 title "real y(t)", \
     'parabola.data' using 1:3 with linespoints linestyle 2 title "neural net y(t)"
