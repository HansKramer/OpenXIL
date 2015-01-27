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

/*This line lets emacs recognize this as -*- C -*- Code
*-----------------------------------------------------------------------
* 
*   File:	XiliMemoryDefines.hh
*   Project:	XIL
*   Revision:	1.8
*   Last Mod:	10:22:16, 03/10/00
*  
*   Description:
*                 
*         
*         
*         
*         
*         
*-----------------------------------------------------------------------
*
*	COPYRIGHT
*
*----------------------------------------------------------------------*/
#pragma ident	"@(#)XiliMemoryDefines.hh	1.8\t00/03/10  "

#ifndef _XIL_MEMORY_DEFINES_H
#define _XIL_MEMORY_DEFINES_H

#include "_XilDefines.h"

/*
 *  XIL_BIT
 */
typedef struct __XilMemoryStorageBit {
   Xil_unsigned8* data;            /* pointer to the first byte of the image */
   unsigned int   scanline_stride; /* the number of bytes between scanlines */
   unsigned int   band_stride;     /* the number of bytes between bands */
   unsigned char  offset;          /* the number of bits to the first pixel */
   Xil_unsigned8* start_data;      /* pointer to the memory to free */
} XilMemoryStorageBit;

/*
 *  XIL_BYTE
 */
typedef struct __XilMemoryStorageByte {
   Xil_unsigned8* data;            /* pointer to the first byte of the image */
   unsigned int   scanline_stride; /* the number of bytes between scanlines */
   unsigned int   pixel_stride;    /* the number of bytes between pixels */
   Xil_unsigned8* start_data;      /* pointer to the memory to free */
} XilMemoryStorageByte;

/*
 *  XIL_SHORT
 */
typedef struct __XilMemoryStorageShort {
   Xil_signed16* data;             /* pointer to the first word of the image */
   unsigned int   scanline_stride; /* the number of shorts between scanlines */
   unsigned int   pixel_stride;    /* the number of shorts between pixels */
   Xil_signed16* start_data;       /* pointer to the memory to free */
} XilMemoryStorageShort;

/*
 *  XIL_FLOAT
 */
typedef struct __XilMemoryStorageFloat32 {
   float* data;                    /* pointer to the first float in the image */
   unsigned int   scanline_stride; /* the number of floats between scanlines */
   unsigned int   pixel_stride;    /* the number of floats between pixels */
   float*  start_data;             /* pointer to the memory to free */
} XilMemoryStorageFloat32;

/*
 *  XIL_UNSIGNED_4
 */
typedef struct __XilMemoryStorageUnsigned4 {
   Xil_unsigned8* data;            /* pointer to the first byte of the image */
   unsigned int   scanline_stride; /* the number of bytes between scanlines */
   unsigned int   pixel_stride;    /* the number of bytes between pixels */
   unsigned int   offset;          /* the number of nibbles to the first pixel */
   Xil_unsigned8* start_data;      /* pointer to the memory to free */
} XilMemoryStorageUnsigned4;

/*
 *  XIL_SIGNED_8
 */
typedef struct __XilMemoryStorageSigned8 {
   Xil_signed8* data;              /* pointer to the first byte of the image */
   unsigned int   scanline_stride; /* the number of bytes between scanlines */
   unsigned int   pixel_stride;    /* the number of bytes between pixels */
   Xil_signed8* start_data  ;      /* pointer to the memory to free */
} XilMemoryStorageSigned8;

/*
 *  XIL_UNSIGNED_16
 */
typedef struct __XilMemoryStorageUnsigned16 {
   Xil_unsigned16* data;           /* pointer to the first word of the image */
   unsigned int   scanline_stride; /* the number of shorts between scanlines */
   unsigned int   pixel_stride;    /* the number of shorts between pixels */
   Xil_unsigned16* start_data;     /* pointer to the memory to free */
} XilMemoryStorageUnsigned16;

/*
 *  XIL_SIGNED_32
 */
typedef struct __XilMemoryStorageSigned32 {
   Xil_signed32* data;             /* pointer to the first word of the image */
   unsigned int   scanline_stride; /* the number of longs between scanlines */
   unsigned int   pixel_stride;    /* the number of longs between pixels */
   Xil_signed32* start_data;       /* pointer to the memory to free */
} XilMemoryStorageSigned32;

/*
 *  XIL_UNSIGNED_32
 */
typedef struct __XilMemoryStorageUnsigned32 {
   Xil_unsigned32* data;           /* pointer to the first word of the image */
   unsigned int   scanline_stride; /* the number of longs between scanlines */
   unsigned int   pixel_stride;    /* the number of longs between pixels */
   Xil_unsigned32* start_data;     /* pointer to the memory to free */
} XilMemoryStorageUnsigned32;

