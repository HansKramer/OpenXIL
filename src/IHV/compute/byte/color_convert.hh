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


#include "XilDeviceManagerComputeBYTE.hh"
#include "XiliUtils.hh"

#define FMASK  0x03ffe000
#define FBYTERANGE  255.0F

inline
float
_XILI_NORMALIZE_B(Xil_unsigned8 b)
{
    float tmp;

    tmp = _XILI_B2F(b) / FBYTERANGE;

    return tmp;
}

inline void
_XILI_QUANTIZE_PHOTO_B(float y601,
                       float cb601,
                       float cr601,
                       Xil_unsigned8* b1,
                       Xil_unsigned8* b2,
                       Xil_unsigned8* b3)
{
    //
    // 255 / 1.402 = 181.883
    //
    *b1 = _XILI_ROUND_U8(181.883F * y601);
    *b2 = _XILI_ROUND_U8(156.0F + 111.4F * cb601);
    *b3 = _XILI_ROUND_U8(137.0F + 135.64F * cr601);
}

inline float
_XILI_NORMALIZE_Y601_B(Xil_unsigned8 y601)
{
    float tmp;

    tmp = (_XILI_B2F(y601) - 16.0F) / 219.0F;

    return tmp;
}

inline
float
_XILI_NORMALIZE_Y709_B(Xil_unsigned8 y709)
{
    float tmp;

    tmp = (_XILI_B2F(y709) - 16.0F) / 219.0F;

    return tmp;
}

inline
void
_XILI_L_TO_CMY_B(Xil_unsigned8 red,
                 Xil_unsigned8 green,
                 Xil_unsigned8 blue,
                 Xil_unsigned8* c,
                 Xil_unsigned8* m,
                 Xil_unsigned8* y)
{
    //
    //  RGBLinear to cmy - byte case
    //
    *c = (255 - red);
    *m = (255 - green);
    *y = (255 - blue);
}

inline
void
_XILI_NL_TO_YCC601(float red,
                   float green,
                   float blue,
                   float* y601,
                   float* cb601,
                   float* cr601)
{
    // RGB Non-linear to ycc601

    // 0.0 <= y <= 1.0
    *y601 = 0.299F * red + 0.587F * green + 0.114F * blue;

    // -0.886 <= cb <= 0.886
    *cb601 = blue - *y601;


    // -0.701 <= cr <= 0.701
    *cr601 = red - *y601;
}

inline void
_XILI_NL_TO_YCC709(float red,
                   float green,
                   float blue,
                   float* y709,
                   float* cb709,
                   float* cr709)
{
    // RGB Non-linear to ycc709

    // 0.0 <= y <= 1.0
    *y709 = 0.2125F * red + 0.7154F * green + 0.0721F * blue;

    // -0.9279 <= cb <= 0.9279
    *cb709 = blue - *y709;

    // -0.7875 <= cr <= 0.7875
    *cr709 = red - *y709;
}

inline
void
_XILI_QUANTIZE_YCC601_B(float y601,
                        float cb601,
                        float cr601,
                        Xil_unsigned8* b1,
                        Xil_unsigned8* b2,
                        Xil_unsigned8* b3)
{
    *b1 = _XILI_ROUND_U8(16.0F  + 219.0F * y601);
    *b2 = _XILI_ROUND_U8(128.0F + 126.0F * cb601);
    *b3 = _XILI_ROUND_U8(128.0F + 160.0F * cr601);
}

inline
void
_XILI_QUANTIZE_YCC709_B(float y709,
                        float cb709,
                        float cr709,
                        Xil_unsigned8* b1,
                        Xil_unsigned8* b2,
                        Xil_unsigned8* b3)
{
    *b1 = _XILI_ROUND_U8( 16.0F + 219.0F * y709);
    *b2 = _XILI_ROUND_U8(128.0F + 121.0F * cb709);
    *b3 = _XILI_ROUND_U8(128.0F + 142.0F * cr709);
}

inline float
_XILI_NL_TO_Y601(float red, float green, float blue)
{
    //
    //  RGB Non-linear to y601
    //
    return 0.299F * red + 0.587F * green + 0.114F * blue;
}

