# Neural Network learning Physics

This is a simple project where a neural network is used to learn Newtonian motion.

## Building
```
    mkdir build
    cd build
    cmake ../
    cmake --build . --parallel
```

## Usage
The main app is built under the directory example. You can run

```
    ./example/example
```

This will output a file calle "parabola.data" with a set of values of computed
network feed forwards outside of the training range. This can be used to
generate a graph that overlaps its values against the real physical values,
calling the include gnuplot script from the build directory

```
    gnuplot ../plots/plot.gnuplot
```

## More Info
[ReachableCode.com](https://www.reachablecode.com)
