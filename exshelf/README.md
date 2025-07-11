# ExShelf

## DISCLAIMER: This is a WIP

This is a small program that aims to help catalogue books on a bookshelf.

The program uses OpenCV and Tesseract to look for titles in the framed image of
a webcam and builds up a list.

# to build (adjust number of cores passed to --parallel accordingly)

```
    sudo apt install libleptonica-dev
    cmake -B build
    cmake --build build --parallel 4

    cd build
    wget https://raw.githubusercontent.com/tesseract-ocr/tessdata/refs/heads/main/ita.traineddata
    wget https://www.dropbox.com/s/r2ingd0l3zt8hxs/frozen_east_text_detection.tar.gz
    tar -xvf frozen_east_text_detection.tar.gz
```

# to run it (from the build directory)

```
    ./bin/exshelf
```

## More Info
[ReachableCode.com](https://www.reachablecode.com)
