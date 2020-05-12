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

mv home/travis/build/hn-88/opencvdeb/opencv/build/OpenCVLocal ~/OpenCVLocal -v
rm -Rvf home

# doing this copy so that ldd can find the libs for linuxdeployqt
# https://www.hpc.dtu.dk/?page_id=1180
sudo cp -R /home/travis/OpenCVLocal/lib/* /usr/local/lib

sudo ln /usr/local/lib/libopencv_core.so.4.3.0 /usr/local/lib/libopencv_core.so.4.3
sudo ln /usr/local/lib/libopencv_core.so.4.3 /usr/local/lib/libopencv_core.so

sudo ln /usr/local/lib/libopencv_highgui.so.4.3.0 /usr/local/lib/libopencv_highgui.so.4.3
sudo ln /usr/local/lib/libopencv_highgui.so.4.3 /usr/local/lib/libopencv_highgui.so

sudo ln /usr/local/lib/libopencv_imgcodecs.so.4.3.0 /usr/local/lib/libopencv_imgcodecs.so.4.3
sudo ln /usr/local/lib/libopencv_imgcodecs.so.4.3 /usr/local/lib/libopencv_imgcodecs.so

sudo ln /usr/local/lib/libopencv_imgproc.so.4.3.0 /usr/local/lib/libopencv_imgproc.so.4.3
sudo ln /usr/local/lib/libopencv_imgproc.so.4.3   /usr/local/lib/libopencv_imgproc.so

sudo ln /usr/local/lib/libopencv_videoio.so.4.3.0 /usr/local/lib/libopencv_videoio.so.4.3
sudo ln /usr/local/lib/libopencv_videoio.so.4.3   /usr/local/lib/libopencv_videoio.so

sudo ln /usr/local/lib/libopencv_video.so.4.3.0 /usr/local/lib/libopencv_video.so.4.3
sudo ln /usr/local/lib/libopencv_video.so.4.3   /usr/local/lib/libopencv_video.so