//
// input: 0 <= y601 <= 255
// output: 0 <= b1 <= 255
//
inline
Xil_unsigned8
_XILI_QUANTIZE_Y601(float y601)
{
    Xil_unsigned8 tmp;

    tmp = _XILI_ROUND_U8(16.0F + 0.8588F * y601);

    return tmp;
}

inline float
_XILI_NL_TO_Y709(float red, float green, float blue)
{
    //
    //  RGB Non-linear to Y709
    //
    return 0.2125F * red + 0.7154F * green + 0.0721F * blue;
}

//
// input: 0 <= y709 <= 255
// output: 0 <= b1 <= 255
//
inline
Xil_unsigned8
_XILI_QUANTIZE_Y709(float y709)
{
    Xil_unsigned8 tmp;

    tmp = _XILI_ROUND_U8(16.0F + 0.8588F * y709);

    return tmp;
}

inline float
_XILI_L_TO_Ylinear(float red,
                   float green,
                   float blue)
{
    float tmp;

    // Linear to ylinear

    tmp = 0.299F * red + 0.587F * green + 0.114F * blue;

    return tmp;
}

inline void
_XILI_NORMALIZE_PHOTO_B(Xil_unsigned8 y601,
                        Xil_unsigned8 cb601,
                        Xil_unsigned8 cr601,
                        float* f1,
                        float* f2,
                        float* f3)
{
    //
    // 255 / 1.402 = 181.883
    //
    *f1 = _XILI_B2F(y601) / 181.883F;
    *f2 = (_XILI_B2F(cb601) - 156.0F) / 111.4F;
    *f3 = (_XILI_B2F(cr601) - 137.0F) / 135.64F;
}

inline void
_XILI_YCC601_TO_NL(float y601,
                   float cb601,
                   float cr601,
                   float* red_nl,
                   float* green_nl,
                   float* blue_nl)
{
    *red_nl = y601 + cr601;

    // 0.114 / 0.587 = 0.194, 0.299 / 0.587 = 0.509
    *green_nl = y601 - 0.194F * cb601 - 0.509F * cr601;

    *blue_nl = y601 + cb601;
}

inline
void
_XILI_NORMALIZE_YCC601_B(float y601,
                         float cb601,
                         float cr601,
                         float* f1,
                         float* f2,
                         float* f3)
{
    *f1 = (y601  -  16.0F) / 219.0F;
    *f2 = (cb601 - 128.0F) / 126.0F;
    *f3 = (cr601 - 128.0F) / 160.0F;
}

inline
void
_XILI_NORMALIZE_YCC709_B(Xil_unsigned8 y709,
                         Xil_unsigned8 cb709,
                         Xil_unsigned8 cr709,
                         float* f1,
                         float* f2,
                         float* f3)
{
    *f1 = (_XILI_B2F(y709)  -  16.0F) / 219.0F;
    *f2 = (_XILI_B2F(cb709) - 128.0F) / 121.0F;
    *f3 = (_XILI_B2F(cr709) - 128.0F) / 142.0F;
}

inline void
_XILI_YCC709_TO_NL(float y709,
                   float cb709,
                   float cr709,
                   float* red_nl,
                   float* green_nl,
                   float* blue_nl)
{
    // ycc709 to RGB Non-linear

    *red_nl = y709 + cr709;

    // 0.0721 / 0.7154 = 0.101, 0.2125 / 0.7154 = 0.297
    *green_nl = y709 - 0.101F * cb709 - 0.297F * cr709;

    *blue_nl = y709 + cb709;
}

inline
void
_XILI_CMY_TO_L_B(Xil_unsigned8 c,
                 Xil_unsigned8 m,
                 Xil_unsigned8 y,
                 Xil_unsigned8* red,
                 Xil_unsigned8* green,
                 Xil_unsigned8* blue)
{
    //
    //  cmy to RGBlinear
    //
    *red   = 255 - c;
    *green = 255 - m;
    *blue  = 255 - y;
}

