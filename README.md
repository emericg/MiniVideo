MiniVideo framework
===================

[![Build Status](https://travis-ci.org/emericg/MiniVideo.svg?branch=master)](https://travis-ci.org/emericg/MiniVideo)
[![Build status](https://ci.appveyor.com/api/projects/status/bt94ewnmw7bv8yab?svg=true)](https://ci.appveyor.com/project/emericg/minivideo)

Introduction
------------

MiniVideo is a **multimedia framework developed from scratch** in C/C++, bundled with small testing programs and a neat [media analyser](mini_analyser/).  
MiniVideo has been tested with several CPU architectures (x86, SH4, MIPS, ARM).  
The project uses a dual CMake/QMake build system (CMake is prefered though). Both library and test programs can be installed into your system.  

MiniVideo has been initially developed in 2010/2011 during an internship I did in a French company called *httv*, as a small **video decoding library developed from scratch** in C. Its goal was to generate video thumbnails, with a source code easy to read and to understand for learning purpose. After a clean-up pass, the code has been published early 2014 with *httv* permission under the LGPL v3 license (video framework) and GPLv3 (test softwares).  

The minivideo library can:
* Open video files with various container to demux and remux audios/videos content.
* Open H.264 compressed videos and decode/export intra-coded pictures.
* Extract various metadata from container and elementary streams.
* Map exact container structure to xml file / GUI.

### Supported video codec (decoding)
- H.264 / MPEG-4 part 10 "Advance Video Coding"
  - I frames only...
  - please note that at still a few bugs inside CABAC decoding process being worked on...

### Supported container formats (import modules)
- AVI [.avi]
- WAVE [.wav]
- ASF [.asf, .wma, .wmv]
- MKV [.mkv, .webm, ...]
- MP4 / MOV (ISOM container) [.mp4, .mov, .3gp, ...]
- MPEG-PS (MPEG "Program Stream") [.mpg, .mpeg, .vob, ...]
- MPEG-1/2 "elementary stream" [.mpg, .mpeg]
- H.264 / H.265 "elementary stream" ("Annex B" format) [.264, .265]
- MP3 "elementary stream" [.mp3]

### Supported container formats (export modules)
- Elementary Streams

### Supported picture formats (output modules)
- jpeg (internal OR when libjpeg support is available)
- png (internal OR when libpng support is available)
- webp (when libwebp support is available)
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


Building MiniVideo's testing softwares
--------------------------------------

Do not forget "FindLibMiniVideo.cmake" directory in the cmake/modules/, which defines
how to find the library (libminivideoframework.so file) and its header (minivideoframework.h file)
In case of problem, it may be necessary to manually indicate the paths of these files.

> $ cd mini_analyser/  
> $ qmake  
> $ make  

> $ cd mini_extractor/build/  
> $ cmake ..  
> $ make  

> $ cd mini_thumbnailer/build/  
> $ cmake ..  
> $ make  

Installation into the system, available for root user with both testing softwares:
>  $ su  
>  **#** make install # INSTALLATION INTO THE SYSTEM, ROOT USER ONLY  


Using mini_analyser
-------------------

> $ cd mini_analyser/build/  
> $ ./mini_analyser  

Then drag and drop files to analyse them!


Using mini_extractor
--------------------

> $ cd mini_extractor/build/  
> $ ./mini_extractor -i 'myfilepath' [-o 'mydirectory'] [-a nb_tracks] [-v nb_tracks]  

Command line arguments:
> -h : print help  
> -i : path to the input video  
> -o : path to the output folder, where extracted streams will be saved  
> -a : maximum number of audio stream(s) to extract  
> -v : maximum number of video stream(s) to extract  


Using mini_thumbnailer
----------------------

> $ cd mini_thumbnailer/build/  
> $ ./mini_thumbnailer -i 'myfilepath' [-o 'mydirectory'] [-f picture_format] [-q picture_quality] [-n picture_number] [-m picture_extractionmode]  

Command line arguments:
> -h : print help  
> -i : path to the input video  
> -o : path to the output folder, where generated thumbnails will be saved  
> -f : export format for the thumbnails (can be 'webp' 'jpg' 'png' 'bmp' 'tga' 'yuv420' 'yuv444')  
> -q : thumbnail quality (1 to 100 range)  
> -n : number of thumbnail to generate (1 to 999 range)  
> -m : extraction mode for the thumbnails (can be 'unfiltered', 'ordered' or 'distributed')  
