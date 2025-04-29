# Simulating stocks with OpenCL

A small example that creates a Monte Carlo simulation with OpenCL

## Build steps

```
    git clone https://github.com/bstatcomp/RandomCL.git ../3rdParties
```

```
    sudo apt install ocl-icd-opencl-dev
```

```
    mkdir build
    cd build
    cmake ../
    cmake --build .
```

## Running it

```
    ./MonteCarloOpenCL
    gnuplot create_histogram.gnuplot
```
Then the file histogram.png should be now in the build directory.

## Troubleshooting

You may need to fix your opencl/drivers if the program cannot find "platforms" to run on.

## More Info
[ReachableCode.com](https://www.reachablecode.com)
