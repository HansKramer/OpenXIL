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
//  File:	XilLookupSinglePrivate.hh
//  Project:	XIL
//  Revision:	1.6
//  Last Mod:	10:21:46, 03/10/00
//
//  Description:
//	Definition of private elements of XilLookupSingle Object
//	
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilLookupSinglePrivate.hh	1.6\t00/03/10  "

#ifdef _XIL_PRIVATE_DATA

public:
    //
    //  Calculates and returns a LUT that converts between the two LUTs "this"
    //    and dst. The resulting LUT's input datatype will be that of the
    //    input datatype of "this", and its output datatype will be that of
    //    the input datatype of dst.  The LUT's offset and number of entries
    //    are the same as those for "this."  Index N of the resulting LUT
    //    contains the index of the nearest color in dst to the color at index
    //    N in "this."  Nearest color is determined by Euclidean distance.
    //    Source and destination LUTs must have the same input datatypes,
    //    output datatypes, and number of bands.
    //    This also works with colorcubes, but the resulting conversion LUT
    //    is NOT a colorcube.
    //
    XilLookup*          convert(XilLookup* dst);

    //
    //  Returns a copy of the XilLookup object
    //
    XilObject*          createCopy();

    //
    //  Constructors
    //
                        XilLookupSingle(XilSystemState* system_state,
                                        XilDataType     input_type,
                                        XilDataType     output_type, 
                                        unsigned int    nbands,
                                        unsigned int    count,
                                        int             offset, 
                                        void*           data);

                        XilLookupSingle(XilSystemState* system_state,
                                        XilDataType     input_type,
                                        XilDataType     output_type, 
                                        unsigned int    nbands,
                                        int             offset); 

    //
    //  This needs to be public so that LookupCombined can deallocate the
    //  LookupSingle object copies that it holds. Lookup Combined is not a
    //  subclass of LookupSingle
    //
                        ~XilLookupSingle();

    //
    // Virtual function to set offset.
    // This function is called by setOffset().
    // It is overloaded in XilLookupColorcube to set
    // both the offset and the adjustedOffset.
    //
    virtual void        vSetOffset(int off);


protected:

    //
    //  Make access to these protected, so the derived class,
    //  XilLookupColorcube, can see them. 
    //
    double              euclideanDistanceSquare(Xil_unsigned8* p0,
                                                Xil_unsigned8* p1);

    int                 offset;
    unsigned int        entries;
    void*               data;

#endif // _XIL_PRIVATE_DATA
