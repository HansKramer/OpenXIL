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
//  File:	XilOpArea.hh
//  Project:	XIL
//  Revision:	1.4
//  Last Mod:	10:20:30, 03/10/00
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
#pragma ident	"@(#)XilOpArea.hh	1.4\t00/03/10  "

#ifndef _XIL_OP_AREA_HH
#define _XIL_OP_AREA_HH

#include <xil/xilGPI.hh>
#include "XiliOpUtils.hh"


class XilOpArea : public XilOp {
protected:
    XilOpArea(XilOpNumber op_number);
    virtual ~XilOpArea();
};

#endif // _XIL_OP_AREA_HH
