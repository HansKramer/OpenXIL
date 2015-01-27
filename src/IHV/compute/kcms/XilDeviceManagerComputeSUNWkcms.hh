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
//  File:	XilDeviceManagerComputeSUNWkcms.hh
//  Project:	XIL
//  Revision:	1.7
//  Last Mod:	10:22:28, 03/10/00
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
#pragma ident	"@(#)XilDeviceManagerComputeSUNWkcms.hh	1.7\t00/03/10  "

#ifndef _XIL_DEVICE_MANAGER_COMPUTE_SUNWKCMS_HH
#define _XIL_DEVICE_MANAGER_COMPUTE_SUNWKCMS_HH

#include <xil/xilGPI.hh>
#include <kcms/kcs.h>

class XilDeviceManagerComputeSUNWkcms : public XilDeviceManagerCompute {
public:
    //
    //  Constructor/Destructor
    //
                   XilDeviceManagerComputeSUNWkcms();
                   ~XilDeviceManagerComputeSUNWkcms();

    //
    //  Required Virtual Functions
    //
    const char*    getDeviceName();
    XilStatus      describeMembers();
    
    //
    //  Compute Routines
    //
    XilStatus       ColorCorrectPreprocess(XilOp*        op,
					   unsigned int  op_count,
					   XilRoi*       roi,
					   void**        compute_data,
					   unsigned int* func_ident);

    XilStatus       ColorCorrect(XilOp*       op,
				 unsigned     op_count,
				 XilRoi*      roi,
				 XilBoxList*  bl);

private:
    Xil_boolean     isRGB(KcsProfileId profile, int key);
};
#endif // _XIL_DEVICE_MANAGER_COMPUTE_SUNWKCMS_HH
