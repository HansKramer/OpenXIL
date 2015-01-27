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
//  File:	XilLookupColorcubePrivate.hh
//  Project:	XIL
//  Revision:	1.5
//  Last Mod:	10:21:53, 03/10/00
//
//  Description:
//	Definition of private elements of XilLookupColorcube Object
//	
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilLookupColorcubePrivate.hh	1.5\t00/03/10  "

#ifdef _XIL_PRIVATE_DATA

public:
    //
    //  Returns a copy of the XilLookup object
    //
    XilObject*      createCopy();

    //
    //  Returns information on the colorcube formatting if this LUT is a
    //    colorcube.
    //
    Xil_boolean     getColorcubeInfo(int*          multipiers,
                                     unsigned int* dimensions,
                                     short*        adjusted_offset);

    //
    //  Constructors
    //
                    XilLookupColorcube(XilSystemState* system_state,
                                       XilDataType     input_type,
                                       XilDataType     output_type,
                                       unsigned int    nbands,
                                       int             offset,
                                       int*            multipliers,
                                       unsigned int*   dimensions);

                    XilLookupColorcube(XilSystemState*     system_state,
                                       XilLookupColorcube* orig_cube);

    //
    // Virtual function to set offset.
    // This function is called by setOffset().
    // Here it sets both the offset and the adjustedOffset.
    //
    virtual void        vSetOffset(int off);

protected:
                       ~XilLookupColorcube();


private:
    int*            multipliers;
    unsigned int*   dimensions;
    unsigned int*   dimsMinus1;
    int*            colorcube_arrays;
    int             adjustedOffset;

#endif // _XIL_PRIVATE_DATA
