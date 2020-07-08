#!/bin/bash

OPENCV_VERSION=4.3.0

cd ~
wget -O opencv.zip https://github.com/opencv/opencv/archive/${OPENCV_VERSION}.zip
unzip opencv.zip
wget -O opencv_contrib.zip https://github.com/opencv/opencv_contrib/archive/${OPENCV_VERSION}.zip
unzip opencv_contrib.zip
cd opencv-${OPENCV_VERSION}
mkdir -p build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D BUILD_PROTOBUF=OFF -D BUILD_opencv_dnn=OFF -D UPDATE_PROTO_FILES=ON -D BUILD_PNG=OFF -D BUILD_JPEG=ON -D INSTALL_C_EXAMPLES=OFF -D OPENCV_EXTRA_MODULES_PATH=~/opencv_contrib-${OPENCV_VERSION}/modules -D WITH_CUDA=OFF -D BUILD_EXAMPLES=OFF -D BUILD_opencv_java=OFF -D BUILD_opencv_python2=OFF -D BUILD_opencv_python3=OFF -D INSTALL_PYTHON_EXAMPLES=OFF -D WITH_TBB=ON -D BUILD_SHARED_LIBS=OFF ..
make -j8
sudo make install

rm -rf opencv.zip
rm -rf opencv_contrib.zip
rm -rf opencv-${OPENCV_VERSION}
rm -rf opencv_contrib-${OPENCV_VERSION}
