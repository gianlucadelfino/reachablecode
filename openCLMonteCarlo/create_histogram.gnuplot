set term png
set output 'histogram.png'

binwidth=0.2
bin(x,width)=width*floor(x/width)

plot 'out.txt' using (bin($1,binwidth)):(1.0) smooth freq with boxes
