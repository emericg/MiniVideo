mini_extractor
==============

[![License: GPL v3](https://img.shields.io/badge/license-GPL%20v3-brightgreen.svg?style=flat-square)](http://www.gnu.org/licenses/gpl-3.0)

Introduction
------------

mini_extractor is an elementary stream extractor software.


Building mini_extractor
-----------------------

Do not forget "FindLibMiniVideo.cmake" directory in the cmake/modules/, which defines
how to find the library (libminivideo.so file) and its header (minivideo.h file)
In case of problem, it may be necessary to manually indicate the paths of these files.

minivideo library must have been built first!

> $ cd mini_extractor/build/  
> $ cmake ..  
> $ make  

Installation into the system, available for root user with both testing softwares:
>  $ su  
>  **#** make install # INSTALLATION INTO THE SYSTEM, ROOT USER ONLY  


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
