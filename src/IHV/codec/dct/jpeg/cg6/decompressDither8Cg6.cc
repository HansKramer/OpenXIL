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
//  File:       decompressDither8Cg6.cc
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:14:23, 03/10/00
//
//  Description:
//
//    GX Molecules
//
//    This class is the definition of Jpeg compression and
//    decompression within XIL.  There is one of these objects per
//    Baseline Jpeg cis.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)decompressDither8Cg6.cc	1.3\t00/03/10  "

#include <xil/xilGPI.hh>
#include "XilDeviceManagerComputeJpegCg6.hh"

XilStatus
XilDeviceManagerComputeJpegCg6::decompressDither8Cg6(
    XilOp*       , 
    unsigned int , 
    XilRoi*      , 
    XilBoxList*  )
{
    // Not implemented yet - return failure
    return XIL_FAILURE;
}

