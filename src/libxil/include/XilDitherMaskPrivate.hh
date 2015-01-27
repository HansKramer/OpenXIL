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
//  File:	XilDitherMaskPrivate.hh
//  Project:	XIL
//  Revision:	1.5
//  Last Mod:	10:21:40, 03/10/00
//
//  Description:
//	Definition of private elements of XilDitherMask Object
//	
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDitherMaskPrivate.hh	1.5\t00/03/10  "

#ifdef _XIL_PRIVATE_DATA

public:
    //
    //  Required virtual functions from XilObject
    //
    XilObject*      createCopy();


    //
    //  Constructor
    //
                    XilDitherMask(XilSystemState* system_state,
                                   unsigned int    xsize,
                                   unsigned int    ysize,
                                   unsigned int    num_bands,
                                   float*          data);

protected:
                    ~XilDitherMask();


private:
    unsigned int    width;
    unsigned int    height;
    unsigned int    nBands;
    float*          data;
    
#endif // _XIL_PRIVATE_DATA
