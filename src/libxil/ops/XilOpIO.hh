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
//  File:	XilOpIO.hh
//  Project:	XIL
//  Revision:	1.9
//  Last Mod:	10:20:39, 03/10/00
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
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilOpIO.hh	1.9\t00/03/10  "

#include "XilOpPoint.hh"

class XilOpIO : public XilOpPoint {
public:
    virtual XilStatus     switchToAlternateOp();

    virtual XilStatus     moveIntoObjectSpace(XiliRect*            rect,
                                              XilDeferrableObject* object);

    virtual XilStatus     moveIntoObjectSpace(XilRoi*              roi,
                                              XilDeferrableObject* object);

    virtual XilStatus     moveIntoGlobalSpace(XiliRect*            rect,
                                              XilDeferrableObject* object);
    
    virtual XilStatus     moveIntoGlobalSpace(XilRoi*              roi,
                                              XilDeferrableObject* object);

    virtual XilStatus     clipToTile(XilDeferrableObject* defobj,
                                     XilTileNumber        tile_number,
                                     XiliRect*            rect);

    virtual XilStatus     readjustBoxStorage(XiliRect* dst_rect);

    //
    //  Return display or capture depending on which it is.
    //
    virtual const char*   getOpName() = 0;

protected:
                          XilOpIO(XilOpNumber op_num,
                                  XilOp*      constructing_op) :
                              XilOpPoint(op_num)
    {
        controllingImage     = NULL;
        realImage            = NULL;
        realOp               = constructing_op;
        mutatedToDefaultFlag = FALSE;
        translatedROI        = FALSE;
        translateX           = 0;
        translateY           = 0;
    }

    virtual               ~XilOpIO()
    {
    }

    //
    //  The device's controlling image...
    //
    XilImage*             controllingImage;

    //
    //  The operation constructing/using the capture/display
    //
    XilOp*                realOp;
    XilImage*             realImage;

    //
    //  Whether we've translated the ROI to the controlling image and how much
    //  we need to translate by. 
    //
    Xil_boolean           translatedROI;
    int                   translateX;
    int                   translateY;

private:
    Xil_boolean           mutatedToDefaultFlag;
};

