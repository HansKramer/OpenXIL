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
//  File:	XilOpAreaFill.cc
//  Project:	XIL
//  Revision:	1.12
//  Last Mod:	10:07:34, 03/10/00
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
#pragma ident	"@(#)XilOpAreaFill.cc	1.12\t00/03/10  "

#include <xil/xilGPI.hh>
#include "XilOpAreaFill.hh"

//
//  setBoxStorage() sets the box to be the entire image for the source.
//
XilStatus
XilOpAreaFill::setBoxStorage(XiliRect*            rect,
                             XilDeferrableObject* defobj,
                             XilBox*              box)
{
    *box = *rect;

    if(defobj == src_image) {
        box->setStorageLocation(0, 0, src_xsize, src_ysize, 0);
    }

    return defobj->setBoxStorage(box);
}
