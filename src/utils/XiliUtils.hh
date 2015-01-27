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
//  File:       XiliUtils.hh
//  Project:    XIL
//  Revision:   1.51
//  Last Mod:   10:23:57, 03/10/00
//
//  Description:
//      Provide common macros/defines for memory image routines.
//      Function prototypes for utility functions
//      Special fast memcpy, fast bit routines.
//
//------------------------------------------------------------------------
//
//      COPYRIGHT
//
//------------------------------------------------------------------------
#pragma ident   "@(#)XiliUtils.hh	1.51\t00/03/10  "

#ifndef _XILI_UTILS_HH
#define _XILI_UTILS_HH

//
//  For size_t
//
#include <stdlib.h>
#ifdef _WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <math.h>

//
//  For XIL datatype definitions
//
#ifdef _XIL_LIBXIL_PRIVATE
#include "_XilDefines.h"
#else
#include <xil/xilGPI.hh>
#endif

// For shl_t (HPUX only)
#if defined(HPUX)
#include <dl.h>
#endif

#ifndef XILI_PATH_MAX
#ifdef _WINDOWS
#define XILI_PATH_MAX    FILENAME_MAX
#else
#define XILI_PATH_MAX    PATH_MAX
#endif
#endif

#if defined(GCC)
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100  + __GNUC_PATCHLEVEL__)
#endif

//
// Macro for byte swapping a 32 bit word.
// Used for endian conversions.
// Notes: 
//
//   1) The argument MUST be a simple variable, not an expression. 
//
//   2) The conversion is done in-place.
//
#define _XILI_BSWAP(x)           \
{                                \
    unsigned int tmp = x;        \
    tmp = (tmp<<16) | (tmp>>16); \
    x = ((tmp>>8) & 0x00ff00ff) | ((tmp<<8) & 0xff00ff00); \
}

//
//  External table to help with byte-to-float conversion.
//
//  Short-to-float is more difficult because the pointer needs to be
//  initialized to point into the middle of the table.  It will require an
//  initialization routine which is already handled in the SHORT compute
//  device.
//
#ifdef _XIL_USE_TABLE_FLT_CNV

extern float  xili_byte_to_float[256];

inline
float
_XILI_B2F(Xil_unsigned8 bval)
{
    return xili_byte_to_float[bval];
}

#else

extern float* xili_byte_to_float;

#define _XILI_B2F(bval)     ((float)(bval))

#endif //_XIL_USE_TABLE_FLT_CNV

//
// Table for calculating the square of a byte value
//
extern const unsigned int _XILI_SQR[256];


//
// Bit copy/set functions
//
typedef unsigned char  bit_base_t;        // our base type for operations 

#define BIT_SHIFT       3           // shift amount 2^^3 for bit_base_t 
#define BIT_NUM_BYTES   1           // bytes per bit_base_t 
#define BIT_NUM_BITS    (BIT_NUM_BYTES * 8)   // bits per bit_base_t 
#define BIT_ALL_ZEROS   ((bit_base_t) 0)
#define BIT_ALL_ONES    ( ~ BIT_ALL_ZEROS)


//
// How big (in bytes) is the actual map?
//
#define BIT_MAP_SIZE(units)  ((units) * BIT_NUM_BYTES)

//
//  set bit table up for bit_base_t's
//
//  Since bit_base_t is just a char, the byte ordering shouldn't matter.
//
static bit_base_t bit_mask[] = {
    0x00000080, 0x00000040, 0x00000020, 0x00000010,
    0x00000008, 0x00000004, 0x00000002, 0x00000001,
#if ( BIT_NUM_BYTES > 1)
    0x00000100, 0x00000200, 0x00000400, 0x00000800,
    0x00001000, 0x00002000, 0x00004000, 0x00008000,
#if (BIT_NUM_BYTES > 2)
    0x00010000, 0x00020000, 0x00040000, 0x00080000,
    0x00100000, 0x00200000, 0x00400000, 0x00800000,
    0x01000000, 0x02000000, 0x04000000, 0x08000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000,
#endif
#endif
#if (BIT_NUM_BYTES > 4)
#include "error; bit_mask[] too small"
#endif
};

//
//  Basic bit operations; don't lock!
//
inline
void
XIL_BMAP_SET(Xil_unsigned8* bits,
             unsigned int   bit_num)
{
    bits[bit_num >> BIT_SHIFT] |= bit_mask[bit_num & (BIT_NUM_BITS-1)];
}

