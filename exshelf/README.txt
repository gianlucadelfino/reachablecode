==ExShelf==

##WIP

This is a small program that aims to help catalogue books on a bookshelf.

The program uses OpenCV and Tesseract to look for titles in the framed image of
a webcam and builds up a list.

# to install tesseract:

sudo apt install libtesseract-dev
sudo apt-get install tesseract-ocr-ita

# to build

mkdir build
cd build
cmake ..

wget https://www.dropbox.com/s/r2ingd0l3zt8hxs/frozen_east_text_detection.tar.gz
tar -xvf frozen_east_text_detection.tar.gz

make -j


# to run it

./exshelf
