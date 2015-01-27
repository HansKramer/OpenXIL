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
//  File:       ZigZag.hh
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:23:48, 03/10/00
//
//  Description:
//
//    ZigZag Block definition
//
//    TODO: Make this part of a class
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)ZigZag.hh	1.4\t00/03/10  "


#ifndef ZIGZAG_H
#define ZIGZAG_H

#include <xil/xilGPI.hh>

class ZigZagArray {
public:
    Xil_unsigned8* getArray() {return zigzag;}

private:
    static Xil_unsigned8 zigzag[64];
};

#endif // ZIGZAG_H