inline
void
XIL_BMAP_CLR(Xil_unsigned8* bits,
             unsigned int   bit_num)
{
    bits[bit_num >> BIT_SHIFT] &= ~bit_mask[bit_num & (BIT_NUM_BITS-1)];
}

inline
Xil_boolean
XIL_BMAP_TST(const Xil_unsigned8* bits,
             unsigned int         bit_num)
{
    if((bits[bit_num >> BIT_SHIFT] &
        bit_mask[bit_num & (BIT_NUM_BITS-1)]) == BIT_ALL_ZEROS) {
        return FALSE;
    } else {
        return TRUE;
    }
}

//
//  Rounding macros.
//
//  Inline function used for rounding floating point values to integral
//  values.
//
//  It should be noted that mathematical rounding (as is done here) will
//  increase the range of values.  Thus, it is important that a rectangle
//  represented as corners should not have all its corners rounded.  For this
//  case, the upper left corner should be rounded and then the second corners
//  should be computed from the clamped difference in the corners
//  (width/height).
//
const float  XILI_RND_NEG     = -0.5F;
const double XILI_RND_NEG_DBL = -0.5;
const float  XILI_RND_POS     =  0.5F;
const double XILI_RND_POS_DBL =  0.5;

inline
int
_XILI_ROUND(float src)
{
    return (src < 0.0F) ?
        ((int)(src + XILI_RND_NEG)) : (int)(src + XILI_RND_POS);
}

inline
int
_XILI_ROUND(double src)
{
    return (src < 0.0) ?
        ((int)(src + XILI_RND_NEG_DBL)) : (int)(src + XILI_RND_POS_DBL);
}

inline
Xil_unsigned8
_XILI_ROUND_1(float src)
{
    if(src < 0.5F) {
	return 0;
    } else {
	return 1;
    }
}

inline
Xil_unsigned8
_XILI_ROUND_U4(float src)
{
    return (src >= 14.5F) ? 15 :
        ((src < 0.5F) ? 0 : ((Xil_unsigned8)(src + XILI_RND_POS)));
}

inline
Xil_unsigned8
_XILI_ROUND_S8(float src)  
{ 				  
    return (src < 0.0F) ?
       ((src <= -127.5F) ? -127 : ((Xil_signed16)(src + XILI_RND_NEG))) :
       ((src >=  126.5F) ?  127 : ((Xil_signed16)(src + XILI_RND_POS)));
}

inline
Xil_unsigned8
_XILI_ROUND_U8(float src)  
{ 				  
    return (src >= 254.5F) ? 255 :
        ((src < 0.5F) ? 0 : ((Xil_unsigned8)(src + XILI_RND_POS)));
}

inline
Xil_signed16
_XILI_ROUND_S16(float src)  
{
    return (src < 0.0F) ?
       ((src <= -32767.5F) ? -32768 : ((Xil_signed16)(src + XILI_RND_NEG))) :
       ((src >=  32766.5F) ?  32767 : ((Xil_signed16)(src + XILI_RND_POS)));
}

inline
Xil_unsigned16
_XILI_ROUND_U16(float src)  
{
    return (src >= 65536.5F) ? 65536 :
        ((src < 0.5F) ? 0 : ((Xil_unsigned16)(src + XILI_RND_POS)));
}

inline
Xil_signed32
_XILI_ROUND_S32(float src)  
{
    return (src >= 0.0F) ?
        (Xil_signed32)(src + XILI_RND_POS) :
        (Xil_signed32)(src + XILI_RND_NEG);
}

inline
Xil_unsigned32
_XILI_ROUND_U32(float src)  
{
    //
    //  SOL64:  Assume "float" is 32-bits so we don't need to clip positively
    //
    return (src < 0.0F) ?  0 : (Xil_unsigned32)(src + XILI_RND_POS);
}

inline
Xil_signed64
_XILI_ROUND_S64(float src)  
{
    return (src >= 0.0F) ?
        (Xil_signed64)(src + XILI_RND_POS) :
        (Xil_signed64)(src + XILI_RND_NEG);
}

inline
Xil_unsigned64
_XILI_ROUND_U64(float src)  
{
    return (src < 0.0F) ?  0 : (Xil_unsigned64)(src + XILI_RND_POS);
}

