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
//  File:	XilDeviceCompressionIdentity.hh
//  Project:	XIL
//  Created:	93/04/14
//  Revision:	1.5
//  Last Mod:	10:22:40, 03/10/00
//
//  Description:
//	The file contains the definitions for Identity compression and
//  decompression.  Each Identity cis has its own instantiation of
//  this class.
//	
//	The Identity bit stream has the following format:
//
//         [ 32-bit INTEGER ]    width
//         [ 32-bit INTEGER ]    height
//         [ 32-bit INTEGER ]    nbands
//         [ IMAGE DATA ]
//	
//	NOTE:  The code included here to implement this bit-stream
//  creates a compressed stream which is not portable between
//  different endian machines (i.e. x86 <--> SPARC).
//	
//------------------------------------------------------------------------
#pragma ident	"@(#)XilDeviceCompressionIdentity.hh	1.5\t00/03/10  "

#ifndef XilDeviceCompressionIdentity_H
#define XilDeviceCompressionIdentity_H

#include <xil/xilGPI.hh>

#define FRAMES_PER_BUFFER  3
#define IDENTITY_FRAME_TYPE 1

class XilDeviceCompressionIdentity : public XilDeviceCompression
{
public:
    XilDeviceCompressionIdentity(XilDeviceManagerCompression* xdct,
                                 XilCis*                   cis);

    ~XilDeviceCompressionIdentity(void);

    //
    //  PURE VIRTUAL IMPLEMENTATIONS
    //  from XilDeviceCompression 
    //
    XilStatus    compress(XilOp*       op,
                          unsigned int op_count,
                          XilRoi*      roi,
                          XilBoxList*  bl);
 
    XilStatus    decompress(XilOp*       op,
                            unsigned int op_count,
                            XilRoi*      roi,
                            XilBoxList*  bl);

    int            getMaxFrameSize(void);
    void           burnFrames(int nframes);


    //
    //  Function to read header and fill in the header information --
    //  specifically width and height
    //


    //
    //  VIRTUAL FUNCTION OVERRIDES
    //  Base class functions do not work for Identity codec.
    //  The Identity codec marks even frames with its own
    //  frame type; this is done in order to illustrate how a codec
    //  with typed frames would interface with the cbm

    void        seek(int framenumber, Xil_boolean history_update=TRUE);
    int         adjustStart(int framenumber);
    int         findNextFrameBoundary(void);

    // Read header to get width and height
    XilStatus   deriveOutputType(void);
   
    //  Reset the codec state, destroy old inputType
    void	reset();

    //
    //  NEW FUNCTIONS
    //
    void        setCompressionQuality(int value);
    int         getCompressionQuality();
    void        setDecompressionQuality(int value);
    int         getDecompressionQuality();
   
private:
    //
    //  Private function used by reset and the constructor to set values
    //
    XilStatus       initValues();

    int comp_quality;
    int decomp_quality;
    Xil_boolean isDerivedType;   // flag for derived type from bitstream

};

#endif
