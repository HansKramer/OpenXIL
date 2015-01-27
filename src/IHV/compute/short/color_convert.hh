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


#include "XiliUtils.hh"

#define SHORTRANGE 65535
#define FSHORTRANGE 65535.0F

inline void
_XILI_Y601_TO_NL(float y601, float* red_nl, float* green_nl, float* blue_nl) 
{
    *red_nl = *green_nl = *blue_nl = y601;
}

inline void
_XILI_Y709_TO_NL(float y709, float* red_nl, float* green_nl, float* blue_nl)
{
    *red_nl = *green_nl = *blue_nl = y709;
}

inline
void
_XILI_L_TO_CMY(float red, 
               float green, 
               float blue, 
               float* c, 
               float* m, 
               float* y) 
{    
    *c = 1.0F - red;
    *m = 1.0F - green;
    *y = 1.0F - blue;
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

inline
float
_XILI_NL_TO_L_1(float red_nl) 
{    
    float tmp;

    if(red_nl >= 0.081F) {
        tmp = (float) pow((red_nl + 0.099F) / 1.099F, 2.222);
    
    } else if(red_nl <= -0.081F) {
        tmp = (float) -pow((red_nl - 0.099F) / -1.099F, 2.222);
  
    } else {
        tmp = red_nl * 0.222F;
    }

    return tmp;
}

#define _XILI_NL_TO_L(red_nl, green_nl, blue_nl, red, green, blue) \
    if(red_nl >= 0.081F) { \
        *red = pow((red_nl + 0.099F) / 1.099F, 2.222);\
    } else if(red_nl <= -0.081F) { \
        *red = -pow((red_nl - 0.099F) / -1.099F, 2.222);\
    } else { \
        *red = red_nl * 0.222F;\
    } \
    \
    if(green_nl >= 0.081F) { \
        *green = pow((green_nl + 0.099F) / 1.099F, 2.222);\
    } else if(green_nl <= -0.081F) { \
        *green = -pow((green_nl - 0.099F) / -1.099F, 2.222);\
    } else { \
      *green = green_nl * 0.222F;\
    } \
    \
    if(blue_nl >= 0.081F) { \
        *blue = pow((blue_nl + 0.099F) / 1.099F, 2.222);\
    } else if(blue_nl <= -0.081F) { \
        *blue = -pow((blue_nl - 0.099F) / -1.099F, 2.222);\
    } else { \
        *blue = blue_nl * 0.222F;\
    }


inline
Xil_signed16
_XILI_QUANTIZE_Y601_S(float y601) 
{   
    return _XILI_ROUND_S16(-28672.0F + 56064.0F * y601);
}

inline
float
_XILI_NORMALIZE_Y601_S(Xil_signed16 y601) 
{  
    return (((y601) + 28672.0F) / 56064.0F);
}

inline
Xil_signed16
_XILI_QUANTIZE_S(float f) 
{
    return _XILI_ROUND_S16(f * SHORTRANGE + XIL_MINSHORT);
}

inline
float
_XILI_NORMALIZE_S(Xil_signed16 s) 
{
    return (float)((s) - XIL_MINSHORT) / (float)(SHORTRANGE);
}

inline
void
_XILI_QUANTIZE_PHOTO_S(float y601, 
                       float cb601, 
                       float cr601, 
                       Xil_signed16* s1, 
                       Xil_signed16* s2, 
                       Xil_signed16* s3) 
{
    //
    //  65535 / 1.402 = 46743.937
    //
    *s1 = _XILI_ROUND_S16(XIL_MINSHORT + 46743.937F * y601);
    *s2 = _XILI_ROUND_S16(7168.0F + 28518.4F * cb601);
    *s3 = _XILI_ROUND_S16(2304.0F + 34723.84F * cr601);
}

inline
void
_XILI_NORMALIZE_PHOTO_S(Xil_signed16 y601, 
                        Xil_signed16 cb601, 
                        Xil_signed16 cr601, 
                        float*       f1, 
                        float*       f2,
                        float*       f3) 
{
    //
    //  65535 / 1.402 = 46743.937
    //
    *f1 = ((float)(y601) - XIL_MINSHORT) / 46743.937F;
    *f2 = ((float)(cb601) - 7168.0F) / 28518.4F;
    *f3 = ((float)(cr601) - 2304.0F) / 34723.84F;
}

inline
void
_XILI_QUANTIZE_YCC601_S(float         y601, 
                        float         cb601, 
                        float         cr601, 
                        Xil_signed16* s1, 
                        Xil_signed16* s2, 
                        Xil_signed16* s3) 
{
    *s1 = _XILI_ROUND_S16(-28672.0F + 56064.0F * y601);
    *s2 = _XILI_ROUND_S16(32256.0F * cb601);
    *s3 = _XILI_ROUND_S16(40960.0F * cr601);
}

inline
void
_XILI_NORMALIZE_YCC601_S(Xil_signed16 y601, 
                         Xil_signed16 cb601, 
                         Xil_signed16 cr601, 
                         float*       f1, 
                         float*       f2, 
                         float*       f3) 
{  
    *f1 = ((float)(y601) + 28672.0F) / 56064.0F;
    *f2 = (float)(cb601) / 32256.0F;
    *f3 = (float)(cr601) / 40960.0F;
}

inline
Xil_signed16
_XILI_QUANTIZE_Y709_S(float y709) 
{ 
    return  _XILI_ROUND_S16(-28672.0F + 56064.0F * y709);
}

inline
float
_XILI_NORMALIZE_Y709_S(Xil_signed16 y709) 
{   
    return (float) (((y709) + 28672.0F) / 56064.0F);
}

inline
void
_XILI_QUANTIZE_YCC709_S(float         y709, 
                        float         cb709, 
                        float         cr709, 
                        Xil_signed16* s1, 
                        Xil_signed16* s2, 
                        Xil_signed16* s3) 
{    
    *s1 = _XILI_ROUND_S16(-28672.0F + 56064.0F * y709);
    *s2 = _XILI_ROUND_S16(30976.0F * cb709);
    *s3 = _XILI_ROUND_S16(36352.0F * cr709);
}

inline
void
_XILI_NORMALIZE_YCC709_S(Xil_signed16 y709, 
                         Xil_signed16 cb709, 
                         Xil_signed16 cr709, 
                         float*       f1, 
                         float*       f2, 
                         float*       f3) 
{    
    *f1 = ((float)(y709) + 28672.0F) / 56064.0F;
    *f2 = (float)(cb709) / 30976.0F;
    *f3 = (float)(cr709) / 36352.0F;
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

inline float
_XILI_NL_TO_Y709(float red, float green, float blue)
{
    //
    //  RGB Non-linear to Y709
    //
    return 0.2125F * red + 0.7154F * green + 0.0721F * blue;
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

inline float
_XILI_NL_TO_Y601(float red, float green, float blue)
{
    //
    //  RGB Non-linear to y601
    //
    return 0.299F * red + 0.587F * green + 0.114F * blue;
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