//
//  Clamping macros.
//
inline
Xil_signed16
_XILI_CLAMP_S16(int src)
{
   return (src > 32767) ? 32767 :
       ((src < -32768) ? -32768 : ((Xil_signed16)(src)));
}

inline
Xil_unsigned8
_XILI_CLAMP_U8(int src)
{
   return (src > 255) ? 255 :
       ((src < 0) ? 0 : ((Xil_unsigned8)(src)));
}

inline
Xil_signed16
_XILI_CLAMP_S16(float src)
{
   return (src > 32767.0F) ? 32767 :
       ((src < -32768.0F) ? -32768 : ((Xil_signed16)(src)));
}

inline
Xil_unsigned8
_XILI_CLAMP_U8(float src)
{
   return (src > 255.0F) ? 255 :
       ((src < 0.0F) ? 0 : ((Xil_unsigned8)(src)));
}

//
//  Commonly used macros
//
#define _XILI_MIN(a,b)   (((a)<(b)) ? (a) : (b))
#define _XILI_MAX(a,b)   (((a)>(b)) ? (a) : (b))
#define _XILI_ABS(x)     (((x)<0) ? (-(x)) : (x))

//
//  Used for testing whether two floating point values can be considered the
//  same value.
//
const double XILI_DBL_EPSILON     = 1.0e-10;
const float  XILI_FLT_EPSILON     = (const float) 1.0e-4;

//
//  Used for deciding whether a (floating point) pixel value is considered
//  to be at an integer pixel.  Note that this value closely coincides with
//  the precision of the pixel extent around the image.
//
const double XILI_PIXEL_ADJACENCY = 1.0e-10;

//
//  Inline functions that compute the nearest PIXEL to the given value.  This
//  is different from mathmatical rounding given our definition of the image
//  coordinate system.
//
//  We define a pixel to represent the area in the coordinate system from -0.5
//  the pixel value and everything up to +0.5 from the pixel value.  This
//  means everything should move closer to pixels toward +Inf.
//
const double XILI_NEAREST_PIXEL_NEG = -0.5 + 2*XILI_PIXEL_ADJACENCY; // -0.499...8
const double XILI_NEAREST_PIXEL_POS =  0.5 - XILI_PIXEL_ADJACENCY;

inline
int
_XILI_NEAREST_PIXEL(double src)
{
    return (src < 0.0) ?
        ((int)(src + XILI_NEAREST_PIXEL_NEG)) : (int)(src + XILI_NEAREST_PIXEL_POS);
}

//
//  Macro used to test if a floating point value is equal to another floating
//  point value within a tolerance.  The tolerance XILI_FLT_EPSILON is
//  defined in XilDefinesPrivate.hh along with other constants.
//
inline
int
XILI_DBL_EQ(double f1,
            double f2)
{
    return fabs(f1 - f2) < XILI_DBL_EPSILON;
}

inline
int
XILI_DBL_EQ_ZERO(double d)
{
    return fabs(d) < XILI_DBL_EPSILON;
}

inline
int
XILI_FLT_EQ(float f1,
            float f2,
            float epsilon = XILI_FLT_EPSILON)
{
    return fabs(f1 - f2) < epsilon;
}

//
//  Inline function used to test whether a single precision floating point value
//  is within the default tolerance of the nearest integer.
//
inline
Xil_boolean
_XILI_FLT_EQ_INT(double f)
{
    return (Xil_boolean)XILI_FLT_EQ((float)_XILI_ROUND(f), (float)f);
}

//
//  Inline function to test if two pixels are equal to each other within the
//  XILI_PIXEL_ADJACENCY tolerance.
//
inline
int
XILI_PIXEL_EQ(double pixel1,
              double pixel2)
{
    return fabs(pixel1 - pixel2) < XILI_PIXEL_ADJACENCY;
}

inline
int
XILI_PIXEL_EQ(double pixel1,
              double pixel2,
              double epsilon)
{
    return fabs(pixel1 - pixel2) < epsilon;
}

//
// Inline function used to test whether a single precision floating point value
// is within the default tolerance of the nearest integer.
//
inline
Xil_boolean
_XILI_PIXEL_EQ_INT(double f)
{
    return (Xil_boolean)XILI_PIXEL_EQ(_XILI_ROUND(f), f);
}

