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
//  File:	XilOpAreaFill.hh
//  Project:	XIL
//  Revision:	1.11
//  Last Mod:	10:20:37, 03/10/00
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
#pragma ident	"@(#)XilOpAreaFill.hh	1.11\t00/03/10  "

#ifndef _XIL_OP_AREA_FILL_HH
#define _XIL_OP_AREA_FILL_HH

#include "XilOpArea.hh"

class XilOpAreaFill : public XilOpArea {
protected:
    XilOpAreaFill(XilOpNumber op_number,
                  XilImage*   source_image) : 
	XilOpArea(op_number)
    {
        src_image = source_image;
        src_xsize = source_image->getWidth();
        src_ysize = source_image->getHeight();
    }

    virtual             ~XilOpAreaFill()
    {
    }

    //
    // Fill isn't forward mappable
    //
    virtual Xil_boolean isForwardMappable()
    {
        return FALSE;
    }

    //
    // Fill operations shouldn't be split for threads
    //
    virtual Xil_boolean canBeSplit()
    {
        return FALSE;
    }

    //
    //  Splitting on tile boundaries isn't available to fill operations since
    //  they may need the entire source there isn't a generic algorithm that
    //  permits the source to be split.
    //
    virtual XilStatus   vSplitOnTileBoundaries(XilBoxList* )
    {
        return XIL_FAILURE;
    }

    //    
    //  Over-riding setBoxStorage map to set the source storage area to
    //  be the entire source.
    //
    virtual XilStatus setBoxStorage(XiliRect*            rect,
                                    XilDeferrableObject* object,
                                    XilBox*              box);

    XilImage*    src_image;
    unsigned int src_xsize;
    unsigned int src_ysize;
};

#endif // _XIL_OP_AREA_FILL_HH
