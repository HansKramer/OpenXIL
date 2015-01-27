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
//  File:       H261CompressorData.hh
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:23:12, 03/10/00
//
//  Description:
//
//    Class definition for H261CompressorData
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)H261CompressorData.hh	1.4\t00/03/10  "

#ifndef H261COMPRESSORDATA_H
#define H261COMPRESSORDATA_H

#include "xil/xilGPI.hh"

#include "H261Decoder.hh"
#include "H261Splatter.hh"

class H261CompressorData {

public: 
    XilCisBufferManager	*cbm;
    int                  version;
    int                  BitsPerImage;
    int                  ImageSkip;
    int			 searchX;
    int			 searchY;
    Xil_boolean          LoopFilter;
    Xil_boolean          EncodeIntra;
    Xil_boolean          FreezeRelease;
    Xil_boolean          SplitScreen;
    Xil_boolean          DocCamera;

    // Constructor
    H261CompressorData() ;

    
    int allocOk()	{return 1;}
    void reset();	
};

#endif /* H261COMPRESSORDATA_H */
