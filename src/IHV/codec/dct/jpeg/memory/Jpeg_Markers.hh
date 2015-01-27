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
//  File:       Jpeg_Markers.hh
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:22:50, 03/10/00
//
//  Description:
//
//    JPEG defines and markers
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Jpeg_Markers.hh	1.3\t00/03/10  "



#ifndef JPEG_MARKERS_H
#define JPEG_MARKERS_H

#define MACRO_BLOCK_SIZE      8
#define MACRO_BLOCK_SIZE_411 16

#define JPEG_BASELINE 0
#define JPEG_EXTENDED 1

// JPEG Marker definitions
	
#define MARKER	0xff
#define TMP	0x01
#define SOF(N)	(0xc0 + N)
#define DHT	0xc4
#define DAC	0xcc
#define RST(N)	(0xd0 + N)
#define SOI	0xd8
#define EOI	0xd9
#define SOS	0xda
#define DQT	0xdb
#define DNL	0xdc
#define DRI	0xdd
#define DHP	0xde
#define EXP	0xdf
#define APP(N)	(0xe0 + N)
#define JPG(N)	(0xf0 + N)
#define COM	0xfe

#endif
