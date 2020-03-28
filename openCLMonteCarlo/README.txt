Build steps

git clone https://github.com/bstatcomp/RandomCL.git ../3rdParties

mkdir build
cd build
cmake ../
make
./MonteCarloOpenCL
gnuplot create_histogram.gnuplot

// The file histogram.png should be now in the build directory

