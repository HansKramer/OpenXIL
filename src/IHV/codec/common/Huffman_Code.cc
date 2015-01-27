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
//  File:       Huffman_Code.cc
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:16:09, 03/10/00
//
//  Description:
//
//    Implementation of Huffman_Code class member functions and
//    friend operators.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Huffman_Code.cc	1.3\t00/03/10  "


#include "xil/xilGPI.hh"
#include "Huffman_Code.hh"

//------------------------------------------------------------------------
//
//  Function:        operator =
//  Created:        92/03/20
//
//  Description:
//        
//    Sets value to that associated with parameter code
//        
//  Parameters:
//
//    const Huffman_Code& hc:  this code is what is being used to set values
//        
//  Returns:
//        
//------------------------------------------------------------------------
void Huffman_Code::operator=(const Huffman_Code& hc)
{
    length = hc.length;
    code   = hc.code;
}


//------------------------------------------------------------------------
//
//  Function:        operator ==
//  Created:        92/03/20
//
//  Description:
//
//   Determines if two Huffman_Codes are equal or not
//        
//  Parameters:
//
//    const Huffman_Code& hc1
//    const Huffman_Code& hc2
//        
//  Returns:
//
//    int     value (1 1, 0 0). 1 if length and code are the
//    same, 0 otherwise.
//
//------------------------------------------------------------------------
int     operator==(const Huffman_Code& hc1, const Huffman_Code& hc2)
{
    if((hc1.length != hc2.length) || (hc1.code != hc2.code)) {
        return 0;
    } else {
        return 1;
    }
}


//------------------------------------------------------------------------
//
//  Function:        operator !=
//  Created:        92/03/20
//
//  Description:
//
//   Determines if two Huffman_Codes are not equal 
//        
//  Parameters:
//
//    const Huffman_Code& hc1
//    const Huffman_Code& hc2
//        
//  Returns:
//
//    int     value 0 if length and code are the same, 1 otherwise.
//    
//
//------------------------------------------------------------------------
int     operator!=(const Huffman_Code& hc1, const Huffman_Code& hc2)
{
    return (!(hc1 == hc2));
}

