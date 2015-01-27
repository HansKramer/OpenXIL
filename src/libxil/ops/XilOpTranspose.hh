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
//  File:	XilOpTranspose.hh
//  Project:	XIL
//  Revision:	1.4
//  Last Mod:	10:20:44, 03/10/00
//
//  Description:
//	
//	
//	
//	
//	
//	
//	
//	
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident  "@(#)XilOpTranspose.hh	1.4\t00/03/10  "

class XilOpTranspose : public XilOp {
public:
    static XilOp*        create(char* function_name,
                                void* args[],
                                int count);

protected:
    //
    //  Constructor
    //
                         XilOpTranspose(XilOpNumber  op_num,
                                        XilFlipType  flip,
                                        unsigned int width,
                                        unsigned int height) :
                             XilOp(op_num)
    {
        fliptype = flip;
        src_xsize = width;
        src_ysize = height;
    }

    //
    //  Destructor
    //
    virtual              ~XilOpTranspose()
    {
    }

    //
    //  Need to account for the transpose operation in generating the
    //  intersected ROI
    //
    virtual XilStatus    generateIntersectedRoi();

    //
    //  Move into object space.
    //
    virtual XilStatus    moveIntoObjectSpace(XiliRect*            rect,
                                             XilDeferrableObject* object);

    virtual XilStatus    moveIntoObjectSpace(XilRoi*              roi,
                                             XilDeferrableObject* object);

    //
    //  Move into global space
    //
    virtual XilStatus    moveIntoGlobalSpace(XiliRect*            rect,
                                             XilDeferrableObject* object);

    virtual XilStatus    moveIntoGlobalSpace(XilRoi*              roi,
                                             XilDeferrableObject* object);

    //
    //  Special version of divideBoxList which takes into account backward and
    //  forward mapping.
    //
    virtual Xil_boolean  divideBoxList(XilBoxList*   boxlist,
                                       unsigned int  box_number,
                                       unsigned int  tile_xdelta,
                                       unsigned int  tile_ydelta);

    //
    //  The box to box cases are over-ridden
    //
    virtual XilStatus    gsBackwardMap(XiliRect*    dst_rect,
                                       XiliRect*    src_rect,
                                       unsigned int src_number);

    virtual XilStatus    gsForwardMap(XiliRect*     src_rect,
                                      unsigned int  src_number,
                                      XiliRect*     dst_rect);


    //
    // Backward map a single point
    //
    virtual XilStatus     vBackwardMap(XilBox*       dst_box,
                                       double        dx,
                                       double        dy,
                                       XilBox*       src_box,
                                       double*       sx,
                                       double*       sy,
                                       unsigned int  src_number);

private:
    //
    // Store away the fliptype for private use
    //
    XilFlipType         fliptype;

    //
    // Store away the source image xsize, ysize
    //
    unsigned int        src_xsize;
    unsigned int        src_ysize;
};
