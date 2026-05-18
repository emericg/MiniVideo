MiniVideo library
=================

[![GitHub action](https://img.shields.io/github/actions/workflow/status/emericg/MiniVideo/builds_minivideo.yml?style=flat-square)](https://github.com/emericg/MiniVideo/actions/workflows/builds_minivideo.yml)
[![License: LGPL v3](https://img.shields.io/badge/license-LGPL%20v3-brightgreen.svg?style=flat-square)](http://www.gnu.org/licenses/lgpl-3.0)

## Introduction

The minivideo library can:
* Open video files with various container to demux and remux audio/video content.
* Open H.264 compressed streams and decode & export intra-coded pictures.
* Extract various metadata from container and elementary streams.
* Map exact container structure to XML file / GUI.

### Supported container formats (import modules)
- AVI [.avi]
- WAVE [.wav]
- ASF [.asf, .wma, .wmv]
- MKV [.mkv, .webm, ...]
- MP4 / MOV (ISOM container) [.mp4, .mov, .3gp, ...]
- MPEG-PS (MPEG "Program Stream") [.mpg, .mpeg, .vob, ...]
- MPEG-1/2 "elementary stream" [.mpg, .mpeg]
- H.264 / H.265  / H.266 "elementary stream" ("Annex B" format) [.264, .265, .266]
- MP3 "elementary stream" [.mp3]

### Supported container formats (export modules)
- Elementary Streams

### Supported video codec (decoding)
- H.264 / MPEG-4 part 10 "Advance Video Coding"
  - I frames only...
  - A few bugs inside the CABAC decoding process are still being worked on...

### Supported picture formats (decoding output modules)
- jpeg (internal OR when libjpeg support is available)
- png (internal OR when libpng support is available)
- webp (when libwebp support is available)
- bmp
- tiff
- tga


## Documentation

### Building libminivideo

Build for development:

```bash
$ cd minivideo/
$ cmake -B build/ -DMINIVIDEO_BUILD_SHARED:BOOL=ON -DMINIVIDEO_BUILD_STATIC:BOOL=ON -DCMAKE_INSTALL_PREFIX=bin/
$ cmake --build build/
```

Build for release and installation:

```bash
$ cd minivideo/
$ cmake -B build/ -DCMAKE_BUILD_TYPE=Release -DMINIVIDEO_BUILD_SHARED:BOOL=ON -DMINIVIDEO_BUILD_STATIC:BOOL=OFF -DCMAKE_INSTALL_PREFIX=/usr
$ cmake --build build/ --config Release
$ cmake --install build/
```

Note: You can tune the build by using some options:
> -DMINIVIDEO_BUILD_SHARED:BOOL=ON  
> -DMINIVIDEO_BUILD_STATIC:BOOL=OFF  
> -DCMAKE_BUILD_TYPE=Release/Debug  
> -DCMAKE_INSTALL_PREFIX=/usr/bin  
> -DCMAKE_TOOLCHAIN_FILE=custom/cmake/toolchains/toolchain.cmake  

Note: Use ninja for faster builds:
> $ cmake -B build/ -G Ninja

Note: You can also change many build options directly in the `minivideo/CMakeLists.txt` file.


### Generating online documentation with Doxygen

```bash
$ cd minivideo/doc/
$ ./generate_doxygen.sh
```

Open the `minivideo/doc/doxygen.html` file with your favorite browser.

### Generating error report with cppcheck

```bash
$ cd minivideo/doc/
$ ./generate_cppcheck.sh
```

Open the `minivideo/doc/cppcheck.html` file with your favorite browser.


## MiniVideo decoding capabilities

### H.264 unsupported features

- I frames only...

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


## Get involved!

### Developers

You can browse the code on the GitHub page, submit patches and pull requests! Your help would be greatly appreciated ;-)

### Users

You can help us find and report bugs, suggest new features, help with translation, documentation and more! Visit the Issues section of the GitHub page to start!


## License

MiniVideo is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.  
Read the [LICENSE](LICENSE.md) file or [consult the license on the FSF website](https://www.gnu.org/licenses/lgpl-3.0.txt) directly.

> Emeric Grange <emeric.grange@gmail.com>
