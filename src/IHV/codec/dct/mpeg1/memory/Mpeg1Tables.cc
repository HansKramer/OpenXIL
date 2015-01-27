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
//This line lets emacs recognize this as -*- C++ -*- Code
//------------------------------------------------------------------------
//
//  File:       Mpeg1Tables.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:14:43, 03/10/00
//
//  Description:
//
//    Tables for Mpeg decoding
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Mpeg1Tables.cc	1.2\t00/03/10  "

#include "Mpeg1Decoder.hh"
#include "Mpeg1DecompressorData.hh"


// 
// Quantization table
//
Xil_unsigned8 Mpeg1DecompressorData::quantIntraInit[64] =
{
    8, 16, 16, 19, 16, 19, 22, 22,
    22, 22, 22, 22, 26, 24, 26, 27,
    27, 27, 26, 26, 26, 26, 27, 27,
    27, 29, 29, 29, 34, 34, 34, 29,
    29, 29, 27, 27, 29, 29, 32, 32,
    34, 34, 37, 38, 37, 35, 35, 34,
    35, 38, 38, 40, 40, 40, 48, 48,
    46, 46, 56, 56, 58, 69, 69, 83,
};


//
// Macroblock Address Increment Table.
//
// Indicates the address increment between successive macroblocks.
// The ESCAPE cide is used for increments greater than 33.
// The STUFF code can be inserted to increase bit rate if necessary
// to match channel transmission or storage requirements.
//
Codes Mpeg1Decoder::MBAtable[35] = 
{
    { 0x0001,  1,  1},
    { 0x0003,  3,  2},
    { 0x0002,  3,  3},
    { 0x0003,  4,  4},
    { 0x0002,  4,  5},
    { 0x0003,  5,  6},
    { 0x0002,  5,  7},
    { 0x0007,  7,  8},
    { 0x0006,  7,  9},
    { 0x000b,  8, 10},
    { 0x000a,  8, 11},
    { 0x0009,  8, 12},
    { 0x0008,  8, 13},
    { 0x0007,  8, 14},
    { 0x0006,  8, 15},
    { 0x0017, 10, 16},
    { 0x0016, 10, 17},
    { 0x0015, 10, 18},
    { 0x0014, 10, 19},
    { 0x0013, 10, 20},
    { 0x0012, 10, 21},
    { 0x0023, 11, 22},
    { 0x0022, 11, 23},
    { 0x0021, 11, 24},
    { 0x0020, 11, 25},
    { 0x001f, 11, 26},
    { 0x001e, 11, 27},
    { 0x001d, 11, 28},
    { 0x001c, 11, 29},
    { 0x001b, 11, 30},
    { 0x001a, 11, 31},
    { 0x0019, 11, 32},
    { 0x0018, 11, 33},
    { 0x0008, 11, ESCAPE},
    { 0x000f, 11, STUFF}
};

//
// Macroblock Type Code (I-Frames)
//
Codes Mpeg1Decoder::MBTItable[2] =
{
    { 0x0001, 1,  0},
    { 0x0001, 2,  1},
};

//
// Macroblock Type Code (P-Frames)
//
Codes Mpeg1Decoder::MBTPtable[7] =
{
    { 0x0001, 1,  0},
    { 0x0001, 2,  1},
    { 0x0001, 3,  2},
    { 0x0003, 5,  3},
    { 0x0002, 5,  4},
    { 0x0001, 5,  5},
    { 0x0001, 6,  6},
};

//
// Macroblock Type Code (B-Frames)
//
Codes Mpeg1Decoder::MBTBtable[11] =
{
    { 0x0002, 2,  0},
    { 0x0003, 2,  1},
    { 0x0002, 3,  2},
    { 0x0003, 3,  3},
    { 0x0002, 4,  4},
    { 0x0003, 4,  5},
    { 0x0003, 5,  6},
    { 0x0002, 5,  7},
    { 0x0003, 6,  8},
    { 0x0002, 6,  9},
    { 0x0001, 6,  10},
};

//
// Codes for dct_dc_size_chrominance
//
Codes Mpeg1Decoder::DCCtable[9] =
{
    { 0x0000, 2,  0},
    { 0x0001, 2,  1},
    { 0x0002, 2,  2},
    { 0x0006, 3,  3},
    { 0x000e, 4,  4},
    { 0x001e, 5,  5},
    { 0x003e, 6,  6},
    { 0x007e, 7,  7},
    { 0x00fe, 8,  8},
};

