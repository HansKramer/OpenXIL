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
//  File:	XiliBitField.hh
//  Project:	XIL
//  Revision:	1.3
//  Last Mod:	10:21:28, 03/10/00
//
//  Description:
//	
//	A class for managing a bit field.
//
//      NOTE:  Currently, the class is limited to a field of 32 bits.
//             Its implementation would need to be extended in order 
//             to support fields greater than 32.
//	
//  MT-level:  Un-Safe
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliBitField.hh	1.3\t00/03/10  "

#ifndef _XILI_BIT_FIELD_HH
#define _XILI_BIT_FIELD_HH

class XiliBitField {
public:
    //
    //  Set the nth bit in the field.
    //
    void             set(unsigned int n)
    {
        // bits[n/numBits] |=  ((unsigned)1 << (n % numBits));
        bits |= ((unsigned)1 << n);
    }

    //
    //  Clear the nth bit in the field.
    //
    void             clear(unsigned int n)
    {
        // bits[n/numBits] &= ~((unsigned)1 << (n % numBits));
        bits &= ~((unsigned)1 << n);
    }

    //
    //  Test whether the nth bit in the field is set.
    //
    Xil_boolean      isSet(unsigned int n) const
    {
        // return bits[n/numBits] &   ((unsigned)1 << (n % numBits));
        return bits & ((unsigned)1 << n);
    }

    //
    //  Zero all the bits in the field.
    //
    void             zero()
    {
        // xili_memset(bits, 0, sizeof(bits));
        bits  = 0;
    }

    //
    //  Get the bottom unsigned int of the bit field
    //
    unsigned int     get() const {
        return bits;
    }

    //
    //  Test if the bit field has any bits set
    //
    Xil_boolean      isAnySet() const {
        return bits != 0;
    }
    
    //
    //  Operators...
    //
    int              operator == (const XiliBitField& bf) const {
        return bits == bf.bits;
    }

    int              operator != (const XiliBitField& bf) const {
        return bits != bf.bits;
    }
    
    XiliBitField&    operator = (XiliBitField& bf) {
        bits = bf.bits;
        return *this;
    }
    
    XiliBitField&    operator |= (XiliBitField& bf) {
        bits |= bf.bits;
        return *this;
    }
    
    //
    //  Constructor/Destructor
    //
    //  TODO:  9/15/95 jlf  Could make the bit field size variable at
    //                      some cost in performance.
    //
                     XiliBitField() : numBits(sizeof(unsigned int)*8)
                     {
                         zero();
                     }
    
                     XiliBitField(XiliBitField& bf) : numBits(bf.numBits)
                     {
                         // memcpy(bits, bf.bits, numBits/8);
                         bits = bf.bits;
                     }
    
private:
    const unsigned int  numBits;
    unsigned int        bits;
    
#define _XILI_BF_SIZE           32
#define _XILI_BITS_PER_MASK     (sizeof (unsigned int) * 8)  // bits per mask
#define _XILI_NUM_MASKS(x, y)   (((x)+((y)-1))/(y))

//    unsigned int        bits[_XILI_NUM_MASKS(_XILI_BF_SIZE,_XILI_BITS_PER_MASK)];
};

#endif // _XILI_BIT_FIELD_HH