/*
 *  XIL_SIGNED_64
 */
typedef struct __XilMemoryStorageSigned64 {
   Xil_signed64* data;             /* pointer to the first word of the image */
   unsigned long scanline_stride;  /* the number of words between scanlines */
   unsigned short pixel_stride;    /* the number of words between pixels */
   Xil_signed64* start_data;       /* pointer to the memory to free */
} XilMemoryStorageSigned64;

/*
 *  XIL_UNSIGNED_64
 */
typedef struct __XilMemoryStorageUnsigned64 {
   Xil_unsigned64* data;           /* pointer to the first word of the image */
   unsigned long scanline_stride;  /* the number of words between scanlines */
   unsigned short pixel_stride;    /* the number of words between pixels */
   Xil_unsigned64* start_data;     /* pointer to the memory to free */
} XilMemoryStorageUnsigned64;

/*
 *  XIL_FLOAT_64
 */
typedef struct __XilMemoryStorageFloat64 {
   double* data;                   /* pointer to the first double in the image */
   unsigned int   scanline_stride; /* the number of doubles between scanlines */
   unsigned int   pixel_stride;    /* the number of doubles between pixels */
   double* start_data;             /* pointer to the memory to free */
} XilMemoryStorageFloat64;

/*
 *  XIL_FLOAT_128
 */
typedef struct __XilMemoryStorageFloat128 {
   long double* data;              /* pointer to the first long double in the image */
   unsigned int   scanline_stride; /* the number of long doubles between scanlines */
   unsigned int   pixel_stride;    /* the number of long doubles between pixels */
   long double* start_data;        /* pointer to the memory to free */
} XilMemoryStorageFloat128;

/*
 *  XIL_COMPLEX_FLOAT_32
 */
typedef struct __XilMemoryStorageComplexFloat32 {
   XilComplexFloat32* data;          /* pointer to the first float in the image */
   unsigned long scanline_stride;    /* the number of floats between scanlines */
   unsigned short pixel_stride;      /* the number of floats between pixels */
   XilComplexFloat32* start_data;    /* pointer to the memory to free */
} XilMemoryStorageComplexFloat32;

/*
 *  XIL_COMPLEX_FLOAT_64
 */
typedef struct __XilMemoryStorageComplexFloat64 {
   XilComplexFloat64* data;         /* pointer to the first double in the image */
   unsigned long scanline_stride;  /* the number of doubles between scanlines */
   unsigned short pixel_stride;    /* the number of doubles between pixels */
   XilComplexFloat64* start_data;   /* pointer to the memory to free */
} XilMemoryStorageComplexFloat64;


/*
 *  XIL_COMPLEX_MAG_FLOAT32
 */
typedef struct __XilMemoryStorageComplexMagFloat32 {
   XilComplexMagFloat32* data;          /* pointer to the first float in the image */
   unsigned long scanline_stride;  /* the number of floats between scanlines */
   unsigned short pixel_stride;    /* the number of floats between pixels */
   XilComplexMagFloat32* start_data;    /* pointer to the memory to free */
} XilMemoryStorageComplexMagFloat32;

/*
 *  XIL_COMPLEX_MAG_FLOAT_64
 */
typedef struct __XilMemoryStorageComplexMagFloat64 {
   XilComplexMagFloat64* data;         /* pointer to the first double in the image */
   unsigned long scanline_stride;  /* the number of doubles between scanlines */
   unsigned short pixel_stride;    /* the number of doubles between pixels */
   XilComplexMagFloat64* start_data;   /* pointer to the memory to free */
} XilMemoryStorageComplexMagFloat64;

typedef union __XilMemoryStorage {
    XilMemoryStorageBit                bit;
    XilMemoryStorageByte               byte;
    XilMemoryStorageShort              shrt;
    XilMemoryStorageFloat32            flt;
    XilMemoryStorageUnsigned4          nibble;
    XilMemoryStorageSigned8            s_byte;
    XilMemoryStorageUnsigned16         u_shrt;
    XilMemoryStorageSigned32           s_long;
    XilMemoryStorageUnsigned32         u_long;
    XilMemoryStorageSigned64           s_longlong;
    XilMemoryStorageUnsigned64         u_longlong;
    XilMemoryStorageFloat64            dbl;
    XilMemoryStorageFloat128           ldbl;
    XilMemoryStorageComplexFloat32     cflt;
    XilMemoryStorageComplexFloat64     cdbl;
    XilMemoryStorageComplexMagFloat32  cmflt;
    XilMemoryStorageComplexMagFloat64  cmdbl;
} XilMemoryStorage;

#endif /* _XIL_MEMORY_DEFINES_H */
