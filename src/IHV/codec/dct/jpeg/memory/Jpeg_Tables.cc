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
//  File:       Jpeg_Tables.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:14:24, 03/10/00
//
//  Description:
//
//   Default tables used during jpeg encoding:
//
//    int qtable0:        default quantization table 0
//
//    int qtable1:        default quantization table 1
//
//    huffman_code dc_table: default dc coeff. huffman tables
//
//    huffman_code ac_table: default ac coeff. huffman tables
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Jpeg_Tables.cc	1.4\t00/03/10  "


#include "xil/xilGPI.hh"
#include "XilDeviceCompressionJpeg.hh"


//
// Luminance Q table
//
int XilDeviceCompressionJpeg::qtable0[8][8] =  {
    {  16,  11,  10,  16,  24,  40,  51,  61 },
    {  12,  12,  14,  19,  26,  58,  60,  55 },
    {  14,  13,  16,  24,  40,  57,  69,  56 },
    {  14,  17,  22,  29,  51,  87,  80,  62 },
    {  18,  22,  37,  56,  68, 109, 103,  77 },
    {  24,  35,  55,  64,  81, 104, 113,  92 },
    {  49,  64,  78,  87, 103, 121, 120, 101 },
    {  72,  92,  95,  98, 112, 100, 103,  99 }
};


//
// Chrominance Q table
//
int XilDeviceCompressionJpeg::qtable1[8][8] =   {
    {  17,  18,  24,  47,  99,  99,  99,  99 },
    {  18,  21,  26,  66,  99,  99,  99,  99 },
    {  24,  26,  56,  99,  99,  99,  99,  99 },
    {  47,  66,  99,  99,  99,  99,  99,  99 },
    {  99,  99,  99,  99,  99,  99,  99,  99 },
    {  99,  99,  99,  99,  99,  99,  99,  99 },
    {  99,  99,  99,  99,  99,  99,  99,  99 },
    {  99,  99,  99,  99,  99,  99,  99,  99 }
};


//
// Luminance DC Huffman Table
//
Huffman_Code XilDeviceCompressionJpeg::dc_table0[16] = {
    {  2, 0x0000 },{  3, 0x0002 },{  3, 0x0003 },{  3, 0x0004 },
    {  3, 0x0005 },{  3, 0x0006 },{  4, 0x000e },{  5, 0x001e },
    {  6, 0x003e },{  7, 0x007e },{  8, 0x00fe },{  9, 0x01fe },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 }
};


//
// Chrominance DC Huffman Table
//
Huffman_Code XilDeviceCompressionJpeg::dc_table1[16] = {
    {  2, 0x0000 },{  2, 0x0001 },{  2, 0x0002 },{  3, 0x0006 },
    {  4, 0x000e },{  5, 0x001e },{  6, 0x003e },{  7, 0x007e },
    {  8, 0x00fe },{  9, 0x01fe },{ 10, 0x03fe },{ 11, 0x07fe },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 }
};

