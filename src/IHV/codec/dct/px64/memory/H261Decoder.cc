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
//  File:       H261Decoder.cc
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:15:11, 03/10/00
//
//  Description:
//
//    Huffman decoding tables and functions
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)H261Decoder.cc	1.5\t00/03/10  "

#include <stdio.h>
#include <math.h>
#include "xil/xilGPI.hh"
#include "H261Decoder.hh"


void dump_decode_table(int *ptr, char *tmp);


//XXX Reset.  Free tables?? Set data to 0

/*
 * H.261 Macroblock address VLC
 *
 */
static Codes MBA[] = 
{
    { 0x0000,  0,  0},
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
    { 0x000f, 11, MBA_STUFFING},	/* MBA stuffing */
    { 0x0001, 16, MBA_START_CODE},	/* start code */
};

/*
 * H.261 Macroblock type VLC
 * Describes presence/absence of 
 *	Macroblock Quantizer,
 *	Motion Vector data,
 *	Coded block pattern and
 *	coefficients.
 * The value field in this table is a bitwise OR of
 *	MTYPE_INTER | MTYPE_MC | MTYPE_FIL | MTYPE_CBP | MTYPE_MQUANT
 */
static Codes MTYPE[] = 
{
    { 0x0001,  4, 0x00},		/* Intra */
    { 0x0001,  7, 0x01},
    { 0x0001,  1, 0x12},
    { 0x0001,  5, 0x13},
    { 0x0001,  9, 0x18},
    { 0x0001,  8, 0x1a},
    { 0x0001, 10, 0x1b},
    { 0x0001,  3, 0x1c},
    { 0x0001,  2, 0x1e},
    { 0x0001,  6, 0x1f},
};

/*
 * H.261 Motion Vector data
 */
static Codes MVD[] =
{
    { 0x0019, 11,  -16},
    { 0x001b, 11,  -15},
    { 0x001d, 11,  -14},
    { 0x001f, 11,  -13},
    { 0x0021, 11,  -12},
    { 0x0023, 11,  -11},
    { 0x0013, 10,  -10},
    { 0x0015, 10,  -9},
    { 0x0017, 10,  -8},
    { 0x0007,  8,  -7},
    { 0x0009,  8,  -6},
    { 0x000b,  8,  -5},
    { 0x0007,  7,  -4},
    { 0x0003,  5,  -3},
    { 0x0003,  4,  -2},
    { 0x0003,  3,  -1},
    { 0x0001,  1,   0},
    { 0x0002,  3,   1},
    { 0x0002,  4,   2},
    { 0x0002,  5,   3},
    { 0x0006,  7,   4},
    { 0x000a,  8,   5},
    { 0x0008,  8,   6},
    { 0x0006,  8,   7},
    { 0x0016, 10,   8},
    { 0x0014, 10,   9},
    { 0x0012, 10,   10},
    { 0x0022, 11,   11},
    { 0x0020, 11,   12},
    { 0x001e, 11,   13},
    { 0x001c, 11,   14},
    { 0x001a, 11,   15},
};

/*
 * H.261 Coded block pattern VLC
 */
static Codes CBP[] =
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

// OPT for space:
//   Can we use one table for TCOEFF, just manually replacing the codes which
//   are different??

/*
 * H.261 Coefficient VLC, for decoding first coeff in a block
 *
 * Value is   (Runlength << 8) | Level
 */
static Codes TCOEFF_F[] = 
{
    { 0x0002,  2,  0x0001},	/* RunLength 0, Level 1 */
    { 0x0003,  2,  0x00ff},	/* RunLength 0, Level -1 */
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

    { 0x0006,  4,  0x0101},	/* RunLength 1, Level 1 */
    { 0x0007,  4,  0x01ff},	/* RunLength 1, Level -1 */
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
    { 0x0044,  9,  0x0c01},
    { 0x0045,  9,  0x0cff},
    { 0x0040,  9,  0x0d01},
    { 0x0041,  9,  0x0dff},
    { 0x001c, 11,  0x0e01},
    { 0x001d, 11,  0x0eff},
    { 0x001a, 11,  0x0f01},
    { 0x001b, 11,  0x0fff},
    { 0x0010, 11,  0x1001},
    { 0x0011, 11,  0x10ff},
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
    { 0x0001,  6,  ESC_CODE},	/* Escape code */
};

/*
 * H.261 Coefficient VLC, for decoding all but first coeff in a block
 */
static Codes TCOEFF_R[] = 
{
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
    { 0x0044,  9,  0x0c01},
    { 0x0045,  9,  0x0cff},
    { 0x0040,  9,  0x0d01},
    { 0x0041,  9,  0x0dff},
    { 0x001c, 11,  0x0e01},
    { 0x001d, 11,  0x0eff},
    { 0x001a, 11,  0x0f01},
    { 0x001b, 11,  0x0fff},
    { 0x0010, 11,  0x1001},
    { 0x0011, 11,  0x10ff},
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
    { 0x0001,  6,  ESC_CODE},	/* Escape code */
    { 0x0002,  2,  EOB_CODE},	/* EOB */
};


H261Decoder::H261Decoder()
{
    isok = 0;

    int TCOEFF_R_len = sizeof(TCOEFF_R)/sizeof(Codes);
    int MBA_len = sizeof(MBA)/sizeof(Codes);
    int MTYPE_len = sizeof(MTYPE)/sizeof(Codes);
    int MVD_len = sizeof(MVD)/sizeof(Codes);
    int CBP_len = sizeof(CBP)/sizeof(Codes);
    int TCOEFF_F_len = sizeof(TCOEFF_F)/sizeof(Codes);

    mbaTable = NULL;
    mvdTable = NULL;
    mtyTable = NULL;
    cbpTable = NULL;
    dctTable = NULL;
    fstTable = NULL;

    if ((mbaTable = (int *)create_decode_table(NULL,MBA,MBA_len)) == NULL) {
      return;
    }
    if ((mvdTable = (int *)create_decode_table(NULL,MVD,MVD_len)) == NULL) {
      return;
    }
    if ((mtyTable = (int *)create_decode_table(NULL,MTYPE,MTYPE_len)) == NULL) {
      return;
    }
    if ((cbpTable = (int *)create_decode_table(NULL,CBP,CBP_len)) == NULL) {
      return;
    }
    if ((dctTable = (int *)create_decode_table(NULL,TCOEFF_R,TCOEFF_R_len))== NULL ) {
      return;
    }
    if ((fstTable = (int *)create_decode_table(NULL,TCOEFF_F,TCOEFF_F_len)) == NULL) {
      return;
    }

    isok = 1;
}

H261Decoder::~H261Decoder()
{
    // Free decode tables
    if (mbaTable) {
      free_decode_table(mbaTable);
      mbaTable = NULL;
    }
    if (mvdTable) {
      free_decode_table(mvdTable);
      mvdTable = NULL;
    }
    if (mtyTable) {
      free_decode_table(mtyTable);
      mtyTable = NULL;
    }
    if (cbpTable) {
      free_decode_table(cbpTable);
      cbpTable = NULL;
    }
    if (dctTable) {
      free_decode_table(dctTable);
      dctTable = NULL;
    }
    if (fstTable) {
      free_decode_table(fstTable);
      fstTable = NULL;
    }

}