//
// Codes for dct_dc_size_luminance
//
Codes Mpeg1Decoder::DCLtable[9] =
{
    { 0x0004, 3,  0},
    { 0x0000, 2,  1},
    { 0x0001, 2,  2},
    { 0x0005, 3,  3},
    { 0x0006, 3,  4},
    { 0x000e, 4,  5},
    { 0x001e, 5,  6},
    { 0x003e, 6,  7},
    { 0x007e, 7,  8},
};

//
// Motion Vector Codes
//
Codes Mpeg1Decoder::MVDtable[33] =
{
    { 0x0019, 11, 0x00},
    { 0x001b, 11, 0x01},
    { 0x001d, 11, 0x02},
    { 0x001f, 11, 0x03},
    { 0x0021, 11, 0x04},
    { 0x0023, 11, 0x05},
    { 0x0013, 10, 0x06},
    { 0x0015, 10, 0x07},
    { 0x0017, 10, 0x08},
    { 0x0007,  8, 0x09},
    { 0x0009,  8, 0x0a},
    { 0x000b,  8, 0x0b},
    { 0x0007,  7, 0x0c},
    { 0x0003,  5, 0x0d},
    { 0x0003,  4, 0x0e},
    { 0x0003,  3, 0x0f},
    { 0x0001,  1, 0x10},
    { 0x0002,  3, 0x11},
    { 0x0002,  4, 0x12},
    { 0x0002,  5, 0x13},
    { 0x0006,  7, 0x14},
    { 0x000a,  8, 0x15},
    { 0x0008,  8, 0x16},
    { 0x0006,  8, 0x17},
    { 0x0016, 10, 0x18},
    { 0x0014, 10, 0x19},
    { 0x0012, 10, 0x1a},
    { 0x0022, 11, 0x1b},
    { 0x0020, 11, 0x1c},
    { 0x001e, 11, 0x1d},
    { 0x001c, 11, 0x1e},
    { 0x001a, 11, 0x1f},
    { 0x0018, 11, 0x20}
};

// 
// Coded Block Pattern (CBP) codes
Codes Mpeg1Decoder::MVCtable[63] =
{
    { 0x0007, 3, 60},

    { 0x000d, 4,  4},
    { 0x000c, 4,  8},
    { 0x000b, 4, 16},
    { 0x000a, 4, 32},

    { 0x0013, 5, 12},
    { 0x0012, 5, 48},
    { 0x0011, 5, 20},
    { 0x0010, 5, 40},
    { 0x000f, 5, 28},
    { 0x000e, 5, 44},
    { 0x000d, 5, 52},
    { 0x000c, 5, 56},
    { 0x000b, 5,  1},
    { 0x000a, 5, 61},
    { 0x0009, 5,  2},
    { 0x0008, 5, 62},

    { 0x000f, 6, 24},
    { 0x000e, 6, 36},
    { 0x000d, 6,  3},
    { 0x000c, 6, 63},

    { 0x0017, 7,  5},
    { 0x0016, 7,  9},
    { 0x0015, 7, 17},
    { 0x0014, 7, 33},
    { 0x0013, 7,  6},
    { 0x0012, 7, 10},
    { 0x0011, 7, 18},
    { 0x0010, 7, 34},

    { 0x001f, 8,  7},
    { 0x001e, 8, 11},
    { 0x001d, 8, 19},
    { 0x001c, 8, 35},
    { 0x001b, 8, 13},
    { 0x001a, 8, 49},
    { 0x0019, 8, 21},
    { 0x0018, 8, 41},
    { 0x0017, 8, 14},
    { 0x0016, 8, 50},
    { 0x0015, 8, 22},

    { 0x0014, 8, 42},
    { 0x0013, 8, 15},
    { 0x0012, 8, 51},
    { 0x0011, 8, 23},
    { 0x0010, 8, 43},
    { 0x000f, 8, 25},
    { 0x000e, 8, 37},
    { 0x000d, 8, 26},
    { 0x000c, 8, 38},
    { 0x000b, 8, 29},
    { 0x000a, 8, 45},
    { 0x0009, 8, 53},
    { 0x0008, 8, 57},
    { 0x0007, 8, 30},
    { 0x0006, 8, 46},
    { 0x0005, 8, 54},
    { 0x0004, 8, 58},

    { 0x0007, 9, 31},
    { 0x0006, 9, 47},
    { 0x0005, 9, 55},
    { 0x0004, 9, 59},
    { 0x0003, 9, 27},
    { 0x0002, 9, 39},
};

