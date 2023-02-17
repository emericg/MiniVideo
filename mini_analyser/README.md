mini_analyser
=============

[![GitHub action](https://img.shields.io/github/actions/workflow/status/emericg/MiniVideo/builds_minianalyser.yml?style=flat-square)](https://github.com/emericg/MiniVideo/actions/workflows/builds_minianalyser.yml)
[![License: GPL v3](https://img.shields.io/badge/license-GPL%20v3-brightgreen.svg?style=flat-square)](http://www.gnu.org/licenses/gpl-3.0)

Introduction
------------

mini_analyser is a software designed to help you extract a maximum of informations and metadata from multimedia files. It can also map container's internal structure and let you visualize it.

### Supported container formats
- AVI [.avi]
- WAVE [.wav]
- ASF [.asf, .wma, .wmv]
- MKV [.mkv, .webm, ...]
- MP4 / MOV (ISOM container) [.mp4, .mov, .3gp, ...]
- MPEG-PS (MPEG "Program Stream") [.mpg, .mpeg, .vob, ...]
- MPEG-1/2 "elementary stream" [.mpg, .mpeg]
- H.264 / H.265 "elementary stream" ("Annex B" format) [.264, .265]
- MP3 "elementary stream" [.mp3]

![main screen](https://i.imgur.com/kDJ6NQx.png)
![video screen](https://i.imgur.com/iuAl85j.png)
![container explorer](https://i.imgur.com/cGtqXPu.png)
![HW decoding checker](https://i.imgur.com/0qGcZxR.png)


Building mini_analyser
----------------------

minivideo library must have been built first!

You can either use CMake (recommanded, more configurable)
> $ cd minivideo/build  
> $ cmake ..  
> $ make  

Or qmake (simplier)
> $ cd minivideo/  
> $ qmake  
> $ make  

Then:
> $ cd mini_analyser/  
> $ qmake  
> $ make  


Using mini_analyser GUI
-----------------------

Just drag and drop multimedia files to analyse them!


Using mini_analyser CLI
-----------------------

> $ cd mini_analyser/bin/  
> $ ./mini_analyser --cli [--details] /path/to/files
