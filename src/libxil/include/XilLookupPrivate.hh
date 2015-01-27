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
//  File:	XilLookupPrivate.hh
//  Project:	XIL
//  Revision:	1.4
//  Last Mod:	07:24:09, 09/27/95
//
//  Description:
//	Definition of private elements of XilLookup Object
//	
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilLookupPrivate.hh	1.4\t95/09/27  "

#ifdef _XIL_PRIVATE_DATA

public:
    XilObjectType    getObjectType() const;

    //
    //  Constructors
    //
    XilLookup(XilSystemState* system_state,
              XilDataType     input_type,
              XilDataType     output_type,
              unsigned int    nbands,
              short           offset,
              int             multipliers[],
              unsigned int    dimensions[]);

    XilLookup(XilSystemState* system_state,
              XilDataType     input_type,
              XilDataType     output_type, 
              unsigned int    nbands,
              unsigned int    count,
              short           offset, 
              void*           data);

    //
    //  Constructor to combine multiple lookups into a single lookup object.
    //
    XilLookup(XilSystemState* system_state,
              XilLookup*      list[],
              unsigned int    num_lookups);

    XilLookup(XilSystemState* system_state,
              XilDataType     input_type,
              XilDataType     output_type,
              unsigned int    nbands,
              unsigned int*   entriesList,
              short*          offsetList);

    Xil_boolean isOK();

protected:
    ~XilLookup();


private:
    Xil_boolean     isOKFlag;

    XilDataType     input;
    XilDataType     output;
    unsigned int    nBands;
    short           offset;
    unsigned int    entries;
    unsigned int    bytesPerEntry;
    Xil_boolean     isColorCubeFlag;
    int*            multipliers;
    unsigned int*   dimensions;
    void*           data;
    
    //
    //  For n band -> n band (combined) lookups
    //
    short*          offsetsList;
    unsigned int*   entriesList;
    void**          dataList;

#endif // _XIL_PRIVATE_DATA
