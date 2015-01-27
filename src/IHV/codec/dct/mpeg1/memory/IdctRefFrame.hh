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
//  File:       IdctRefFrame.hh
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:22:58, 03/10/00
//
//  Description:
//
//    Define objects for MacroBlocks, Frames, and a Reference
//    for use in decoding P and B frames.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)IdctRefFrame.hh	1.5\t00/03/10  "

#ifndef IDCTREFFRAME_H
#define IDCTREFFRAME_H

#include <xil/xilGPI.hh>

//
// Define a MacroBlock object, which consists of 6 8x8 DCT blocks,
// Ordered as YYYYCrCb. The elements are really shorts, but we
// allocate them as doubles in order to get 64 bit alignment
// sine the IDCT routine depends on it.
//


struct MacroBlock {
    double block88[6][16];
};

 
//
// The Reference frame holds a ptr to an array
// of Macroblocks and a pointer to a Reference object
// which is a de-blocked version of the frame
//
class Mpeg1ReferenceFrame {
public:
    Mpeg1ReferenceFrame();
    ~Mpeg1ReferenceFrame();

    void             insertMacroBlock(MacroBlock* mb, 
                                      int* y_dst, int* cb_dst, int* cr_dst);
    void             insertBlock(int* src_blk, int* dst_block, 
                                 unsigned int line_stride);
    void             getMVReference(MacroBlock* mb, int mbx, int mby,
                                    int frac_dx, int frac_dy, int refqual);

    XilStatus        allocMacroBlocks(int nblocks);
    XilStatus        createReference(unsigned int width,
                                     unsigned int height);
    void             populateReference();
    void             reset();

    Xil_signed16*    getYDataPtr()          {return ydata;}        
    Xil_signed16*    getCrDataPtr()         {return crdata;}        
    Xil_signed16*    getCbDataPtr()         {return cbdata;}        

    unsigned int     getYScanlineStride()   {return ystride;}
    unsigned int     getCrScanlineStride()  {return cstride;}
    unsigned int     getCbScanlineStride()  {return cstride;}

    void             setType(int type_arg)  {type = type_arg;}
    int              getType()              {return type;}

    void             setFrameId(int id)     {frame_id = id;}
    int              getFrameId()           {return frame_id;}

    MacroBlock*      getMacroBlockArray()   {return mb;}
    MacroBlock*      getMacroBlock(int i)   {return mb+i;}

    Xil_boolean      getFilledFlag();
    void             setFilledFlag(Xil_boolean on_off);


private:
    Xil_boolean      filled;    // Marks the frame as already filled
    int              type;
    int              frame_id;
    int              w16;      // Width  bumped up to next mult of 16
    int              h16;      // Height bumped up to next mult of 16
    int              nxmb;     // Number of Macroblocks in X
    int              nymb;     // Number of Macroblocks in Y
    MacroBlock*      mb;       // Array of MacroBlocks

    Xil_signed16*    theBuffer;

    Xil_signed16*    ydata;
    Xil_signed16*    crdata;
    Xil_signed16*    cbdata;

    unsigned int     width;
    unsigned int     height;
    unsigned int     ystride;
    unsigned int     yblkstride;
    unsigned int     ymbstride;
    unsigned int     cstride;
    unsigned int     cmbstride;
};

#endif // IDCTREFFRAME_H