//
// Codes for dct_coeff_first
// LSB of code indicates sign (1 == negative)
//
Codes Mpeg1Decoder::TCOEFF_F[223] = 
{
    { 0x0002,  2,  0x0001},
    { 0x0003,  2,  0x00ff},
    { 0x0008,  5,  0x0002},
    { 0x0009,  5,  0x00fe},
    { 0x000a,  6,  0x0003},
    { 0x000b,  6,  0x00fd},
    { 0x000c,  8,  0x0004},
    { 0x000d,  8,  0x00fc},
    { 0x004c,  9,  0x0005},
    { 0x004d,  9,  0x00fb},
    { 0x0042,  9,  0x0006},
    { 0x0043,  9,  0x00fa},
    { 0x0014, 11,  0x0007},
    { 0x0015, 11,  0x00f9},
    { 0x003a, 13,  0x0008},
    { 0x003b, 13,  0x00f8},
    { 0x0030, 13,  0x0009},
    { 0x0031, 13,  0x00f7},
    { 0x0026, 13,  0x000a},
    { 0x0027, 13,  0x00f6},
    { 0x0020, 13,  0x000b},
    { 0x0021, 13,  0x00f5},
    { 0x0034, 14,  0x000c},
    { 0x0035, 14,  0x00f4},
    { 0x0032, 14,  0x000d},
    { 0x0033, 14,  0x00f3},
    { 0x0030, 14,  0x000e},
    { 0x0031, 14,  0x00f2},
    { 0x002e, 14,  0x000f},
    { 0x002f, 14,  0x00f1},
    { 0x003e, 15,  0x0010},
    { 0x003f, 15,  0x00f0},
    { 0x003c, 15,  0x0011},
    { 0x003d, 15,  0x00ef},
    { 0x003a, 15,  0x0012},
    { 0x003b, 15,  0x00ee},
    { 0x0038, 15,  0x0013},
    { 0x0039, 15,  0x00ed},
    { 0x0036, 15,  0x0014},
    { 0x0037, 15,  0x00ec},
    { 0x0034, 15,  0x0015},
    { 0x0035, 15,  0x00eb},
    { 0x0032, 15,  0x0016},
    { 0x0033, 15,  0x00ea},
    { 0x0030, 15,  0x0017},
    { 0x0031, 15,  0x00e9},
    { 0x002e, 15,  0x0018},
    { 0x002f, 15,  0x00e8},
    { 0x002c, 15,  0x0019},
    { 0x002d, 15,  0x00e7},
    { 0x002a, 15,  0x001a},
    { 0x002b, 15,  0x00e6},
    { 0x0028, 15,  0x001b},
    { 0x0029, 15,  0x00e5},
    { 0x0026, 15,  0x001c},
    { 0x0027, 15,  0x00e4},
    { 0x0024, 15,  0x001d},
    { 0x0025, 15,  0x00e3},
    { 0x0022, 15,  0x001e},
    { 0x0023, 15,  0x00e2},
    { 0x0020, 15,  0x001f},
    { 0x0021, 15,  0x00e1},
    { 0x0030, 16,  0x0020},
    { 0x0031, 16,  0x00e0},
    { 0x002e, 16,  0x0021},
    { 0x002f, 16,  0x00df},
    { 0x002c, 16,  0x0022},
    { 0x002d, 16,  0x00de},
    { 0x002a, 16,  0x0023},
    { 0x002b, 16,  0x00dd},
    { 0x0028, 16,  0x0024},
    { 0x0029, 16,  0x00dc},
    { 0x0026, 16,  0x0025},
    { 0x0027, 16,  0x00db},
    { 0x0024, 16,  0x0026},
    { 0x0025, 16,  0x00da},
    { 0x0022, 16,  0x0027},
    { 0x0023, 16,  0x00d9},
    { 0x0020, 16,  0x0028},
    { 0x0021, 16,  0x00d8},

    { 0x0006,  4,  0x0101},
    { 0x0007,  4,  0x01ff},
    { 0x000c,  7,  0x0102},
    { 0x000d,  7,  0x01fe},
    { 0x004a,  9,  0x0103},
    { 0x004b,  9,  0x01fd},
    { 0x0018, 11,  0x0104},
    { 0x0019, 11,  0x01fc},
    { 0x0036, 13,  0x0105},
    { 0x0037, 13,  0x01fb},
    { 0x002c, 14,  0x0106},
    { 0x002d, 14,  0x01fa},
    { 0x002a, 14,  0x0107},
    { 0x002b, 14,  0x01f9},
    { 0x003e, 16,  0x0108},
    { 0x003f, 16,  0x01f8},
    { 0x003c, 16,  0x0109},
    { 0x003d, 16,  0x01f7},
    { 0x003a, 16,  0x010a},
    { 0x003b, 16,  0x01f6},
    { 0x0038, 16,  0x010b},
    { 0x0039, 16,  0x01f5},
    { 0x0036, 16,  0x010c},
    { 0x0037, 16,  0x01f4},
    { 0x0034, 16,  0x010d},
    { 0x0035, 16,  0x01f3},
    { 0x0032, 16,  0x010e},
    { 0x0033, 16,  0x01f2},

    { 0x0026, 17,  0x010f},
    { 0x0027, 17,  0x01f1},
    { 0x0024, 17,  0x0110},
    { 0x0025, 17,  0x01f0},
    { 0x0022, 17,  0x0111},
    { 0x0023, 17,  0x01ef},
    { 0x0020, 17,  0x0112},
    { 0x0021, 17,  0x01ee},

    { 0x000a,  5,  0x0201},
    { 0x000b,  5,  0x02ff},
    { 0x0008,  8,  0x0202},
    { 0x0009,  8,  0x02fe},
    { 0x0016, 11,  0x0203},
    { 0x0017, 11,  0x02fd},
    { 0x0028, 13,  0x0204},
    { 0x0029, 13,  0x02fc},
    { 0x0028, 14,  0x0205},
    { 0x0029, 14,  0x02fb},

    { 0x000e,  6,  0x0301},
    { 0x000f,  6,  0x03ff},
    { 0x0048,  9,  0x0302},
    { 0x0049,  9,  0x03fe},
    { 0x0038, 13,  0x0303},
    { 0x0039, 13,  0x03fd},
    { 0x0026, 14,  0x0304},
    { 0x0027, 14,  0x03fc},

    { 0x000c,  6,  0x0401},
    { 0x000d,  6,  0x04ff},
    { 0x001e, 11,  0x0402},
    { 0x001f, 11,  0x04fe},
    { 0x0024, 13,  0x0403},
    { 0x0025, 13,  0x04fd},

    { 0x000e,  7,  0x0501},
    { 0x000f,  7,  0x05ff},
    { 0x0012, 11,  0x0502},
    { 0x0013, 11,  0x05fe},
    { 0x0024, 14,  0x0503},
    { 0x0025, 14,  0x05fd},

    { 0x000a,  7,  0x0601},
    { 0x000b,  7,  0x06ff},
    { 0x003c, 13,  0x0602},
    { 0x003d, 13,  0x06fe},
    { 0x0028, 17,  0x0603},
    { 0x0029, 17,  0x06fd},

    { 0x0008,  7,  0x0701},
    { 0x0009,  7,  0x07ff},
    { 0x002a, 13,  0x0702},
    { 0x002b, 13,  0x07fe},

    { 0x000e,  8,  0x0801},
    { 0x000f,  8,  0x08ff},
    { 0x0022, 13,  0x0802},
    { 0x0023, 13,  0x08fe},

    { 0x000a,  8,  0x0901},
    { 0x000b,  8,  0x09ff},
    { 0x0022, 14,  0x0902},
    { 0x0023, 14,  0x09fe},

    { 0x004e,  9,  0x0a01},
    { 0x004f,  9,  0x0aff},
    { 0x0020, 14,  0x0a02},
    { 0x0021, 14,  0x0afe},

    { 0x0046,  9,  0x0b01},
    { 0x0047,  9,  0x0bff},
    { 0x0034, 17,  0x0b02},
    { 0x0035, 17,  0x0bfe},

    { 0x0044,  9,  0x0c01},
    { 0x0045,  9,  0x0cff},
    { 0x0032, 17,  0x0c02},
    { 0x0033, 17,  0x0cfe},

    { 0x0040,  9,  0x0d01},
    { 0x0041,  9,  0x0dff},
    { 0x0030, 17,  0x0d02},
    { 0x0031, 17,  0x0dfe},

    { 0x001c, 11,  0x0e01},
    { 0x001d, 11,  0x0eff},
    { 0x002e, 17,  0x0e02},
    { 0x002f, 17,  0x0efe},

    { 0x001a, 11,  0x0f01},
    { 0x001b, 11,  0x0fff},
    { 0x002c, 17,  0x0f02},
    { 0x002d, 17,  0x0ffe},

    { 0x0010, 11,  0x1001},
    { 0x0011, 11,  0x10ff},
    { 0x002a, 17,  0x1002},
    { 0x002b, 17,  0x10fe},

    { 0x003e, 13,  0x1101},
    { 0x003f, 13,  0x11ff},
    { 0x0034, 13,  0x1201},
    { 0x0035, 13,  0x12ff},
    { 0x0032, 13,  0x1301},
    { 0x0033, 13,  0x13ff},
    { 0x002e, 13,  0x1401},
    { 0x002f, 13,  0x14ff},
    { 0x002c, 13,  0x1501},
    { 0x002d, 13,  0x15ff},
    { 0x003e, 14,  0x1601},
    { 0x003f, 14,  0x16ff},
    { 0x003c, 14,  0x1701},
    { 0x003d, 14,  0x17ff},
    { 0x003a, 14,  0x1801},
    { 0x003b, 14,  0x18ff},
    { 0x0038, 14,  0x1901},
    { 0x0039, 14,  0x19ff},
    { 0x0036, 14,  0x1a01},
    { 0x0037, 14,  0x1aff},

    { 0x003e, 17,  0x1b01},
    { 0x003f, 17,  0x1bff},
    { 0x003c, 17,  0x1c01},
    { 0x003d, 17,  0x1cff},
    { 0x003a, 17,  0x1d01},
    { 0x003b, 17,  0x1dff},
    { 0x0038, 17,  0x1e01},
    { 0x0039, 17,  0x1eff},
    { 0x0036, 17,  0x1f01},
    { 0x0037, 17,  0x1fff},

    { 0x0001,  6,  ESCAPE},
};

