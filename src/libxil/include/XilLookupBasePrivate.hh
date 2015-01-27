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
//  File:	XilLookupBasePrivate.hh
//  Project:	XIL
//  Revision:	1.7
//  Last Mod:	10:21:50, 03/10/00
//
//  Description:
//	Definition of private elements of XilLookup base Object
//	
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilLookupBasePrivate.hh	1.7\t00/03/10  "

#ifdef _XIL_PRIVATE_DATA

public:
    //
    // Pure virtual function for lookup conversion
    // Implementation valid only in LookupSingle and LookupColorcube
    //
    virtual XilLookup*  convert(XilLookup* dst_lut) = 0;
    
    //
    //  Constructors
    //
                    XilLookup(XilSystemState* system_state,
                              XilDataType     input_type,
                              XilDataType     output_type);

                    XilLookup(XilSystemState* system_state);

protected:
                    ~XilLookup();

    //
    // Make access to these protected so subclasses can see them
    //
    XilDataType     inputType;
    XilDataType     outputType;
    XilLookupType   lookupType;
    unsigned int    inputNBands;
    unsigned int    outputNBands;
    unsigned int    bytesPerEntry;
    unsigned int    bytesPerBand;
    unsigned int    maxSize;
    Xil_boolean     isColorcubeFlag;

    XilMutex        lookupMutex;

#endif // _XIL_PRIVATE_DATA
