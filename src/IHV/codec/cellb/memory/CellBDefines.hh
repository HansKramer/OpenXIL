/***********************************************************************


            EXHIBIT A - XIL 1.4.1 (OPEN SOURCE VERSION) License


The contents of this file are subject to the XIL 1.4.1 (Open Source
Version) License Agreement Version 1.0 (the "License").  You may not
use this file except in compliance with the License.  You may obtain a
copy of the License at:

    http://www.sun.com/software/imaging/XIL/xilsrc.html

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
the License for the specific language governing rights and limitations
under the License.

The Original Code is XIL 1.4.1 (Open Source Version).
The Initial Developer of the Original Code is: Sun Microsystems, Inc..
Portions created by:_______________________________________________
are Copyright(C):__________________________________________________
All Rights Reserved.
Contributor(s):____________________________________________________


***********************************************************************/

//------------------------------------------------------------------------
//
//  File:   CellBDefines.hh
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:23:15, 03/10/00
//
//  Description:
//
//
//
//
//
//
//
//
//  MT-level:  <??????>
//
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)CellBDefines.hh	1.2\t00/03/10  "

#ifndef CELLBDEFINES_HH
#define CELLBDEFINES_HH

//     Definition and Description of CellB Bytestream.
// 
//     Revision 0.1
//     ------------
// 
//     CellB is an image compression technology designed to
//     minimize the computational requirements for compression
//     and decompression. It is descended from a compression
//     technology known as Cell. The two primary differences
//     between Cell and CellB are the simplification of the
//     compression process and the ability of CellB to generate
//     high quality output using a fixed colormap.
// 
//     The Cell and CellB byte streams are so similar that they
//     could conceivably be merged into a single bytestream
//     defintion. While there is some merit in combining the
//     two bytestream definitions, Cell and CellB into a single
//     definition, the primary impact would be on the definition
//     of the existing Cell. Since the objective of here is to
//     describe the CellB bytestream, the approach which has been
//     taken describes CellB independent of the original Cell
//     definition. Anyone interested in a true merger will have
//     to refer elsewhere.
// 
//     The lowest level definiton of CellB is comprised of a
//     bitstream definiton and set of algorithms which can be
//     used to generate the components of the bitstream. The
//     bitstream is described here in detail. The algorithms
//     are discussed at a high level with references to sections
//     of the code where the algorithm is implemented.
// 
//     The smallest meaningful unit of the CellB bitstream is
//     is an 8-bit octet or byte. All CellB bitstreams are
//     composed of an integral number of bytes, and it is
//     therefore referred to as a byte stream. All legal sequences
//     of bytes are considered to be "bytecodes". The bytestream
//     is composed of a variable length string of bytecodes.
// 
//     All byte codes describe one or more 4x4 tiles, known as
//     cells, within an image. As a result the bytestream is only
//     capable of describing images which are composed of an integral
//     number of tiles. Also, it is generally assumed that cells
//     are encoded within the bytestream in a scanline order;
//     from left to right and from top to bottom.
// 
//     The first bytecode defined is the "Normal Cell" code.
//     The format of this code is:
// 
//     0 M M M M M M M        Byte N
//     M M M M M M M M        Byte N+1
//     [   U/V code  ]        Byte N+2
//     [   Y/Y code  ]        Byte N+3
// 
//     The U/V field of the Normal Cell bytecode represents the
//     chrominance component of the cell. This U/V code is actually
//     an index into a table of vectors which represent two independent
//     components of chrominance. The table used in this example encoder
//     can be found in uvtable.cc. This table is generated using Vector
//     Quantization techniques. In the example implementation the table
//     is statistically based and constant over all input images. Our
//     example table contains more representatives when U and V are
//     near zero than when U and V greater. This U/V code is determined
//     by calculating the mean values for U and V (see encode_cell( ))
//     and using the most significant 6 bit of U and V to address a
//     quantizer table (uvremap[64][64]). As mentioned earlier the
//     quantized u/v value is determined by the decoder also by
//     performed via a table loop-up (uvtable[252]).
//
//     NOTE: these tables are considered to be a part of the Bitstream
//     definition, even thought they are not described here.
// 
//     The Y/Y field of the Normal Cell bytecode represents two
//     luminance values which are used to reconstruct the 4 x 4
//     cell.  This Y/Y value is also an index into a table of
//     two-component vectors. The current compressor implementation
//     employs a quantizer which is both statistically and perceptually
//     optimized. In the current implementation the quantizer is
//     constant for all images which are encoded. This quantizer
//     takes advantage of the high correlation of luminance values
//     within local regions, resulting in a set of representative
//     vectors which are most densely populated around the diagonal
//     (where y1 == y2). The visual systems sensitivity to contrast in
//     luminance is also taken into consideration, resulting in a
//     distribution of representatives which are spaced further
//     apart in regions where the contrast between the two values
//     s low. The resulting constant quantization tables which are
//     used in this first implementation are defined in the file
//     yytable.cc.
// 
//     In this prototype implementation the value for the Y/Y field
//     is selected by the following process. For each tile an
//     approximation to the mean luminance for the cell is first
//     computed. Next the pixels within the cell are partitioned into
//     two groups those who fall above the mean, and those who fall
//     below it. The mean of these partitions are used to index a two
//     dimensional vector quantizer which returns a byte value for the
//     Y/Y field.
// 
//     The remaining field of a Normal Cell bytecode corresponds to
//     a bit mask. This mask is generated by setting or clearing a
//     bit corresponding to each pixel in the 4 x 4 cell. The mask
//     is normalized so that the first bit within the mask is always
//     corresponds to the first component of the Y/Y vector. Therefore,
//     the first bit of a mask is always cleared to 0. If this first
//     pixel is above the cell mean, then other pixels in the cell
//     which are above the mean will also be cleared while those below
//     will be set to a 1. However, If the first pixel falls below
//     the mean then other pixels which fall below the mean will be
//     cleared while the remainer are set. The example encoder performs
//     this task in encode_cell( ).
// 
//     The following diagram shows the relationship between bits in
//     the mask and pixels within a 4 x 4 cell.
// 
//         Byte N        Byte N+1
//     +-+-+-+-+-+-+-+-+   +-+-+-+-+-+-+-+-+
//     | |M|M|M|M|M|M|M|   |M|M|M|M|M|M|M|M|
//     |0|1|1|1|1|1|0|0|   |0|0|0|0|0|0|0|0|
//     | |4|3|2|1|0|9|8|   |7|6|5|4|3|2|1|0|
//     +-+-+-+-+-+-+-+-+   +-+-+-+-+-+-+-+-+
// 
// 
//         +---+---+---+---+
//         | 0 |M14|M13|M12|
//         +---+---+---+---+
//         |M11|M10|M09|M08|
//         +---+---+---+---+
//         |M07|M06|M05|M04|
//         +---+---+---+---+
//         |M03|M02|M01|M00|
//         +---+---+---+---+
// 
// 
//     The second bytecode which has been defined in the 0.1 revision
//     of the CellB bytestream is a Skip bytecode. It has the following
//     format:
// 
//         1 0 0 S S S S S        Byte N
// 
//     The Skip bytecode instructs the decoder to skip the next S+1
//     cells in the current frame being decoded. This code can be
//     used to implement a simple form of inter-frame encoding.
//     The prototype implementation introduces Skip bytecodes when
//     the difference between the current value of a cell is close
//     to the value of the cell which occurred in the same spatial
//     position in the previous frame. Runs of up to 32 skip codes
//     are combined into a single bytecode. The heuristic used for
//     closeness is in the skipCell( ) function.  The heuristic in
//     this implementation also considers the possibility that
//     byte-code aligned fragments of the bytestream might be lost
//     or ignored by the decoder. It guarantees that each cell will
//     be transmitted at least once every 17 frames, and it trys
//     to distribute these update over the interval evenly by randomly
//     updating cells with a known probability. Closeness is determined
//     by first testing if the chrominance is within tolerance, and
//     then calculating the absolute luminance error between the
//     current and the previous cell. If this error is found to be
//     within an established tolerance (144) the cell is skipped.
// 
//  TODO: the bytestream redundancy issue may need to be configurable
//     through attributes.
//  NOTE: the actual bitstream definition should specify a maximum
//     for retransmittal (132 like H.261 ?).  The 17 value is just
//     the current implementation.
// 
//     The final byte code which has been defined but is not used
//     by the current implementation. It is the Sync bytecode which
//     has the following format:
// 
//         1 1 1 1 1 1 1 1        Byte N
//         1 1 1 1 1 1 1 1        Byte N+1
//         1 1 1 1 1 1 1 1        Byte N+2
//         1 1 1 1 1 1 1 1        Byte N+3
// 
//     This code can be periodically introduced in a bytestream
//     which in the case where the underlying transport mechanism
//     is an lossless unbuffered stream. This code allows a decoder
//     to join in a conference in progress, by reading from the stream
//     until it encounters the first valid sync bytecode and then
//     proceeding to the first non-0xff byte at that point the
//     decoder can proceed to decoding as normal. The existence
//     and frequency of sync bytecodes is encoder dependent, and
//     as mentioned earlier, its necessity is transport dependent.
//
//    TODO: add use of sync bytecode to current encoder.


const Xil_unsigned8   CELLB_MAX_SKIP          = 32;
const Xil_unsigned8   SKIPCODE                = 0x80;

//
//  This defines the size of each buffer in the Cis Buffer Manager.
//  I have chosen 10 because that's what Cell did.
//  (TODO: should this be adjusted?)
//
const int          FRAMES_PER_BUFFER   = 10;

//  These constants are used in figuring out the size of the various
//  tables used in precomputing values.
const int          BITS_IN_CELL_PLUS_ONE = 17;

//  I'm not sure where the 252 below is derived from.
const int          UVTABLE_SIZE = 252;
const int          MAX_BYTE_VAL = 256;


#endif // CELLBDEFINES_HH

