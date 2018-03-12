#!/bin/bash

cd ~
wget -O opencv.zip https://github.com/opencv/opencv/archive/3.4.1.zip
unzip opencv.zip
wget -O opencv_contrib.zip https://github.com/opencv/opencv_contrib/archive/3.4.1.zip
unzip opencv_contrib.zip
cd opencv-3.4.1
mkdir -p build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D BUILD_PROTOBUF=OFF -D BUILD_opencv_dnn=OFF -D UPDATE_PROTO_FILES=ON -D BUILD_PNG=OFF -D BUILD_JPEG=ON -D INSTALL_C_EXAMPLES=OFF -D OPENCV_EXTRA_MODULES_PATH=~/opencv_contrib-3.4.1/modules -D WITH_CUDA=OFF -D BUILD_EXAMPLES=OFF -D BUILD_opencv_java=OFF -D BUILD_opencv_python2=OFF -D BUILD_opencv_python3=OFF -D INSTALL_PYTHON_EXAMPLES=OFF -D WITH_TBB=ON -D BUILD_SHARED_LIBS=OFF ..
make -j4
sudo make install

rm -rf opencv.zip
rm -rf opencv_contrib.zip
rm -rf opencv-3.4.1
rm -rf opencv_contrib-3.4.1