inline
Xil_boolean
_XILI_PIXEL_EQ_INT(double f,
                   double epsilon)
{
    return (Xil_boolean)XILI_PIXEL_EQ(_XILI_ROUND(f), f, epsilon);
}

//
//  Test to see if x2 and y2 caused an empty rect to occur but only if they're
//  beyond our XILI_PIXEL_ADJACENCY.
//
//  Returns TRUE if the rect is not empty and FALSE if the rect is empty.
//
inline
Xil_boolean
XILI_CHECK_RECT_EMPTY(double* x1,
                      double* y1,
                      double  x2,
                      double  y2)
{
    Xil_boolean ret_val = TRUE;

    //
    //  If it's clipped to an empty rect, then return FALSE.
    //
    if(x2 < *x1) {
        //
        //  Now verify that it's within our pixel adjacency.
        //
        if(x2 < (*x1 - XILI_PIXEL_ADJACENCY)) {
            ret_val = FALSE;
        } else {
            *x1 = x2;
        }
    }

    if(y2 < *y1) {
        if(y2 < (*y1 - XILI_PIXEL_ADJACENCY)) {
            ret_val = FALSE;
        } else {
            *y1 = y2;
        }
    }

    return ret_val;
}

//
//  XIL DataType Utilities
//
inline
size_t
xili_sizeof(XilDataType data_type)
{
    switch(data_type) {
      case XIL_BIT:                    return sizeof(Xil_unsigned8);
      case XIL_BYTE:                   return sizeof(Xil_unsigned8);
      case XIL_SHORT:                  return sizeof(Xil_signed16);
      case XIL_FLOAT:                  return sizeof(Xil_float32);
      case XIL_UNSIGNED_4:             return sizeof(Xil_unsigned8);
      case XIL_SIGNED_8:               return sizeof(Xil_signed8);
      case XIL_UNSIGNED_16:            return sizeof(Xil_unsigned16);
      case XIL_SIGNED_32:              return sizeof(Xil_signed32);
      case XIL_UNSIGNED_32:            return sizeof(Xil_unsigned32);
      case XIL_SIGNED_64:              return sizeof(Xil_signed64);
      case XIL_UNSIGNED_64:            return sizeof(Xil_unsigned64);
      case XIL_FLOAT_64:               return sizeof(Xil_float64);
      case XIL_FLOAT_128:              return sizeof(Xil_float128);
      case XIL_COMPLEX_FLOAT_32:       return sizeof(XilComplexFloat32);
      case XIL_COMPLEX_FLOAT_64:       return sizeof(XilComplexFloat64);
      case XIL_COMPLEX_MAG_FLOAT_32:   return sizeof(XilComplexMagFloat32);
      case XIL_COMPLEX_MAG_FLOAT_64:   return sizeof(XilComplexMagFloat64);
    }

    return 0;  // Since the argument is an enumerated type
               // the compiler should guarantee that invalid
               // values never get passed. Is this sufficient ?
}

inline
Xil_boolean
xili_is_supported_datatype(XilDataType data_type)
{
    switch(data_type) {
      case XIL_BIT:
      case XIL_BYTE:
      case XIL_SHORT:
      case XIL_FLOAT:
      case XIL_UNSIGNED_4:
      case XIL_SIGNED_8:
      case XIL_UNSIGNED_16:
      case XIL_SIGNED_32:
      case XIL_UNSIGNED_32:
      case XIL_SIGNED_64:
      case XIL_UNSIGNED_64:
      case XIL_FLOAT_64:
      case XIL_FLOAT_128:
      case XIL_COMPLEX_FLOAT_32:
      case XIL_COMPLEX_FLOAT_64:
      case XIL_COMPLEX_MAG_FLOAT_32:
      case XIL_COMPLEX_MAG_FLOAT_64:
        return TRUE;
    }

    return FALSE;
}

inline
char*
xili_datatype_to_string(XilDataType data_type)
{
    switch(data_type) {
      case XIL_BIT:                   return "1";
      case XIL_BYTE:                  return "8";
      case XIL_SHORT:                 return "16";
      case XIL_FLOAT:                 return "f32";
      case XIL_UNSIGNED_4:            return "u4";
      case XIL_SIGNED_8:              return "s8";
      case XIL_UNSIGNED_16:           return "u16";
      case XIL_SIGNED_32:             return "s32";
      case XIL_UNSIGNED_32:           return "u32";
      case XIL_SIGNED_64:             return "s64";
      case XIL_UNSIGNED_64:           return "u64";
      case XIL_FLOAT_64:              return "f64";
      case XIL_FLOAT_128:             return "f128";
      case XIL_COMPLEX_FLOAT_32:      return "cf32";
      case XIL_COMPLEX_FLOAT_64:      return "cf64";
      case XIL_COMPLEX_MAG_FLOAT_32:  return "cmf32";
      case XIL_COMPLEX_MAG_FLOAT_64:  return "cmf64";
    }
    
    return NULL;
}

