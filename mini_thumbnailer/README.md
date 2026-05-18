mini_thumbnailer
================

[![License: GPL v3](https://img.shields.io/badge/license-GPL%20v3-brightgreen.svg?style=flat-square)](http://www.gnu.org/licenses/gpl-3.0)

Introduction
------------

mini_thumbnailer is a thumbnail picture extraction software.


Building mini_thumbnailer
-------------------------

> minivideo library must have been built first!

```bash
$ cd mini_thumbnailer/
$ cmake -B build/ -DMiniVideo_ROOT=/path/to/minivideo/
$ cmake --build build/
```

System wide installation:

```bash
$ cd mini_thumbnailer/
$ cmake -B build/ -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DMiniVideo_ROOT=/path/to/minivideo/
$ cmake --build build/
# cmake --install build/
```


Using mini_thumbnailer
----------------------

```bash
$ cd mini_thumbnailer/bin/
$ ./mini_thumbnailer -i 'myfilepath' [-o 'mydirectory'] [-f picture_format] [-q picture_quality] [-n picture_number] [-m picture_extractionmode]
```

Command line arguments:
> -h : print help  
> -i : path to the input video  
> -o : path to the output folder, where generated thumbnails will be saved  
> -f : export format for the thumbnails (can be 'webp' 'jpg' 'png' 'bmp' 'tga' 'yuv420' 'yuv444')  
> -q : thumbnail quality (1 to 100 range)  
> -n : number of thumbnail to generate (1 to 999 range)  
> -m : extraction mode for the thumbnails (can be 'unfiltered', 'ordered' or 'distributed')  
