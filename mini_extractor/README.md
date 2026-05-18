mini_extractor
==============

[![License: GPL v3](https://img.shields.io/badge/license-GPL%20v3-brightgreen.svg?style=flat-square)](http://www.gnu.org/licenses/gpl-3.0)

Introduction
------------

mini_extractor is an elementary stream extractor software.


Building mini_extractor
-----------------------

> minivideo library must have been built first!

```bash
$ cd mini_extractor/
$ cmake -B build/ -DMiniVideo_ROOT=/path/to/minivideo/
$ cmake --build build/
```

System wide installation:

```bash
$ cd mini_extractor/
$ cmake -B build/ -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DMiniVideo_ROOT=/path/to/minivideo/
$ cmake --build build/
# cmake --install build/
```


Using mini_extractor
--------------------

```bash
$ cd mini_extractor/bin/
$ ./mini_extractor -i 'myfilepath' [-o 'mydirectory'] [-a nb_tracks] [-v nb_tracks]
```

Command line arguments:
> -h : print help  
> -i : path to the input video  
> -o : path to the output folder, where extracted streams will be saved  
> -a : maximum number of audio stream(s) to extract  
> -v : maximum number of video stream(s) to extract  