//
// Function and defines to get sysconf info.

#if defined(_WINDOWS) || defined(LINUX)
// NOTE : Whenever a new _SC_?? constant is defined here make sure
//        to add code in xili_sysconf().
// taken from unistd.h
#ifndef _SC_PAGESIZE
#define _SC_PAGESIZE          11
#endif
#ifndef _SC_NPROCESSORS_ONLN
#define _SC_NPROCESSORS_ONLN  15
#endif
#ifndef _SC_PHYS_PAGES
#define _SC_PHYS_PAGES        500
#endif
#elif defined(IRIX)
#ifndef _SC_NPROCESSORS_ONLN
#define _SC_NPROCESSORS_ONLN  15
#endif
#endif

long xili_sysconf(int);

//
//  Obtain the system memory page size for use in memory allocation, primarily
//  for images.
//
//  TODO: This will be a platform-dependent call
//        which will need to be replaced with the
//        appropriate function on other platforms.
//
inline
unsigned int
xili_get_pagesize()
{
  return ((unsigned int ) xili_sysconf(_SC_PAGESIZE));
}

//
//  Utility functions to query the datatype
//
inline
Xil_boolean
xili_is_integer_datatype(XilDataType data_type)
{
    switch(data_type) {
      case XIL_BIT:                   return TRUE;
      case XIL_BYTE:                  return TRUE;
      case XIL_SHORT:                 return TRUE;
      case XIL_FLOAT:                 return FALSE;
      case XIL_UNSIGNED_4:            return TRUE;
      case XIL_SIGNED_8:              return TRUE;
      case XIL_UNSIGNED_16:           return TRUE;
      case XIL_SIGNED_32:             return TRUE;
      case XIL_UNSIGNED_32:           return TRUE;
      case XIL_SIGNED_64:             return TRUE;
      case XIL_UNSIGNED_64:           return TRUE;
      case XIL_FLOAT_64:              return FALSE;
      case XIL_FLOAT_128:             return FALSE;
      case XIL_COMPLEX_FLOAT_32:      return FALSE;
      case XIL_COMPLEX_FLOAT_64:      return FALSE;
      case XIL_COMPLEX_MAG_FLOAT_32:  return FALSE;
      case XIL_COMPLEX_MAG_FLOAT_64:  return FALSE;
    }
    return FALSE;
}

inline
Xil_boolean
xili_is_signed_integer_datatype(XilDataType data_type)
{
    switch(data_type) {
      case XIL_BIT:                   return FALSE;
      case XIL_BYTE:                  return FALSE;
      case XIL_SHORT:                 return TRUE;
      case XIL_FLOAT:                 return FALSE;
      case XIL_UNSIGNED_4:            return FALSE;
      case XIL_SIGNED_8:              return TRUE;
      case XIL_UNSIGNED_16:           return FALSE;
      case XIL_SIGNED_32:             return TRUE;
      case XIL_UNSIGNED_32:           return FALSE;
      case XIL_SIGNED_64:             return TRUE;
      case XIL_UNSIGNED_64:           return FALSE;
      case XIL_FLOAT_64:              return FALSE;
      case XIL_FLOAT_128:             return FALSE;
      case XIL_COMPLEX_FLOAT_32:      return FALSE;
      case XIL_COMPLEX_FLOAT_64:      return FALSE;
      case XIL_COMPLEX_MAG_FLOAT_32:  return FALSE;
      case XIL_COMPLEX_MAG_FLOAT_64:  return FALSE;
    }
    return FALSE;
}