//
// Luminance AC Huffman Table
//
Huffman_Code XilDeviceCompressionJpeg::ac_table0[256] = {
    {  4, 0x000a }, {  2, 0x0000 }, {  2, 0x0001 }, {  3, 0x0004 },
    {  4, 0x000b }, {  5, 0x001a }, {  7, 0x0078 }, {  8, 0x00f8 },
    { 10, 0x03f6 }, { 16, 0xff82 }, { 16, 0xff83 }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, {  4, 0x000c }, {  5, 0x001b }, {  7, 0x0079 },
    {  9, 0x01f6 }, { 11, 0x07f6 }, { 16, 0xff84 }, { 16, 0xff85 },
    { 16, 0xff86 }, { 16, 0xff87 }, { 16, 0xff88 }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, {  5, 0x001c }, {  8, 0x00f9 }, { 10, 0x03f7 },
    { 12, 0x0ff4 }, { 16, 0xff89 }, { 16, 0xff8a }, { 16, 0xff8b },
    { 16, 0xff8c }, { 16, 0xff8d }, { 16, 0xff8e }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, {  6, 0x003a }, {  9, 0x01f7 }, { 12, 0x0ff5 },
    { 16, 0xff8f }, { 16, 0xff90 }, { 16, 0xff91 }, { 16, 0xff92 },
    { 16, 0xff93 }, { 16, 0xff94 }, { 16, 0xff95 }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, {  6, 0x003b }, { 10, 0x03f8 }, { 16, 0xff96 },
    { 16, 0xff97 }, { 16, 0xff98 }, { 16, 0xff99 }, { 16, 0xff9a },
    { 16, 0xff9b }, { 16, 0xff9c }, { 16, 0xff9d }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },


    {  0, 0x0000 }, {  7, 0x007a }, { 11, 0x07f7 }, { 16, 0xff9e },
    { 16, 0xff9f }, { 16, 0xffa0 }, { 16, 0xffa1 }, { 16, 0xffa2 },
    { 16, 0xffa3 }, { 16, 0xffa4 }, { 16, 0xffa5 }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, {  7, 0x007b }, { 12, 0x0ff6 }, { 16, 0xffa6 },
    { 16, 0xffa7 }, { 16, 0xffa8 }, { 16, 0xffa9 }, { 16, 0xffaa },
    { 16, 0xffab }, { 16, 0xffac }, { 16, 0xffad }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, {  8, 0x00fa }, { 12, 0x0ff7 }, { 16, 0xffae },
    { 16, 0xffaf }, { 16, 0xffb0 }, { 16, 0xffb1 }, { 16, 0xffb2 },
    { 16, 0xffb3 }, { 16, 0xffb4 }, { 16, 0xffb5 }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, {  9, 0x01f8 }, { 15, 0x7fc0 }, { 16, 0xffb6 },
    { 16, 0xffb7 }, { 16, 0xffb8 }, { 16, 0xffb9 }, { 16, 0xffba },
    { 16, 0xffbb }, { 16, 0xffbc }, { 16, 0xffbd }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, {  9, 0x01f9 }, { 16, 0xffbe }, { 16, 0xffbf },
    { 16, 0xffc0 }, { 16, 0xffc1 }, { 16, 0xffc2 }, { 16, 0xffc3 },
    { 16, 0xffc4 }, { 16, 0xffc5 }, { 16, 0xffc6 }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, {  9, 0x01fa }, { 16, 0xffc7 }, { 16, 0xffc8 },
    { 16, 0xffc9 }, { 16, 0xffca }, { 16, 0xffcb }, { 16, 0xffcc },
    { 16, 0xffcd }, { 16, 0xffce }, { 16, 0xffcf }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, { 10, 0x03f9 }, { 16, 0xffd0 }, { 16, 0xffd1 },
    { 16, 0xffd2 }, { 16, 0xffd3 }, { 16, 0xffd4 }, { 16, 0xffd5 },
    { 16, 0xffd6 }, { 16, 0xffd7 }, { 16, 0xffd8 }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, { 10, 0x03fa }, { 16, 0xffd9 }, { 16, 0xffda },
    { 16, 0xffdb }, { 16, 0xffdc }, { 16, 0xffdd }, { 16, 0xffde },
    { 16, 0xffdf }, { 16, 0xffe0 }, { 16, 0xffe1 }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, { 11, 0x07f8 }, { 16, 0xffe2 }, { 16, 0xffe3 },
    { 16, 0xffe4 }, { 16, 0xffe5 }, { 16, 0xffe6 }, { 16, 0xffe7 },
    { 16, 0xffe8 }, { 16, 0xffe9 }, { 16, 0xffea }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, { 16, 0xffeb }, { 16, 0xffec }, { 16, 0xffed },
    { 16, 0xffee }, { 16, 0xffef }, { 16, 0xfff0 }, { 16, 0xfff1 },
    { 16, 0xfff2 }, { 16, 0xfff3 }, { 16, 0xfff4 }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    { 11, 0x07f9 }, { 16, 0xfff5 }, { 16, 0xfff6 }, { 16, 0xfff7 },
    { 16, 0xfff8 }, { 16, 0xfff9 }, { 16, 0xfffa }, { 16, 0xfffb },
    { 16, 0xfffc }, { 16, 0xfffd }, { 16, 0xfffe }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 }
};