//
// Codes for dct_coeff_next
// LSB of code indicates sign (1 == negative)
//
Codes Mpeg1Decoder::TCOEFF_R[224] = 
{
    { 0x0002,  2,  EOBP},
    { 0x0006,  3,  0x0001},
    { 0x0007,  3,  0x00ff},
    { 0x0008,  5,  0x0002},
    { 0x0009,  5,  0x00fe},
    { 0x000a,  6,  0x0003},
    { 0x000b,  6,  0x00fd},
    { 0x000c,  8,  0x0004},
    { 0x000d,  8,  0x00fc},
    { 0x004c,  9,  0x0005},
    { 0x004d,  9,  0x00fb},
    { 0x0042,  9,  0x0006},
    { 0x0043,  9,  0x00fa},
    { 0x0014, 11,  0x0007},
    { 0x0015, 11,  0x00f9},
    { 0x003a, 13,  0x0008},
    { 0x003b, 13,  0x00f8},
    { 0x0030, 13,  0x0009},
    { 0x0031, 13,  0x00f7},
    { 0x0026, 13,  0x000a},
    { 0x0027, 13,  0x00f6},
    { 0x0020, 13,  0x000b},
    { 0x0021, 13,  0x00f5},
    { 0x0034, 14,  0x000c},
    { 0x0035, 14,  0x00f4},
    { 0x0032, 14,  0x000d},
    { 0x0033, 14,  0x00f3},
    { 0x0030, 14,  0x000e},
    { 0x0031, 14,  0x00f2},
    { 0x002e, 14,  0x000f},
    { 0x002f, 14,  0x00f1},
    { 0x003e, 15,  0x0010},
    { 0x003f, 15,  0x00f0},
    { 0x003c, 15,  0x0011},
    { 0x003d, 15,  0x00ef},
    { 0x003a, 15,  0x0012},
    { 0x003b, 15,  0x00ee},
    { 0x0038, 15,  0x0013},
    { 0x0039, 15,  0x00ed},
    { 0x0036, 15,  0x0014},
    { 0x0037, 15,  0x00ec},
    { 0x0034, 15,  0x0015},
    { 0x0035, 15,  0x00eb},
    { 0x0032, 15,  0x0016},
    { 0x0033, 15,  0x00ea},
    { 0x0030, 15,  0x0017},
    { 0x0031, 15,  0x00e9},
    { 0x002e, 15,  0x0018},
    { 0x002f, 15,  0x00e8},
    { 0x002c, 15,  0x0019},
    { 0x002d, 15,  0x00e7},
    { 0x002a, 15,  0x001a},
    { 0x002b, 15,  0x00e6},
    { 0x0028, 15,  0x001b},
    { 0x0029, 15,  0x00e5},
    { 0x0026, 15,  0x001c},
    { 0x0027, 15,  0x00e4},
    { 0x0024, 15,  0x001d},
    { 0x0025, 15,  0x00e3},
    { 0x0022, 15,  0x001e},
    { 0x0023, 15,  0x00e2},
    { 0x0020, 15,  0x001f},
    { 0x0021, 15,  0x00e1},
    { 0x0030, 16,  0x0020},
    { 0x0031, 16,  0x00e0},
    { 0x002e, 16,  0x0021},
    { 0x002f, 16,  0x00df},
    { 0x002c, 16,  0x0022},
    { 0x002d, 16,  0x00de},
    { 0x002a, 16,  0x0023},
    { 0x002b, 16,  0x00dd},
    { 0x0028, 16,  0x0024},
    { 0x0029, 16,  0x00dc},
    { 0x0026, 16,  0x0025},
    { 0x0027, 16,  0x00db},
    { 0x0024, 16,  0x0026},
    { 0x0025, 16,  0x00da},
    { 0x0022, 16,  0x0027},
    { 0x0023, 16,  0x00d9},
    { 0x0020, 16,  0x0028},
    { 0x0021, 16,  0x00d8},

    { 0x0006,  4,  0x0101},
    { 0x0007,  4,  0x01ff},
    { 0x000c,  7,  0x0102},
    { 0x000d,  7,  0x01fe},
    { 0x004a,  9,  0x0103},
    { 0x004b,  9,  0x01fd},
    { 0x0018, 11,  0x0104},
    { 0x0019, 11,  0x01fc},
    { 0x0036, 13,  0x0105},
    { 0x0037, 13,  0x01fb},
    { 0x002c, 14,  0x0106},
    { 0x002d, 14,  0x01fa},
    { 0x002a, 14,  0x0107},
    { 0x002b, 14,  0x01f9},
    { 0x003e, 16,  0x0108},
    { 0x003f, 16,  0x01f8},
    { 0x003c, 16,  0x0109},
    { 0x003d, 16,  0x01f7},
    { 0x003a, 16,  0x010a},
    { 0x003b, 16,  0x01f6},
    { 0x0038, 16,  0x010b},
    { 0x0039, 16,  0x01f5},
    { 0x0036, 16,  0x010c},
    { 0x0037, 16,  0x01f4},
    { 0x0034, 16,  0x010d},
    { 0x0035, 16,  0x01f3},
    { 0x0032, 16,  0x010e},
    { 0x0033, 16,  0x01f2},

    { 0x0026, 17,  0x010f},
    { 0x0027, 17,  0x01f1},
    { 0x0024, 17,  0x0110},
    { 0x0025, 17,  0x01f0},
    { 0x0022, 17,  0x0111},
    { 0x0023, 17,  0x01ef},
    { 0x0020, 17,  0x0112},
    { 0x0021, 17,  0x01ee},

    { 0x000a,  5,  0x0201},
    { 0x000b,  5,  0x02ff},
    { 0x0008,  8,  0x0202},
    { 0x0009,  8,  0x02fe},
    { 0x0016, 11,  0x0203},
    { 0x0017, 11,  0x02fd},
    { 0x0028, 13,  0x0204},
    { 0x0029, 13,  0x02fc},
    { 0x0028, 14,  0x0205},
    { 0x0029, 14,  0x02fb},

    { 0x000e,  6,  0x0301},
    { 0x000f,  6,  0x03ff},
    { 0x0048,  9,  0x0302},
    { 0x0049,  9,  0x03fe},
    { 0x0038, 13,  0x0303},
    { 0x0039, 13,  0x03fd},
    { 0x0026, 14,  0x0304},
    { 0x0027, 14,  0x03fc},

    { 0x000c,  6,  0x0401},
    { 0x000d,  6,  0x04ff},
    { 0x001e, 11,  0x0402},
    { 0x001f, 11,  0x04fe},
    { 0x0024, 13,  0x0403},
    { 0x0025, 13,  0x04fd},

    { 0x000e,  7,  0x0501},
    { 0x000f,  7,  0x05ff},
    { 0x0012, 11,  0x0502},
    { 0x0013, 11,  0x05fe},
    { 0x0024, 14,  0x0503},
    { 0x0025, 14,  0x05fd},

    { 0x000a,  7,  0x0601},
    { 0x000b,  7,  0x06ff},
    { 0x003c, 13,  0x0602},
    { 0x003d, 13,  0x06fe},
    { 0x0028, 17,  0x0603},
    { 0x0029, 17,  0x06fd},

    { 0x0008,  7,  0x0701},
    { 0x0009,  7,  0x07ff},
    { 0x002a, 13,  0x0702},
    { 0x002b, 13,  0x07fe},

    { 0x000e,  8,  0x0801},
    { 0x000f,  8,  0x08ff},
    { 0x0022, 13,  0x0802},
    { 0x0023, 13,  0x08fe},

    { 0x000a,  8,  0x0901},
    { 0x000b,  8,  0x09ff},
    { 0x0022, 14,  0x0902},
    { 0x0023, 14,  0x09fe},

    { 0x004e,  9,  0x0a01},
    { 0x004f,  9,  0x0aff},
    { 0x0020, 14,  0x0a02},
    { 0x0021, 14,  0x0afe},

    { 0x0046,  9,  0x0b01},
    { 0x0047,  9,  0x0bff},
    { 0x0034, 17,  0x0b02},
    { 0x0035, 17,  0x0bfe},

    { 0x0044,  9,  0x0c01},
    { 0x0045,  9,  0x0cff},
    { 0x0032, 17,  0x0c02},
    { 0x0033, 17,  0x0cfe},

    { 0x0040,  9,  0x0d01},
    { 0x0041,  9,  0x0dff},
    { 0x0030, 17,  0x0d02},
    { 0x0031, 17,  0x0dfe},

    { 0x001c, 11,  0x0e01},
    { 0x001d, 11,  0x0eff},
    { 0x002e, 17,  0x0e02},
    { 0x002f, 17,  0x0efe},

    { 0x001a, 11,  0x0f01},
    { 0x001b, 11,  0x0fff},
    { 0x002c, 17,  0x0f02},
    { 0x002d, 17,  0x0ffe},

    { 0x0010, 11,  0x1001},
    { 0x0011, 11,  0x10ff},
    { 0x002a, 17,  0x1002},
    { 0x002b, 17,  0x10fe},

    { 0x003e, 13,  0x1101},
    { 0x003f, 13,  0x11ff},
    { 0x0034, 13,  0x1201},
    { 0x0035, 13,  0x12ff},
    { 0x0032, 13,  0x1301},
    { 0x0033, 13,  0x13ff},
    { 0x002e, 13,  0x1401},
    { 0x002f, 13,  0x14ff},
    { 0x002c, 13,  0x1501},
    { 0x002d, 13,  0x15ff},
    { 0x003e, 14,  0x1601},
    { 0x003f, 14,  0x16ff},
    { 0x003c, 14,  0x1701},
    { 0x003d, 14,  0x17ff},
    { 0x003a, 14,  0x1801},
    { 0x003b, 14,  0x18ff},
    { 0x0038, 14,  0x1901},
    { 0x0039, 14,  0x19ff},
    { 0x0036, 14,  0x1a01},
    { 0x0037, 14,  0x1aff},

    { 0x003e, 17,  0x1b01},
    { 0x003f, 17,  0x1bff},
    { 0x003c, 17,  0x1c01},
    { 0x003d, 17,  0x1cff},
    { 0x003a, 17,  0x1d01},
    { 0x003b, 17,  0x1dff},
    { 0x0038, 17,  0x1e01},
    { 0x0039, 17,  0x1eff},
    { 0x0036, 17,  0x1f01},
    { 0x0037, 17,  0x1fff},

    { 0x0001,  6,  ESCAPE},
};

