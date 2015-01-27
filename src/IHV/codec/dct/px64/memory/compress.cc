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
//  File:       compress.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:15:20, 03/10/00
//
//  Description:
//
//    H261 Compression - not implemented
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)compress.cc	1.2\t00/03/10  "

#include <xil/xilGPI.hh>
#include "XilDeviceCompressionH261.hh"

XilStatus 
XilDeviceCompressionH261::compress(XilOp*       ,
                                   unsigned int ,
                                   XilRoi*      ,
                                   XilBoxList*  )
{
    //
    // Currently the compression half of the H261 Codec 
    // is not implemented in XIL
    //
    setInMolecule(FALSE);

    XIL_CIS_ERROR(XIL_ERROR_CIS_DATA, "di-313", TRUE, this, FALSE, TRUE);

    return XIL_FAILURE;
}



