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
//  File:	XiliMarker.hh
//  Project:	XIL
//  Revision:	1.5
//  Last Mod:	10:21:28, 03/10/00
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
//  MT-level:  <??????>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliMarker.hh	1.5\t00/03/10  "

#ifndef _XILI_MARKER_HH
#define _XILI_MARKER_HH

#include "_XilDefines.h"
#include "XiliBitField.hh"
#include "XiliList.hh"

//
//  Forward declare this class from XilFunctionInfoPrivate.hh
//
class XiliDirection;

//------------------------------------------------------------------------
//
//  Class:	XiliMarker
//
//  Description:
//	
//    In the function definition is this direction list which provides
//    information on how to traverse the molecule once the function is
//    found.  bitField is a field that as markers are picked up, they
//    are or'd together.  So, for a particular functionNumber, a final
//    result is obtained if all of the markers are picked up verifying
//    all of the paths of the molecule have been visited and thus, the
//    function can be used. 
//
//	
//  Notes:
//	
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
class XiliMarker {
public:
    //
    //  Constructor/Destructor
    //
                          XiliMarker()
    {
    }
    
                          XiliMarker(unsigned int  fnum)
    {
        functionNumber = fnum;
    }
    
                          ~XiliMarker()
    {
    }

    //
    //  Equality test operator
    //
    int                   operator==(const XiliMarker& rval) const
    {
        return functionNumber == rval.functionNumber;
    }

#ifdef DEBUG
    //
    //  Output the contents of the object to stderr
    //
    void                  dump() const
    {
        fprintf(stderr, "funcNum=  %d;  bitField=  %x\n",
                functionNumber, bitField.get());
    }
#endif

    //
    //  Public data members -- used by XilOpPrivate.cc
    //
    unsigned int          functionNumber;
    XiliBitField          bitField;
    
    //
    //  The direction list here is used to determine exactly which function
    //    this marker is for.  It is only used by the insert algorithm to
    //    determine whether to leave a new marker or use one that is already
    //    here...
    //
    XiliList<XiliDirection>* directionList;

    XiliMarker*        next;
};

#endif // _XILI_MARKER_HH
