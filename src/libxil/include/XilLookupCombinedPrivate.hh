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
//  File:	XilLookupCombinedPrivate.hh
//  Project:	XIL
//  Revision:	1.3
//  Last Mod:	10:21:48, 03/10/00
//
//  Description:
//	Definition of private elements of XilLookupCombined Object
//	
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilLookupCombinedPrivate.hh	1.3\t00/03/10  "

#ifdef _XIL_PRIVATE_DATA

public:
    //
    //  Returns a copy of the XilLookup object
    //
    XilObject*        createCopy();

    XilLookup*        convert(XilLookup* dst_lut);

    //
    //  Constructors
    //

    //
    //  Constructor to combine multiple XilLookupSingle's
    //  into a single XilLookupCombined object.
    //
                      XilLookupCombined(XilSystemState*  system_state,
                                        XilLookupSingle* list[],
                                        unsigned int     num_lookups);

protected:
                      ~XilLookupCombined();


private:
    //
    //  For n band -> n band (combined) lookups
    //
    int*              offsetsList;
    unsigned int*     entriesList;
    XilLookupSingle** lutList;
    const void**      dataList;

#endif // _XIL_PRIVATE_DATA
