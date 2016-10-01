MiniVideo TODO list
===================

TODO build system
-----------------

- doxygen hook?
- cppcheck hook?

TODO test softwares
-------------------

- error when an invalid path is passed as argument with the -i option
- error when an invalid path is passed as argument with the -o option
- add a non-regression/performance test suit
- a "mini_player" would definitely be an interesting thing to do

TODO video library
------------------

// Error codes handling
- replace remaining exit(EXIT_FAILURE) by callbacks
- there is no more error handling after intra_prediction_process()

// Bitstream
- behavior regarding unexpected end_of_file (currently it only trigger exit(EXIT_FAILURE))
- rename goto_offset_bitstream()
- better aligned memory reads for embedded platform
- big endian support probably doesn't work anymore

// File parsers
- MP4 parser (improvements)
  - progressive streaming support
- AVI parser (improvements)
  - global overhaul
- WAVE parser (improvements)
  - sample index
- MP3 parser (improvements)
  - sample index
- MPEG PS parser (improvements)
  - multiple frames per PES packet?
  - IDR detection
  - tag_descriptors
- MPEG TS parser
- MKV parser
- MXF parser (maybe)
- ASF parser (maybe)

// Video decoder
- CABAC bugfix
- reimplement transformbypass_decoding() to use static data table
- scaling_list extraction process common to SPS & PPS

PERFORMANCE IMPROVEMENTS
------------------------

// MACROBLOCKS
- Replace residual_block_cabac() AND residual_block_cavlc() calls by a single function pointer residual_block()
- Less call to deriv_neighbouringlocations()
- Smarter management of (mb->CodedBlockPatternLuma == 0 || mb->CodedBlockPatternChroma == 0) in macroblock_layer()
- No spatial transformations if no residual datas
- Quantization Parameters & TransformBypassModeFlag = only when mb_qp_delta is read?
- Replace iClip1_YCbCr_8() by Clip1_YCbCr_8()

// CABAC
- "intable" computation not done for each binIdx?
- Stop the memcmp() if we leave the decodedSE string
- Stop the memcmp() if match > 1

// TRANSFORMS
- Change some int u[][] to uint8 u[][]
- Homogenize the idct code for luma/chroma DC
- Reuse arrays within the idct code
  - idct4x4() use e[4][4] instead of g[4][4]
  - idct8x8() use e[8][8] instead of h[8][8] m[8][8] k[8][8]
