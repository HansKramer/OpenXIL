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
//  File:       ColorValue.hh
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:23:34, 03/10/00
//
//  Description:
//
//        Contains routines for converting between yuv and rgb
//        color space.  All of these functions are inlined for maximum
//        speed.
//        
//        Also contains the ColorValue class.  This class is
//        currently used to provide a uniform interface to color pixels.
//
//        There are numerous operators that continue to grow for
//        using a ColorValue as a generic type like 'int'.
//
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)ColorValue.hh	1.3\t00/03/10  "

#ifndef ColorValue_H
#define ColorValue_H

#include "xil/xilGPI.hh"

#ifdef TODO
#include "SquaresTable.hh"
#endif

#define CLAMP(X) (((X) & ~255) ? (((X) < 0) ? 0 : 255) : (X))

#define  YR_VAL  0.256635
#define  YG_VAL  0.503971
#define  YB_VAL  0.098218

#define  UR_VAL -0.148195
#define  UG_VAL -0.290921
#define  UB_VAL  0.439216

#define  VR_VAL  0.439216
#define  VG_VAL -0.367580
#define  VB_VAL -0.071637

#define  RY_VAL  1.164385
#define  RV_VAL  1.596431

#define  GY_VAL  1.164382
#define  GU_VAL -0.392973
#define  GV_VAL -0.812944

#define  BY_VAL  1.164383
#define  BU_VAL  2.016403

inline
Xil_unsigned8
rgb2y(Xil_unsigned8 r, Xil_unsigned8 g, Xil_unsigned8 b)
{
    int t;

    t =  int((float) YR_VAL * (float) r
           + (float) YG_VAL * (float) g
           + (float) YB_VAL * (float) b
           + (float) 16.0 + (float) 0.5);

    return t;
}


inline
Xil_unsigned8
rgb2u(Xil_unsigned8 r, Xil_unsigned8 g, Xil_unsigned8 b)
{
    int t;

    t =  int((float) UR_VAL * (float) r
           + (float) UG_VAL * (float) g
           + (float) UB_VAL * (float) b
           + (float) 128.0 + (float) 0.5);

    return t;
}

inline
Xil_unsigned8
rgb2v(Xil_unsigned8 r, Xil_unsigned8 g, Xil_unsigned8 b)
{
    int t;

    t =  int((float) VR_VAL * (float) r
           + (float) VG_VAL * (float) g
           + (float) VB_VAL * (float) b
           + (float) 128.0 + (float) 0.5);

    return t;
}

inline
Xil_unsigned8
yuv2r(Xil_unsigned8 y, Xil_unsigned8, Xil_unsigned8 v)
{
    int t = int((float) RY_VAL  * ((float)y-16.0)  +
                (float) RV_VAL  * ((float)v-128.0) +
                (float) 0.5);

    return (CLAMP(t));
}

inline
Xil_unsigned8
yuv2g(Xil_unsigned8 y, Xil_unsigned8 u, Xil_unsigned8 v)
{
    int t =  int((float) GY_VAL  * ((float)y-16.0)  +
                 (float) GU_VAL  * ((float)u-128.0) +
                 (float) GV_VAL  * ((float)v-128.0) +
                 (float) 0.5);

    return (CLAMP(t));
}


inline
Xil_unsigned8
yuv2b(Xil_unsigned8 y, Xil_unsigned8 u, Xil_unsigned8)
{
    int t = int((float) BY_VAL  *  ((float)y-16.0)  +
                (float) BU_VAL  *  ((float)u-128.0) +
                (float) 0.5);

    return (CLAMP(t));
}

//------------------------------------------------------------------------
//
//  Class:        ColorValue
//  Created:        92/04/21
//
// Description:
//  This class is intended to provide a fairly easy/uniform access
//  to colormap entries which includes conversion of an entry from RGB
//  to YUV color space.  A ColorValue has four single-byte bands for color.
//
//      The structure is treated like an int for speed.  It is
//      expected to remain the size of an int in order to maintain
//      speed and eliminate padding when used in multi-dimentional
//      arrays.
//        
// Notes:
//        
//        
// Deficiencies/TODO:
//        A three banded class might be useful too.
//        
//------------------------------------------------------------------------
class ColorValue {
public:
    ColorValue(void);

    ColorValue(const ColorValue& e);

    void RGBtoYUV(void) {
        int y = rgb2y(b0, b1, b2);
        int u = rgb2u(b0, b1, b2);
        int v = rgb2v(b0, b1, b2);

        b0 = y;
        b1 = u;
        b2 = v;
    }

