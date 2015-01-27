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
//  File:       H261FrameInfo.hh
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:23:10, 03/10/00
//
//  Description:
//
//    Definition of structure describing the decompressed
//    storage for a frame and its upsampled version.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)H261FrameInfo.hh	1.2\t00/03/10  "

#ifndef H261_FRAME_INFO_HH
#define H261_FRAME_INFO_HH

#include "xil/xilGPI.hh"

//
// Structure to describe the size and storage layout
// of the decoded YUV data and the upsampled RBG data.
// (Note: The "RGB" data may actually still be in YUV
//         colorspace for the atomic decompress case, 
//         but we use the name RGB to distinguish it).
//        
struct H261FrameInfo {
    unsigned int   width;
    unsigned int   height;
    unsigned int   nbands; // TODO: lperry - Unnecessary ??
    Xil_unsigned8* yptr;
    Xil_unsigned8* uptr;
    Xil_unsigned8* vptr;
    Xil_unsigned8* rptr;
    Xil_unsigned8* gptr;
    Xil_unsigned8* bptr;
    unsigned int   yps;
    unsigned int   ups;
    unsigned int   vps;
    unsigned int   yss;
    unsigned int   uss;
    unsigned int   vss;
    unsigned int   rps;
    unsigned int   gps;
    unsigned int   bps;
    unsigned int   rss;
    unsigned int   gss;
    unsigned int   bss;
};

#endif // H261_FRAME_INFO_HH
