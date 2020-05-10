#!/bin/bash

# install script for OpenCV-4.3.0
# using pre-built binaries
# from https://github.com/hn-88/opencvdeb/

# install all the opencv dependencies
sudo apt-get install -y build-essential
sudo apt-get install -y cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
sudo apt-get install -y libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev

wget https://github.com/hn-88/opencvdeb/releases/download/v4.3.0.1/OpenCVbuild.zip
unzip OpenCVbuild.zip

mv home/travis/build/hn-88/opencvdeb/opencv/build/OpenCVLocal OpenCVLocal -v
rm -Rvf ~/home