    void YUVtoRGB(void) {
        Xil_unsigned8 t0, t1, t2;

        t0 = yuv2r(b0, b1, b2);
        t1 = yuv2g(b0, b1, b2);
        t2 = yuv2b(b0, b1, b2);

        b0 = t0;
        b1 = t1;
        b2 = t2;
    }

    void BGRtoYUV(void) {
        int y = rgb2y(b2, b1, b0);
        int u = rgb2u(b2, b1, b0);
        int v = rgb2v(b2, b1, b0);

        b0 = y;
        b1 = u;
        b2 = v;
    }

    void YUVtoBGR(void) {
        Xil_unsigned8 t0, t1, t2;

        t0 = yuv2r(b2, b1, b0);
        t1 = yuv2g(b2, b1, b0);
        t2 = yuv2b(b2, b1, b0);

        b0 = t2;
        b1 = t1;
        b2 = t0;
    }

    void RGBtoBGR(void) {
        Xil_unsigned8 t = b0;
        b0 = b2;
        b2 = t;
    }

    void BGRtoRGB(void) {
        Xil_unsigned8 t = b0;
        b0 = b2;
        b2 = t;
    }

    Xil_unsigned8&  band0(void) {
        return b0;
    }

    Xil_unsigned8&  band1(void) {
        return b1;
    }

    Xil_unsigned8&  band2(void) {
        return b2;
    }

    Xil_unsigned8   band0(void) const {
        return b0;
    }

    Xil_unsigned8   band1(void) const {
        return b1;
    }

    Xil_unsigned8   band2(void) const {
        return b2;
    }

    int             distance(const ColorValue& color) {
#ifdef TODO
      // Replace after sqrs_table is in
        int dist = sqrs_table[this->band0()-color.band0()] +
                   sqrs_table[this->band1()-color.band1()] +
                   sqrs_table[this->band2()-color.band2()];
#endif
        int dist = ((this->band0()-color.band0()) * (this->band0()-color.band0())) +
                   ((this->band1()-color.band1()) * (this->band1()-color.band1())) +
                   ((this->band2()-color.band2()) * (this->band2()-color.band2()));

        return dist;
    }

    ColorValue& operator= (const ColorValue& rval) {
        *((int*)this) = (*(const int*)&rval);
        return *this;
    }

    ColorValue& operator= (const int rval) {
        *((int*)this) = rval;
        return *this;
    }

    ColorValue& operator= (Xil_unsigned8 rval) {
        b0 = b1 = b2 = rval;
        return *this;
    }

    void        setColor(Xil_unsigned8 inb0, Xil_unsigned8 inb1,
                         Xil_unsigned8 inb2) {
        b0 = inb0;
        b1 = inb1;
        b2 = inb2;
    }

    int         operator== (ColorValue& rval) {
        return *((int*)this) == *((int*)&rval);
    }

    int         operator!= (ColorValue& rval) {
        return *((int*)this) != *((int*)&rval);
    }

    ColorValue& operator+= (const ColorValue& rval) {
        b0  += rval.b0;
        b1  += rval.b1;
        b2  += rval.b2;

        return *this;
    }

    ColorValue& operator^= (const ColorValue& rval) {
        *((int*)this) ^= *((const int*)&rval);
        return *this;
    }

    ColorValue& operator+= (int rval) {
        b0  += rval;
        b1  += rval;
        b2  += rval;

        return *this;
    }

    ColorValue  operator+  (const ColorValue& rval) {
        ColorValue  tmp;
        ColorValue* ptmp = &tmp;
        *((int*)ptmp) = *((int*)this) + *((const int*)&rval);
        return *ptmp;
    }

    ColorValue  operator+  (int rval) {
        ColorValue tmp;

        tmp.b0 = b0 + rval;
        tmp.b1 = b1 + rval;
        tmp.b2 = b2 + rval;

        return tmp;
    }

    ColorValue  operator/  (int rval) {
        ColorValue tmp;

        tmp.b0 = ((int)b0 / rval);
        tmp.b1 = ((int)b1 / rval);
        tmp.b2 = ((int)b2 / rval);

        return tmp;
    }

    ColorValue  operator*  (int rval) {
        ColorValue tmp;

        tmp.b0 = b0 * rval;
        tmp.b1 = b1 * rval;
        tmp.b2 = b2 * rval;

        return tmp;
    }

protected:
    Xil_unsigned8   b3;  //  Just here for byte-boundry padding and speed
    Xil_unsigned8   b0;
    Xil_unsigned8   b1;
    Xil_unsigned8   b2;

};

#endif  // ColorValue_H
