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
//  File:	XilOpPoint.hh
//  Project:	XIL
//  Revision:	1.6
//  Last Mod:	10:20:31, 03/10/00
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
#pragma ident	"@(#)XilOpPoint.hh	1.6\t00/03/10  "

#ifndef _XIL_OP_POINT_HH
#define _XIL_OP_POINT_HH

#include <xil/xilGPI.hh>
#include "XiliOpUtils.hh"

class XilOpPoint : public XilOp {
protected:
            XilOpPoint(XilOpNumber op_number)
                : XilOp(op_number)
    {
    }

    virtual ~XilOpPoint()
    {
    }
};

#endif // _XIL_OP_POINT_HH