//
// Chrominance AC Huffman Table
//
Huffman_Code XilDeviceCompressionJpeg::ac_table1[256] = {
    {  2, 0x0000 }, {  2, 0x0001 }, {  3, 0x0004 }, {  4, 0x000a },
    {  5, 0x0018 }, {  5, 0x0019 }, {  6, 0x0038 }, {  7, 0x0078 },
    {  9, 0x01f4 }, { 10, 0x03f6 }, { 12, 0x0ff4 }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, {  4, 0x000b }, {  6, 0x0039 },{  8, 0x00f6 },
    {  9, 0x01f5 }, { 11, 0x07f6 }, { 12, 0x0ff5 },{ 16, 0xff88 },
    { 16, 0xff89 }, { 16, 0xff8a }, { 16, 0xff8b }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, {  5, 0x001a }, {  8, 0x00f7 }, { 10, 0x03f7 },
    { 12, 0x0ff6 }, { 15, 0x7fc2 }, { 16, 0xff8c }, { 16, 0xff8d },
    { 16, 0xff8e }, { 16, 0xff8f }, { 16, 0xff90 }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, {  5, 0x001b }, {  8, 0x00f8 }, { 10, 0x03f8 },
    { 12, 0x0ff7 }, { 16, 0xff91 }, { 16, 0xff92 }, { 16, 0xff93 },
    { 16, 0xff94 }, { 16, 0xff95 }, { 16, 0xff96 }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, {  6, 0x003a }, {  9, 0x01f6 }, { 16, 0xff97 },
    { 16, 0xff98 }, { 16, 0xff99 }, { 16, 0xff9a }, { 16, 0xff9b },
    { 16, 0xff9c }, { 16, 0xff9d }, { 16, 0xff9e }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, {  6, 0x003b }, { 10, 0x03f9 }, { 16, 0xff9f },
    { 16, 0xffa0 }, { 16, 0xffa1 }, { 16, 0xffa2 }, { 16, 0xffa3 },
    { 16, 0xffa4 }, { 16, 0xffa5 }, { 16, 0xffa6 }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, {  7, 0x0079 }, { 11, 0x07f7 }, { 16, 0xffa7 },
    { 16, 0xffa8 }, { 16, 0xffa9 }, { 16, 0xffaa }, { 16, 0xffab },
    { 16, 0xffac }, { 16, 0xffad }, { 16, 0xffae }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, {  7, 0x007a }, { 11, 0x07f8 }, { 16, 0xffaf },
    { 16, 0xffb0 }, { 16, 0xffb1 }, { 16, 0xffb2 }, { 16, 0xffb3 },
    { 16, 0xffb4 }, { 16, 0xffb5 }, { 16, 0xffb6 }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, {  8, 0x00f9 }, { 16, 0xffb7 }, { 16, 0xffb8 },
    { 16, 0xffb9 }, { 16, 0xffba }, { 16, 0xffbb }, { 16, 0xffbc },
    { 16, 0xffbd }, { 16, 0xffbe }, { 16, 0xffbf }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, {  9, 0x01f7 }, { 16, 0xffc0 }, { 16, 0xffc1 },
    { 16, 0xffc2 }, { 16, 0xffc3 }, { 16, 0xffc4 }, { 16, 0xffc5 },
    { 16, 0xffc6 }, { 16, 0xffc7 }, { 16, 0xffc8 }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, {  9, 0x01f8 }, { 16, 0xffc9 }, { 16, 0xffca },
    { 16, 0xffcb }, { 16, 0xffcc }, { 16, 0xffcd }, { 16, 0xffce },
    { 16, 0xffcf }, { 16, 0xffd0 }, { 16, 0xffd1 }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, {  9, 0x01f9 }, { 16, 0xffd2 }, { 16, 0xffd3 },
    { 16, 0xffd4 }, { 16, 0xffd5 }, { 16, 0xffd6 }, { 16, 0xffd7 },
    { 16, 0xffd8 }, { 16, 0xffd9 }, { 16, 0xffda }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, {  9, 0x01fa }, { 16, 0xffdb }, { 16, 0xffdc },
    { 16, 0xffdd }, { 16, 0xffde }, { 16, 0xffdf }, { 16, 0xffe0 },
    { 16, 0xffe1 }, { 16, 0xffe2 }, { 16, 0xffe3 }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, { 11, 0x07f9 }, { 16, 0xffe4 }, { 16, 0xffe5 },
    { 16, 0xffe6 }, { 16, 0xffe7 }, { 16, 0xffe8 }, { 16, 0xffe9 },
    { 16, 0xffea }, { 16, 0xffeb }, { 16, 0xffec }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    {  0, 0x0000 }, { 14, 0x3fe0 }, { 16, 0xffed }, { 16, 0xffee },
    { 16, 0xffef }, { 16, 0xfff0 }, { 16, 0xfff1 }, { 16, 0xfff2 },
    { 16, 0xfff3 }, { 16, 0xfff4 }, { 16, 0xfff5 }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },

    { 10, 0x03fa }, { 15, 0x7fc3 }, { 16, 0xfff6 }, { 16, 0xfff7 },
    { 16, 0xfff8 }, { 16, 0xfff9 }, { 16, 0xfffa }, { 16, 0xfffb },
    { 16, 0xfffc }, { 16, 0xfffd }, { 16, 0xfffe }, {  0, 0x0000 },
    {  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 },{  0, 0x0000 }
};
