#-------------------------------------------------------------------------------
# MiniVideo framework build system
# minivideo library must be built first in order for the associated tools to work
#-------------------------------------------------------------------------------

TEMPLATE = subdirs
SUBDIRS = minivideo mini_thumbnailer mini_extractor mini_analyser

mini_thumbnailer.depends = minivideo
mini_extractor.depends = minivideo
mini_analyser.depends = minivideo
