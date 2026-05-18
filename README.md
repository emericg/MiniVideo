MiniVideo framework
===================

[![GitHub action](https://img.shields.io/github/actions/workflow/status/emericg/MiniVideo/builds_minivideo.yml?style=flat-square)](https://github.com/emericg/MiniVideo/actions/workflows/builds_minivideo.yml)
[![GitHub action](https://img.shields.io/github/actions/workflow/status/emericg/MiniVideo/builds_minianalyser.yml?style=flat-square)](https://github.com/emericg/MiniVideo/actions/workflows/builds_minianalyser.yml)

MiniVideo is a **multimedia framework developed from scratch** in C/C++, bundled with small testing programs and a neat [media analyser](mini_analyser/).  
MiniVideo has been tested with several CPU architectures (x86, SH4, MIPS, ARM).  
The project uses a CMake build system. Both library and test programs can be installed into your system.  

MiniVideo has been initially developed in 2010/2011 during an internship I did in a French company called *httv*, as a small **video decoding library developed from scratch** in C.
Its goal was to generate video thumbnails, with a source code easy to read and to understand for learning purpose.
After a clean-up pass, the code has been published early 2014 with *httv* permission under the LGPL v3 license (video framework) and GPLv3 (test softwares).  

The minivideo library can:
* Open video files with various container to demux and remux audio/video content.
* Open H.264 compressed streams and decode & export intra-coded pictures.
* Extract various metadata from container and elementary streams.
* Map exact container structure to XML file / GUI.


MiniVideo library features
--------------------------

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


MiniVideo library
-----------------

Check out [minivideo](minivideo) documentation!  


MiniVideo framework included softwares
--------------------------------------

[mini_analyser](mini_analyser) is a GUI software designed to help you extract a maximum of informations and metadata from multimedia files.  
It can also map container's internal structure and let you visualize it.  

[mini_extractor](mini_extractor) is an elementary stream extractor software.  

[mini_thumbnailer](mini_thumbnailer) is a thumbnail picture extraction software.  