inline
Xil_boolean
xili_is_floating_datatype(XilDataType data_type)
{
    switch(data_type) {
      case XIL_BIT:                   return FALSE;
      case XIL_BYTE:                  return FALSE;
      case XIL_SHORT:                 return FALSE;
      case XIL_FLOAT:                 return TRUE;
      case XIL_UNSIGNED_4:            return FALSE;
      case XIL_SIGNED_8:              return FALSE;
      case XIL_UNSIGNED_16:           return FALSE;
      case XIL_SIGNED_32:             return FALSE;
      case XIL_UNSIGNED_32:           return FALSE;
      case XIL_SIGNED_64:             return FALSE;
      case XIL_UNSIGNED_64:           return FALSE;
      case XIL_FLOAT_64:              return TRUE;
      case XIL_FLOAT_128:             return TRUE;
      case XIL_COMPLEX_FLOAT_32:      return FALSE;
      case XIL_COMPLEX_FLOAT_64:      return FALSE;
      case XIL_COMPLEX_MAG_FLOAT_32:  return FALSE;
      case XIL_COMPLEX_MAG_FLOAT_64:  return FALSE;
    }
    return FALSE;
}

inline
Xil_boolean
xili_is_aggregate_datatype(XilDataType data_type)
{
    switch(data_type) {
      case XIL_BIT:                   return FALSE;
      case XIL_BYTE:                  return FALSE;
      case XIL_SHORT:                 return FALSE;
      case XIL_FLOAT:                 return FALSE;
      case XIL_UNSIGNED_4:            return FALSE;
      case XIL_SIGNED_8:              return FALSE;
      case XIL_UNSIGNED_16:           return FALSE;
      case XIL_SIGNED_32:             return FALSE;
      case XIL_UNSIGNED_32:           return FALSE;
      case XIL_SIGNED_64:             return FALSE;
      case XIL_UNSIGNED_64:           return FALSE;
      case XIL_FLOAT_64:              return FALSE;
      case XIL_FLOAT_128:             return FALSE;
      case XIL_COMPLEX_FLOAT_32:      return TRUE;
      case XIL_COMPLEX_FLOAT_64:      return TRUE;
      case XIL_COMPLEX_MAG_FLOAT_32:  return TRUE;
      case XIL_COMPLEX_MAG_FLOAT_64:  return TRUE;
    }
    return FALSE;
}


//
//  Return minimum value which can be held in the datatype.
//
//  TODO: These routines can't handle 32 bit or larger types yet.
//        This is due to the problem of what type to use
//        to return the value. Unsigned int is needed to hold the
//        max but signed int is neede to hold the min for signed types.
//
//        The error return is a kludge. Should probably be a define
//        like XIL_UNSUPPORTED_TYPE or something.
//
inline
int
xili_get_datatype_min(XilDataType data_type)
{
    switch(data_type) {
        case XIL_BIT:                   return 0;
        case XIL_BYTE:                  return 0;
        case XIL_SHORT:                 return -32768;
        case XIL_UNSIGNED_4:            return 0;
        case XIL_SIGNED_8:              return -128;
        case XIL_UNSIGNED_16:           return 0;
        default:                        return 42; //error
    }
}

//
//  Return maximum value which can be held in the datatype.
//
inline
int
xili_get_datatype_max(XilDataType data_type)
{
    switch(data_type) {
        case XIL_BIT:                   return 1;
        case XIL_BYTE:                  return 255;
        case XIL_SHORT:                 return 32767;
        case XIL_UNSIGNED_4:            return 15;
        case XIL_SIGNED_8:              return 127;
        case XIL_UNSIGNED_16:           return 65535;
        default:                        return 42; //error
    }
}

void
xili_cspace_name_to_opcode(const char*          name,
                           XilColorspaceOpCode* op_code,
                           unsigned int*        num_bands);

inline
size_t
xili_maxlen_of_datatype_string()
{
    return 6;
}

//
// Function prototypes
//

//  For fast bit copys...
void
xili_bit_memcpy(Xil_unsigned8* src_scanline, 
                Xil_unsigned8* dst_scanline,
                unsigned int   xsize,
                unsigned int   src_offset,
                unsigned int   dst_offset);

//  For fast bit sets...
void
xili_bit_setvalue(Xil_unsigned8* dst_scanline, 
                  unsigned int   value,
                  unsigned int   xsize,
                  unsigned int   dst_offset);

//  For fast bit nots...
void
xili_bit_not(Xil_unsigned8* src_scanline, 
             Xil_unsigned8* dst_scanline,
             unsigned int   xsize,
             unsigned int   src_offset,
             unsigned int   dst_offset);

