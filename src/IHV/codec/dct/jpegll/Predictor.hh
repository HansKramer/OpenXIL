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
//  File:       Predictor.hh
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:23:04, 03/10/00
//
//  Description:
//
//    Class Declaration for Predictor.   Differentially encodes an
//    image by the JPEG lossless standard.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Predictor.hh	1.5\t00/03/10  "


#ifndef PREDICTOR_H
#define PREDICTOR_H

#include "XilDeviceCompressionJpegLL.hh"
#include "CompressInfo.hh"

class Predictor {
public:
    Predictor(CompressInfo* ci, 
              int           point_transform, 
              int           predictor_type,
              Xil_boolean   isInterleaved);

    ~Predictor();
    Xil_boolean isOK();

    void predict8(Xil_unsigned8* src, Xil_boolean doRestart);
    void predict16(Xil_signed16* src, Xil_boolean doRestart);

    Xil_signed16* getDiffs();


private:
    unsigned int   src_ps;  // Same as image, unless pt_transform>0
    unsigned int   src_ss;  // then its the values for the temp buffer
    unsigned int   dst_ps;
    unsigned int   dst_ss;

    unsigned int   bands_per_scan;    // # bands per SOS, max = 4
    unsigned int   pt_transform;      // Point transform
    unsigned int   prediction_type;
    unsigned int   precision;
    unsigned int   width;
    unsigned int   nbands;
    XilDataType    datatype;
    void*          base;
    int            first_pixel;

    Xil_signed16*  saveBuf[2];
    Xil_signed16*  ptBuf;
    Xil_signed16*  diffBuf;
    unsigned int   rowCounter;

    Xil_boolean    isOKFlag;
};

#endif // PREDICTOR_H
