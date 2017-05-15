MiniVideo library
=================

[![Build Status](https://travis-ci.org/emericg/MiniVideo.svg?branch=master)](https://travis-ci.org/emericg/MiniVideo)
[![Build status](https://ci.appveyor.com/api/projects/status/bt94ewnmw7bv8yab?svg=true)](https://ci.appveyor.com/project/emericg/minivideo)
[![License: LGPL v3](https://img.shields.io/badge/license-LGPL%20v3-brightgreen.svg)](http://www.gnu.org/licenses/lgpl-3.0)

Introduction
------------

The minivideo library can:
* Open video files with various container to demux and remux audios/videos content.
* Open H.264 compressed videos and decode them to export intra-coded pictures.
* Extract various metadatas from container and elementary streams.
* Map exact container structure to xml file.

### Supported video codec (decoding)
- H.264 / MPEG-4 part 10 "Advance Video Coding"
  - please note that at still a few bugs inside CABAC decoding process being worked on...

### Supported container formats (import modules)
- AVI [.avi]
- WAVE [.wav]
- MKV [.mkv, .webm, ...]
- MP4 / MOV (ISOM container) [.mp4, .mov, .3gp, ...]
- MPEG-PS (MPEG "Program Stream") [.mpg, .mpeg, .vob, ...]
- MPEG-1/2 "elementary stream" [.mpg, .mpeg]
- H.264 / H.265 "elementary stream" ("Annex B" format) [.264, .265]
- MP3 "elementary stream" (.mp3)

### Supported container formats (export modules)
- Elementary Streams
- MPEG-PS (MPEG "Program Stream") [.mpg, .mpeg, .vob, ...]

### Supported picture formats (output modules)
- webp (when libwebp support available)
- jpeg (when libjpeg support is available)
- png (internal OR when libpng support available)
- bmp
- tiff
- tga


Building minivideo library
--------------------------

> $ cd minivideo/build/  
> $ cmake ..  
> $ make  

Note: You can easily enable multithreaded build with the "make -jX" argument:
> $ make -j$(grep -c ^processor /proc/cpuinfo)  

Note: You can tune CMake by adding extra arguments:
> -DCMAKE_BUILD_TYPE=Release/Debug  
> -DCMAKE_BUILD_Mode=Dynamic/Static  
> -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/xxx.cmake  
> -DCMAKE_INSTALL_PREFIX=/usr/bin  

Note: You can also change several build options directly into the "minivideo/CMakeLists.txt" file.

Installation into the system, available for root user:
>  $ su  
>  **#** make install # INSTALLATION INTO THE SYSTEM, ROOT USER ONLY  


Generating online documentation with Doxygen
--------------------------------------------

> $ cd minivideo/doc/  
> $ ./generate_doxygen.sh  

Open "minivideo/doc/doxygen.html" with your favorite browser.


Generating error report with cppcheck
-------------------------------------

> $ cd minivideo/doc/  
> $ ./generate_cppcheck.sh  

Open "minivideo/doc/cppcheck.html" with your favorite browser.


MiniVideo decoding capabilities
===============================

H.264 unsupported features
--------------------------

// UNSUPPORTED for BP and XP profiles
- (FMO) Flexible Macroblock Ordering
- (ASO) Arbitrary slice ordering
- (RS) Redundant slice

// UNSUPPORTED for XP profile
- Data partitioning
- SI and SP slices

// UNSUPPORTED for HiP profile ("HIGH")
- CABAC decoding process still has a few bugs (WIP)
- No deblocking filter
- Interlaced coding (also PicAFF and MBAFF features)
- 4:0:0 "greyscale" subsampling

// UNSUPPORTED for Hi10P profile ("HIGH + 10bits samples")
- Sample depths > 8 bits

// UNSUPPORTED for Hi422P profile ("HIGH + 10bits samples + 4:2:2 subsampling")
- Sample depths > 8 bits
- 4:2:2 subsampling

// UNSUPPORTED for Hi444P profile ("HIGH + 14bits samples + 4:4:4 subsampling")
- Sample depths > 10 bits
- 4:4:4 subsampling
- Separate color plane coding
- IPCM macroblocks