//  For fast checks for zeros...
Xil_boolean
xili_bit_check_for_zero(const Xil_unsigned8* dst_scanline, 
                        unsigned int         xsize,
                        unsigned int         dst_offset);

//  For checking a bit XilImage for zeros...
#if 0
Xil_boolean
xili_bit_check_for_zero(XilImage* dst);
#endif

//
//  We used to have our own version of memcpy that ran
//  more efficiently than the system version. With the
//  newer hardware, this is no longer true - so use the
//  system memcpy always.
//

#define _XIL_USE_SYSTEM_MEMCPY 1
#ifdef _XIL_USE_SYSTEM_MEMCPY

#include <string.h>

#define xili_memcpy(s1, s2, n) (memcpy((s1), (s2), (n)))
#define xili_memset(s1, s2, n) (memset((s1), (s2), (n)))

#else
//
//  These are no longer in use
//
extern "C" { 
    extern void* xili_memcpy(void* s1, const void* s2, size_t n);
}

extern "C" { 
    extern void* xili_memset(void* s1, int c, size_t n);
}
#endif

//
//  Hash function for strings.
//
unsigned int
xili_hash_string(const char* the_string);


//
// Function to print system errno string

void        xili_print_debug(char* format, ...);
const char* xili_strerror(int Errno);
#ifdef _WINDOWS
char*       xili_get_install_path();
#endif

//
// xili_dl?? funcs prototypes and typedefs

#if defined(_WINDOWS)
typedef	HINSTANCE	XilDlHandle;
#elif defined(HPUX)
typedef	shl_t		XilDlHandle;
#else
typedef	void*		XilDlHandle;
#endif


XilDlHandle  xili_dlopen(char *lib_name);
void*        xili_dlsym(XilDlHandle dlhandle, const char *symbol_name);
int          xili_dlclose(XilDlHandle dlhandle);
const char*  xili_dlerror();

char *xili_dgettext(const char *, const char *);

//
// XilGlobalState.cc : tables for symbol names
//

//
//  COMPUTE entry symbol
//

#if defined(_WINDOWS)
#define _XILI_COMPUTE_ENTRY_SYMBOL \
                            "?create@XilDeviceManagerCompute@@SAPAV1@IIPAI0@Z"
#elif defined(HPUX)
#define _XILI_COMPUTE_ENTRY_SYMBOL \
                            "create__23XilDeviceManagerComputeSFUiT1PUiT3"
#elif defined(GCC)

#if GCC_VERSION == 29600 
#define _XILI_COMPUTE_ENTRY_SYMBOL "create__23XilDeviceManagerComputeUiUiPUiT3"
#elif GCC_VERSION >= 30000
#define _XILI_COMPUTE_ENTRY_SYMBOL "_ZN23XilDeviceManagerCompute6createEjjPjS0_"
#endif

#elif defined(IRIX)
#define _XILI_COMPUTE_ENTRY_SYMBOL \
                            "create__23XilDeviceManagerComputeSFUiT1PUiT3"
#else
#define _XILI_COMPUTE_ENTRY_SYMBOL \
                            "__0fXXilDeviceManagerComputeGcreateUiTBPUiTDT"
#endif


//
// IO entry symbol
//

#if defined(_WINDOWS)
#define _XILI_IO_ENTRY_SYMBOL \
                                  "?create@XilDeviceManagerIO@@SAPAV1@IIPAI0@Z"
#elif defined(HPUX)
#define _XILI_IO_ENTRY_SYMBOL \
                                  "create__18XilDeviceManagerIOSFUiT1PUiT3"
#elif defined(GCC)

#if GCC_VERSION == 29600 
#define _XILI_IO_ENTRY_SYMBOL     "create__18XilDeviceManagerIOUiUiPUiT3"
#elif GCC_VERSION >= 30000
#define _XILI_IO_ENTRY_SYMBOL     "_ZN18XilDeviceManagerIO6createEjjPjS0_"
#endif

#elif defined(IRIX)
#define _XILI_IO_ENTRY_SYMBOL \
                                  "create__18XilDeviceManagerIOSFUiT1PUiT3"
#else
#define _XILI_IO_ENTRY_SYMBOL \
                                  "__0fSXilDeviceManagerIOGcreateUiTBPUiTDT"
#endif

//
// COMPRESSION entry symbol
//

