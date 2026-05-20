mini_analyser
=============

[![GitHub action](https://img.shields.io/github/actions/workflow/status/emericg/MiniVideo/builds_minianalyser.yml?style=flat-square)](https://github.com/emericg/MiniVideo/actions/workflows/builds_minianalyser.yml)
[![License: GPL v3](https://img.shields.io/badge/license-GPL%20v3-brightgreen.svg?style=flat-square)](http://www.gnu.org/licenses/gpl-3.0)

mini_analyser is a software designed to help you extract a maximum of informations and metadata from multimedia files. It can also map container's internal structure and let you visualize it.

### Supported container formats
- AVI [.avi]
- WAVE [.wav]
- ASF [.asf, .wma, .wmv]
- MKV [.mkv, .webm, ...]
- MP4 / MOV (ISOM container) [.mp4, .mov, .3gp, ...]
- MPEG-PS (MPEG "Program Stream") [.mpg, .mpeg, .vob, ...]
- MPEG-1/2 "elementary stream" [.mpg, .mpeg]
- H.264 / H.265 / H.266 "elementary stream" ("Annex B" format) [.264, .265, .266]
- MP3 "elementary stream" [.mp3]

### Screenshots

![main screen](https://i.imgur.com/kDJ6NQx.png)
![video screen](https://i.imgur.com/iuAl85j.png)
![container explorer](https://i.imgur.com/cGtqXPu.png)
![HW decoding checker](https://i.imgur.com/0qGcZxR.png)


## Documentation

### Dependencies

- Qt 6.5+
- minivideo

### Building mini_analyser

> minivideo library must have been built first!

```bash
$ cd mini_analyser/
$ cmake -B build/ -DMiniVideo_ROOT=/path/to/minivideo/
$ cmake --build build/
```

### Using mini_analyser GUI

Just drag and drop multimedia files to analyse them!

### Using mini_analyser CLI

> $ cd mini_analyser/bin/  
> $ ./mini_analyser --cli [--details] /path/to/files

### Third party projects used by mini_analyser

* Qt6 [website](https://www.qt.io) ([LGPL 3](https://www.gnu.org/licenses/lgpl-3.0.txt))
* MiniVideo [website](https://github.com/emericg/MiniVideo) ([LGPL 3](https://www.gnu.org/licenses/lgpl-3.0.txt))
* pugixml [website](https://pugixml.org/) ([MIT](https://opensource.org/licenses/MIT))
* QHexView [website](https://github.com/Dax89/QHexView) ([MIT](https://opensource.org/licenses/MIT))
* QHexEdit2 [website](https://github.com/Simsys/qhexedit2) ([LGPL 2.1+](https://www.gnu.org/licenses/lgpl-2.1.txt))
* QCustomPlot [website](https://www.qcustomplot.com/) ([GPL 3](https://www.gnu.org/licenses/gpl-3.0.txt))
* Graphical resources: [assets/COPYING](assets/COPYING)


## Get involved!

### Developers

You can browse the code on the GitHub page, submit patches and pull requests! Your help would be greatly appreciated ;-)

### Users

You can help us find and report bugs, suggest new features, help with translation, documentation and more! Visit the Issues section of the GitHub page to start!


## License

mini_analyser is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.  
Read the [LICENSE](LICENSE.md) file or [consult the license on the FSF website](https://www.gnu.org/licenses/gpl-3.0.txt) directly.

> Emeric Grange <emeric.grange@gmail.com>
