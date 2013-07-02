# Ground Truth System #

Ground Truth System - A tool for visually tracking a moving target on a calibrated ground plane and recording position and angle in 2-dimensions.

[![Build Status](https://travis-ci.org/dysonltd/gts.png?branch=develop)](https://travis-ci.org/dysonltd/gts)

![Screenshot](help/doc/gts_userguide_files/screenshot.png?raw=true)

Several additional functions are included that allow for post-processing of the tracking data to generate [IEC](http://www.iec.ch/) specific results.

## Requirements ##

### Linux ###

__CMake__ (2.8.10.1)

    sudo apt-get install cmake cmake-qt-gui

__Qt__ (4.8.1)

    sudo apt-get install libqt4-dev

__OpenCV__ (2.4)

    git clone https://github.com/Itseez/opencv.git 
    git checkout 2.4.5

__Unicap__

    sudo apt-get install libunicap2-dev

### Windows ###

* [CMake](http://www.cmake.org/cmake/resources/software.html) (2.8.10.1)

* [Qt Creator](http://qt-project.org/downloads) (2.6.0)

* [Xvid](http://www.xvid.org/) (1.3.2)

* [OpenCV](http://sourceforge.net/projects/opencvlibrary/files/opencv-win/) (2.4)

__MinGW Only__

* [MinGW](ftp://ftp.qt.nokia.com/misc/MinGW-gcc440_1.zip) (4.4)

* [Qt MinGW Libraries](http://qt-project.org/downloads) (4.8.3)

* [MSYS](http://www.mingw.org/wiki/MSYS) (1.0.11)

__Visual Studio Only__

* [Microsoft Visual C++ 2010](http://www.microsoft.com/visualstudio/eng/products/visual-studio-2010-express)

## Compiling ##

__Linux__

From root directory:

    $ mkdir build && cd build
    $ cmake [options] ../
	    e.g. cmake -DOpenCV_ROOT_DIR=/home/username/opencv2.4.5/ -DCMAKE_BUILD_TYPE=Debug -DGTS_TESTS=ON -DCMAKE_INSTALL_PREFIX=/home/username/gts/ ../
    $ make
    $ make install

__Windows (MinGW)__

    MKDIR build
    CD build
    cmake.exe -G"MinGW Makefiles" -DOpenCV_ROOT_DIR="C:\PATH-TO-OPENCV" ..   
    mingw32-make
    CD ..

__Windows (VS2010)__

	MKDIR build
	CD build
	cmake.exe -G"NMake Makefiles" -DOpenCV_ROOT_DIR="C:\PATH-TO-OPENCV ..
	nmake
	CD ..
	
__CMake Options__

- `OpenCV_ROOT_DIR` - Path to OpenCV libraries and includes (REQUIRED on Windows, looks in standard places on Linux)
- `GTS_TESTS` - Build Google Test Unit Tests (default=OFF).
- `GTS_HELP` - Build GTS Help Assistant (default=ON).
- `CMAKE_INSTALL_PREFIX` - Location for `make install` to place binary and help files.  

##Windows Installer##

To redistribute this application on Windows, an installer is available. You will first need to download the 2 additional files (Xvid and Microsoft VS2010 Redist package as linked in the [DOWNLOADS](/installer/files/DOWNLOADS) file.

__MinGW__
	
	MKDIR build
	CD build
	SET OpenCV_DIR="C:\Software\OpenCV-2.2.0-MinGW\opencv"
	cmake.exe -G"MinGW Makefiles" -DOpenCV_DIR="C:\Software\OpenCV-2.2.0-MinGW\opencv" ..
	mingw32-make package

__VS2010__
  	
	MKDIR build
	CD build
	cmake.exe -G"NMake Makefiles" -DOpenCV_ROOT_DIR="C:\PATH-TO-OPENCV" .. 
	nmake package
	CD ..

## Unit Tests ##

GTS uses the Google Test Framework for unit testing. A few tests have been included, but we hope to add to these over time and welcome any additions.

## Supported Cameras ##

The Unicap library on Linux and DirectShow on Windows are used to enumerate cameras. OpenCV is used for recording and tracking of videos.
Therefore cameras that are supported by these libraries should work on the GTS.

## Issues ##

Bugs and feature requests should be added to the Issues section of this repository. If you have a fix for such, please see below to have it considered to be merged in.

## Contributions ##

For contributions to be considered we require that users first read and follow the steps in the [CONTRIBUTING](CONTRIBUTING.md) file.

## Documentation ##

In addition to the __help__ tool that can be compiled alongside the app, the documentation can be found in this wiki.

## License ##

This application is distributed under the GPLv3 license found in the [LICENSE](LICENSE) file.

Attributions and licenses for third-party libraries can be found in the [OPEN SOURCE LICENSES](OPENSOURCE_LICENSES) file.
