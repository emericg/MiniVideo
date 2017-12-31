MiniVideo library TODO list
===========================

TODOs
-----

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
- MPEG PS parser (improvements)
  - multiple frames per PES packet?
  - IDR detection
  - tag_descriptors
- MPEG TS parser
- Ogg parser
- MXF parser (maybe)

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
- Change some int u[][] to uint8 u[][]?
- Homogenize the idct code for luma/chroma DC
