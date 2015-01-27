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
//  File:       JpegByteArray.hh
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:23:45, 03/10/00
//
//  Description:
//
//    JpegByteArray Class. 
//    A very simple array class, which tracks its own length.
//    The max size must be specified a construction time.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)JpegByteArray.hh	1.2\t00/03/10  "


#ifndef JPEG_BYTE_ARRAY_HH
#define JPEG_BYTE_ARRAY_HH

#include <xil/xilGPI.hh>

class JpegByteArray {
public:

    JpegByteArray(int array_size);
    ~JpegByteArray();

    XilStatus append(Xil_unsigned8 element);

    Xil_unsigned8 retrieve(int position);

    int length();

    Xil_boolean isOK();

private:
    Xil_unsigned8* theArray;
    int            array_size;
    int            current_length;
    Xil_boolean    isOKFlag;
};

#endif // JPEG_BYTE_ARRAY_HH
