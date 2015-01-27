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
//  File:       JpegLL_Tables.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:14:59, 03/10/00
//
//  Description:
//
//    Static tables for Jpeg Lossless Codec
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)JpegLL_Tables.cc	1.2\t00/03/10  "


#include "XilDeviceCompressionJpegLL.hh"

Huffman_Code 
XilDeviceCompressionJpegLL::jpegll_table0[17] = {
    {  2, 0x0000 }, {  3, 0x0002 }, {  3, 0x0003 }, {  3, 0x0004 },
    {  3, 0x0005 }, {  3, 0x0006 }, {  4, 0x000e }, {  5, 0x001e },
    {  6, 0x003e }, {  7, 0x007e }, {  8, 0x00fe }, {  9, 0x01fe },
    { 10, 0x03fe }, { 11, 0x07fe }, { 12, 0x0ffe }, { 13, 0x1ffe },
    { 14, 0x3ffe }
};

Huffman_Code 
XilDeviceCompressionJpegLL::jpegll_table1[17] = {
    {  2, 0x0000 }, {  2, 0x0001 }, {  2, 0x0002 }, {  3, 0x0006 },
    {  4, 0x000e }, {  5, 0x001e }, {  6, 0x003e }, {  7, 0x007e },
    {  8, 0x00fe }, {  9, 0x01fe }, { 10, 0x03fe }, { 11, 0x07fe },
    { 12, 0x0ffe }, { 13, 0x1ffe }, { 14, 0x3ffe }, { 15, 0x7ffe },
    { 16, 0xfffe }
};

Huffman_Code 
XilDeviceCompressionJpegLL::jpegll_table2[17] = {
    {  2, 0x0000 }, {  3, 0x0002 }, {  3, 0x0003 }, {  3, 0x0004 },
    {  3, 0x0005 }, {  3, 0x0006 }, {  4, 0x000e }, {  5, 0x001e },
    {  6, 0x003e }, {  7, 0x007e }, {  8, 0x00fe }, {  9, 0x01fe },
    { 10, 0x03fe }, { 11, 0x07fe }, { 12, 0x0ffe }, { 13, 0x1ffe },
    { 14, 0x3ffe }
};

Huffman_Code 
XilDeviceCompressionJpegLL::jpegll_table3[17] = {
    {  2, 0x0000 }, {  2, 0x0001 }, {  2, 0x0002 }, {  3, 0x0006 },
    {  4, 0x000e }, {  5, 0x001e }, {  6, 0x003e }, {  7, 0x007e },
    {  8, 0x00fe }, {  9, 0x01fe }, { 10, 0x03fe }, { 11, 0x07fe },
    { 12, 0x0ffe }, { 13, 0x1ffe }, { 14, 0x3ffe }, { 15, 0x7ffe },
    { 16, 0xfffe }
};

