MiniVideo
=========


Introduction
------------

MiniVideo is a small **video decoding library developed from scratch** in C, bundled with small testing programs developed in C++.  
MiniVideo has been tested with several CPU architectures (x86, SH4, MIPS, ARM).  
The build system uses CMake. Both library and test programs can be installed into your system.  

MiniVideo has been developed in 2010/2011 during an internship I did in a French company called *httv*. Its goal was to provide a video thumbnailing program, with a source code easy to read and to understand for learning purpose. After a clean-up pass, the code has been published early 2014 with *httv* permission under the LGPL v3 license (decoding backend) and GPLv3 (testing softwares).  

The minivideo library can:
* Open video files with various container to demux and remux audios/videos content.
* Open H.264 video stream and decode them to export intra-coded pictures.
* Extract various metadatas from container and elementary stream.

### Supported video codec (decoding)
- H.264 / MPEG-4 part 10 "Advance Video Coding"
  - please note that at still a few bugs inside CABAC decoding process being worked on...

### Supported container formats (import module)
- H.264 "elementary stream" ("Annex B" format) [.264]
- MPEG-1/2 "elementary stream" [.mpg, .mpeg]
- MPEG-PS (MPEG Program Stream) [.mpg, .mpeg, .vob, ...]
- MP4 (MPEG-4 ISO container) [.mp4, .mov, .3gp, ...]
- AVI [.avi]

### Supported picture formats (export module)
- jpeg (only if external libjpeg support is available)
- png (external libpng support available)
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
>  # make install # INSTALLATION INTO THE SYSTEM, ROOT USER ONLY  


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
>  # make install # INSTALLATION INTO THE SYSTEM, ROOT USER ONLY  


Using mini_analyser
-------------------

> $ cd mini_analyser/build/  
> $ export LD_LIBRARY_PATH=../../minivideo/build
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
> -f : export format for the thumbnails (can be 'jpg' 'png' 'bmp' 'tga' 'yuv420' 'yuv444')  
> -q : thumbnail quality (1 to 100 range)  
> -n : number of thumbnail to generate (1 to 999 range)  
> -m : extraction mode for the thumbnails (can be 'unfiltered', 'ordered' or 'distributed')  


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
