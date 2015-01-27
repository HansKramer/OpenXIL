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
//   File:	_XilRoi.hh
//   Project:	XIL
//   Revision:	1.14
//   Last Mod:	10:21:19, 03/10/00
//  
//  Description:
//	
//	XilRoi object which describes a region of interest on an
//      XIL image.
//	
//  MT-level:  UNSAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilRoi.hh	1.3\t95/06/30  "


#include "_XilNonDeferrableObject.hh"

//
//  Private Includes
//
#ifdef  _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_INCLUDES

#include "XilRoiPrivate.hh"

#undef _XIL_PRIVATE_INCLUDES
#endif

#ifndef _XIL_ROI_HH
class XilRoi : public XilNonDeferrableObject {
public:
protected:

private:
#ifdef _XIL_LIBXIL_PRIVATE
#define _XIL_PRIVATE_DATA

#include "XilRoiPrivate.hh"

#undef _XIL_PRIVATE_DATA
#else
    //
    //  Make it clear to the compiler that a destructor exists.  This is done
    //  so GPI users will get a compile-time error if they attempt to delete
    //  this class.
    //
                        ~XilRoi();
#endif // _XIL_PRIVATE_DATA
};

#define _XIL_ROI_HH
#endif // _XIL_ROI_HH