inline
void
_XILI_CMYK_TO_L_B(Xil_unsigned8 c,
                  Xil_unsigned8 m,
                  Xil_unsigned8 y,
                  Xil_unsigned8 k,
                  Xil_unsigned8* red,
                  Xil_unsigned8* green,
                  Xil_unsigned8* blue)
{
    Xil_unsigned8 tmpc = _XILI_CLAMP_U8((int)c + (int)k);
    Xil_unsigned8 tmpm = _XILI_CLAMP_U8((int)m + (int)k);
    Xil_unsigned8 tmpy = _XILI_CLAMP_U8((int)y + (int)k);

    *red   = 255 - tmpc;
    *green = 255 - tmpm;
    *blue  = 255 - tmpy;
}

inline Xil_unsigned8
_XILI_QUANTIZE_B(float f)
{
    Xil_unsigned8 tmp;

    tmp = _XILI_ROUND_U8(f * FBYTERANGE);

    return tmp;
}

//
// input: 0 <= y601 <= 1.0
// output: 0 <= b1 <= 255
//
inline Xil_unsigned8
_XILI_QUANTIZE_Y601_B(float y601)
{
    Xil_unsigned8 tmp;

    tmp = _XILI_ROUND_U8(16.0F + 219.0F * y601);

    return tmp;
}

//
// input: 0 <= y709 <= 1.0
// output: 0 <= b1 <= 255
//
inline
Xil_unsigned8
_XILI_QUANTIZE_Y709_B(float y709)
{
    Xil_unsigned8 tmp;

    tmp = _XILI_ROUND_U8(16.0F + 219.0F * y709);

    return tmp;
}

inline
void
_XILI_CMY_TO_L(float c,
               float m,
               float y,
               float* red,
               float* green,
               float* blue)
{
    //
    // cmy to RGBlinear
    //
    *red   = 1.0F - c;
    *green = 1.0F - m;
    *blue  = 1.0F - y;
}

inline
void
_XILI_CMYK_TO_L(float c,
                float m,
                float y,
                float k,
                float* red,
                float* green,
                float* blue)
{
    //
    //  cmyk ro RGBlinear
    //
    c += k;
    m += k;
    y += k;

    float tmpc = (c > 1.0F ? 1.0F : c);
    float tmpm = (m > 1.0F ? 1.0F : m);
    float tmpy = (y > 1.0F ? 1.0F : y);

    *red   = 1.0F - tmpc;
    *green = 1.0F - tmpm;
    *blue  = 1.0F - tmpy;
}

inline
float
_XILI_L_TO_NL_1(float red)
{
    float tmp;

    if(red >= 0.018F) {
        tmp = (float) (1.099F * pow(red, 0.45F) - 0.099F);

    } else if(red <= -0.018F) {
        tmp = (float) (-1.099F * pow(-red, 0.45F) + 0.099F);

    } else {
        tmp = red * 4.5F;
    }

    return tmp;
}

//
//  A macro because the compiler can't handle inlining the complex code.
//
#define _XILI_L_TO_NL(red, green, blue, red_nl, green_nl, blue_nl) \
    if(red >= 0.018F) { \
        *red_nl = (float)(1.099F * pow((red), 0.45F) - 0.099F);\
    } else if(red <= -0.018F) { \
        *red_nl = (float)(-1.099F * pow(-(red), 0.45F) + 0.099F);\
    } else { \
        *red_nl = red * 4.5F;\
    } \
    \
    if(green >= 0.018F) { \
        *green_nl = (float) (1.099F * pow(green, 0.45F) - 0.099F);\
    } else if((green) <= -0.018) { \
        *green_nl = (float)(-1.099F * pow(-green, 0.45F) + 0.099F);\
    } else { \
      *green_nl = green * 4.5F;\
    } \
    \
    if(blue >= 0.018F) { \
        *blue_nl = (float)(1.099F * pow(blue, 0.45F) - 0.099F);\
    } else if(blue <= -0.018F) { \
        *blue_nl = (float)(-1.099F * pow(-blue, 0.45F) + 0.099F);\
    } else { \
      *blue_nl = blue * 4.5F;\
    }