#if defined(_WINDOWS)
#define _XILI_COMPRESSION_ENTRY_SYMBOL \
                         "?create@XilDeviceManagerCompression@@SAPAV1@IIPAI0@Z"
#elif defined(HPUX)
#define _XILI_COMPRESSION_ENTRY_SYMBOL \
                         "create__27XilDeviceManagerCompressionSFUiT1PUiT3"
#elif defined(GCC)

#if GCC_VERSION == 29600
#define _XILI_COMPRESSION_ENTRY_SYMBOL  "create__27XilDeviceManagerCompressionUiUiPUiT3"
#elif GCC_VERSION >= 30000
#define _XILI_COMPRESSION_ENTRY_SYMBOL  "_ZN27XilDeviceManagerCompression6createEjjPjS0_"
#endif

#elif defined(IRIX)
#define _XILI_COMPRESSION_ENTRY_SYMBOL \
                         "create__27XilDeviceManagerCompressionSFUiT1PUiT3"
#else
#define _XILI_COMPRESSION_ENTRY_SYMBOL \
                         "__0fbXilDeviceManagerCompressionGcreateUiTBPUiTDT"
#endif

//
// STORAGE entry symbol
//

#if defined(_WINDOWS)
#define _XILI_STORAGE_ENTRY_SYMBOL \
                             "?create@XilDeviceManagerStorage@@SAPAV1@IIPAI0@Z"
#elif defined(HPUX)
#define _XILI_STORAGE_ENTRY_SYMBOL \
                             "create__23XilDeviceManagerStorageSFUiT1PUiT3"
#elif defined(GCC)

#if GCC_VERSION == 29600
#define _XILI_STORAGE_ENTRY_SYMBOL "create__23XilDeviceManagerStorageUiUiPUiT3"
#elif GCC_VERSION >= 30000
#define _XILI_STORAGE_ENTRY_SYMBOL "_ZN23XilDeviceManagerStorage6createEjjPjS0_"
#endif

#elif defined(IRIX)
#define _XILI_STORAGE_ENTRY_SYMBOL \
                             "create__23XilDeviceManagerStorageSFUiT1PUiT3"
#else
#define _XILI_STORAGE_ENTRY_SYMBOL \
                             "__0fXXilDeviceManagerStorageGcreateUiTBPUiTDT"
#endif


//
// OP entry symbol
//

#if defined(_WINDOWS)
#define _XILI_OP_ENTRY_SYMBOL \
                                  "?XilOpEntry@@YA?AW4__XilStatus@@XZ"
#elif defined(HPUX)
#define _XILI_OP_ENTRY_SYMBOL \
                                  "XilOpEntry__Fv"
#elif defined(GCC)

#if GCC_VERSION == 29600
#define _XILI_OP_ENTRY_SYMBOL  "XilOpEntry__Fv"
#elif GCC_VERSION >= 30000
#define _XILI_OP_ENTRY_SYMBOL  "_Z10XilOpEntryv"
#endif

#elif defined(IRIX)
#define _XILI_OP_ENTRY_SYMBOL \
                                  "XilOpEntry__Fv"
#else
#define _XILI_OP_ENTRY_SYMBOL \
                                  "__0FKXilOpEntryv"
#endif

//
// OP_FUNC entry symbol
//

#if defined(_WINDOWS)
#define _XILI_OP_FUNC_ENTRY_SYMBOL \
                                 "?create@%s@@SAPAVXilOp@@PADQAPAXH@Z"
#elif defined(HPUX) || defined(IRIX)
#define _XILI_OP_FUNC_ENTRY_SYMBOL \
                                 "create__%d%sSFPcPPvi"
#elif defined(GCC)

#if GCC_VERSION == 29600
#define _XILI_OP_FUNC_ENTRY_SYMBOL "create__%d%sPcPPvi"
#elif GCC_VERSION >= 30000
#define _XILI_OP_FUNC_ENTRY_SYMBOL "_ZN%d%s6createEPcPPvi"
#endif

#else
#define _XILI_OP_FUNC_ENTRY_SYMBOL \
                                 "__0f%c%sGcreatePcPPviT"
#define _XILI_OP_FUNC_ENTRY_LOOP_SYMBOL \
                                 "__0f%d%c%sGcreatePcPPviT"
#endif


//
// mode defines for access()
//

#ifdef _WINDOWS
#define F_OK       00
#endif


#endif // _XILI_UTILS_HH
