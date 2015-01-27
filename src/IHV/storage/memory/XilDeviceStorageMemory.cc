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
//  File:	XilDeviceStorageMemory.cc
//  Project:	XIL
//  Revision:	1.70
//  Last Mod:	10:16:28, 03/10/00
//
//  Description:
//
//
//
//
//
//
//
//
//  MT-level:  SAFE
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilDeviceStorageMemory.cc	1.70\t00/03/10  "

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <malloc.h>
#include <fcntl.h>
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#else
#include <sys/mman.h>
#endif /* _WINDOWS */
#include <xil/xilGPI.hh>
#include "XiliUtils.hh"
#include "XilDeviceStorageMemory.hh"

#define roundup(val, gran)       ((val)+(gran)-1 & ~((gran)-1))

#ifndef _WINDOWS
int            XilDeviceStorageMemory::devZeroFd         = -1;
int            XilDeviceStorageMemory::refCount          =  0;
#endif /* _WINDOWS */
unsigned int   XilDeviceStorageMemory::pageSize          =  0;
XilMutex       XilDeviceStorageMemory::staticMutex;

XilDeviceStorageMemory::XilDeviceStorageMemory(XilImage* parent_image) :
    XilDeviceStorage(parent_image)
{
    //
    //  Since it's static, many images may be created at the same time.
    //
    staticMutex.lock();
#ifndef _WINDOWS
    if(devZeroFd == -1) {
        //
        //  If it fails, then we don't use /dev/zero, we just use valloc().
        //
        devZeroFd = open("/dev/zero", O_RDWR);
    }

    refCount++;
#endif /* _WINDOWS */

    if(pageSize == 0) {
        pageSize = xili_get_pagesize();
    }

    dataType = parent_image->getDataType();

    staticMutex.unlock();
}

XilDeviceStorageMemory::~XilDeviceStorageMemory()
{
#ifndef _WINDOWS
    staticMutex.lock();
    if(--refCount == 0) {
        close(devZeroFd);
        devZeroFd = -1;
    }
    staticMutex.unlock();
#endif /* _WINDOWS */
}

//
//  setPixel() and getPixel() assume that the storage that has been passed in
//  such that dataPtr is pointing to the correct location within the image and
//  we only need to use the offsets to get to the pixel we want to set.
//
XilStatus
XilDeviceStorageMemory::setPixel(XilStorage*  storage,
				 float*       values,
                                 unsigned int xoffset,
				 unsigned int yoffset,
                                 unsigned int boffset,
				 unsigned int nbands)
{
    if(! storage->isType(XIL_PIXEL_SEQUENTIAL)) {
        unsigned int i = 0;
        unsigned int b;
        unsigned int end_band = nbands+boffset;
        for(b=boffset; b<end_band; b++) {
            //
            //  bit, byte, short, float  are the ones that are fully
            //  implemented for XIL 1.3 the rest are untested and may
            //  be ifdef'd out for a later release.
            //
            switch(dataType) {
              case XIL_BIT:
              {
                  Xil_unsigned8* data =
                      ((Xil_unsigned8*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b);
                  unsigned int   offset  = storage->getOffset(b);

                  //
                  //  Set up the bit we want to set the pixel to
                  //
                  if(values[i++] < 0.5) {
                      XIL_BMAP_CLR(data, offset + xoffset);
                  } else {
                      XIL_BMAP_SET(data, offset + xoffset);
                  }
              }
              break;

              case XIL_BYTE:
              {
                  Xil_unsigned8* data =
                      ((Xil_unsigned8*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);
                  
                  *data = _XILI_ROUND_U8(values[i++]);
              }
              break;

              case XIL_SHORT:
              {
                  Xil_signed16* data =
                      ((Xil_signed16*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  *data = _XILI_ROUND_S16(values[i++]);
              }
              break;

              case XIL_FLOAT: {
                  Xil_float32* data =
                      ((Xil_float32*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  *data = values[i++];
              }
              break;

              //
              //  Untested datatypes begin here
              //
              case XIL_UNSIGNED_4:
              {
                  Xil_unsigned8* data =
                      ((Xil_unsigned8*)storage->getDataPtr(b)) +
                      yoffset      * storage->getScanlineStride(b)  +
                      (xoffset>>1) * storage->getScanlineStride(b);
                  unsigned int   offset = storage->getOffset(b);

                  //
                  //  Set up the bits we want to set the pixel to.
                  //
                  *data |= _XILI_ROUND_U4(values[i++])>>offset;
              }
              break;

              case XIL_SIGNED_8:
              {
                  Xil_signed8* data =
                      ((Xil_signed8*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  *data = _XILI_ROUND_S8(values[i++]);
              }
              break;

              case XIL_UNSIGNED_16:
              {
                  Xil_unsigned16* data =
                      ((Xil_unsigned16*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  *data = _XILI_ROUND_U16(values[i++]);
              }
              break;

              case XIL_SIGNED_32:
              {
                  Xil_signed32* data =
                      ((Xil_signed32*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  *data = _XILI_ROUND_S32(values[i++]);
              }
              break;

              case XIL_UNSIGNED_32:
              {
                  Xil_unsigned32* data =
                      ((Xil_unsigned32*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  *data = _XILI_ROUND_U32(values[i++]);
              }
              break;

              case XIL_SIGNED_64:
              {
                  Xil_signed64* data =
                      ((Xil_signed64*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  *data = _XILI_ROUND_S64(values[i++]);
              }
              break;

              case XIL_UNSIGNED_64:
              {
                  Xil_unsigned64* data =
                      ((Xil_unsigned64*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  *data = _XILI_ROUND_U64(values[i++]);
              }
              break;

              case XIL_FLOAT_64:
              {
                  Xil_float64* data =
                      ((Xil_float64*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  *data = values[i++];
              }
              break;

              case XIL_FLOAT_128:
              {
                  Xil_float128* data =
                      ((Xil_float128*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  *data = values[i++];
              }
              break;

              case XIL_COMPLEX_FLOAT_32:
              {
                  XilComplexFloat32* data =
                      ((XilComplexFloat32*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  //
                  //  Ordering is real followed by imaginary
                  //
                  data->real = values[i++];
                  data->img  = values[i++];
              }
              break;

              case XIL_COMPLEX_FLOAT_64: 
              {
                  XilComplexFloat64* data =
                      ((XilComplexFloat64*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  //
                  //  Ordering is real followed by imaginary
                  //
                  data->real = values[i++];
                  data->img  = values[i++];
              }
              break;

              case XIL_COMPLEX_MAG_FLOAT_32:
              {
                  XilComplexMagFloat32* data =
                      ((XilComplexMagFloat32*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  //
                  //  Ordering is mag followed by phase
                  //
                  data->mag   = values[i++];
                  data->phase = values[i++];
              }
              break;

              case XIL_COMPLEX_MAG_FLOAT_64:
              {
                  XilComplexMagFloat64* data =
                      ((XilComplexMagFloat64*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  //
                  //  Ordering is mag followed by phase
                  //
                  data->mag   = values[i++];
                  data->phase = values[i++];
              }
              break;
            }
        }
    } else { ///////////// XIL_PIXEL_SEQUENTIAL ////////////////////
        unsigned int i = 0;
        unsigned int b;
        unsigned int end_band = nbands+boffset;

	//
	//  byte, short, float  are the ones that are fully
	//  implemented for XIL 1.3 the rest are untested and may
	//  be ifdef'd out for a later release.
	//
	switch(dataType) {
	  case XIL_BYTE:
          {
              Xil_unsigned8* data;
              unsigned int   scanline_stride;
              unsigned int   pixel_stride;
              storage->getStorageInfo(&pixel_stride,
                                      &scanline_stride,
                                      NULL, NULL,
                                      (void**)&data);

              data += yoffset * scanline_stride + xoffset * pixel_stride;

              for(b=boffset; b<end_band; b++) {
                  *(data+b) = _XILI_ROUND_U8(values[i++]);
              }
	  }
          break;

	  case XIL_SHORT:
          {
              Xil_signed16* data;
              unsigned int  scanline_stride;
              unsigned int  pixel_stride;
              storage->getStorageInfo(&pixel_stride,
                                      &scanline_stride,
                                      NULL, NULL,
                                      (void**)&data);

              data += yoffset * scanline_stride + xoffset * pixel_stride;

              for(b=boffset; b<end_band; b++) {
                  *(data+b) = _XILI_ROUND_S16(values[i++]);
              }
	  }
          break;

	  case XIL_FLOAT: {
              Xil_float32* data;
              unsigned int scanline_stride;
              unsigned int pixel_stride;
              storage->getStorageInfo(&pixel_stride,
                                      &scanline_stride,
                                      NULL, NULL,
                                      (void**)&data);

              data += yoffset * scanline_stride + xoffset * pixel_stride;

              for(b=boffset; b<end_band; b++) {
                  *(data+b) = values[i++];
              }
	  }
          break;

	  //
	  //  Untested datatypes begin here
	  //
	  case XIL_UNSIGNED_4:
          {
              //
              //  Making this loop implementation easier by writing GENERAL
              //  loop...
              //
              for(b=boffset; b<end_band; b++) {
                  Xil_unsigned8* data =
                      ((Xil_unsigned8*)storage->getDataPtr(b)) +
                      yoffset      * storage->getScanlineStride(b)  +
                      (xoffset>>1) * storage->getScanlineStride(b);
                  unsigned int   offset = storage->getOffset(b);

                  //
                  //  Set up the bits we want to set the pixel to.
                  //
                  *data |= _XILI_ROUND_U4(values[i++])>>offset;
              }
	  }
          break;

	  case XIL_SIGNED_8:
          {
              Xil_signed8* data =
                  ((Xil_signed8*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              for(b=boffset; b<end_band; b++) {
                  *(data+b) = _XILI_ROUND_S8(values[i++]);
              }
	  }
          break;

	  case XIL_UNSIGNED_16:
          {
              Xil_unsigned16* data =
                  ((Xil_unsigned16*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              for(b=boffset; b<end_band; b++) {
                  *(data+b) = _XILI_ROUND_U16(values[i++]);
              }
	  }
          break;

	  case XIL_SIGNED_32:
          {
              Xil_signed32* data =
                  ((Xil_signed32*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              for(b=boffset; b<end_band; b++) {
                  *(data+b) = _XILI_ROUND_S32(values[i++]);
              }
	  }
          break;

	  case XIL_UNSIGNED_32:
          {
              Xil_unsigned32* data =
                  ((Xil_unsigned32*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              for(b=boffset; b<end_band; b++) {
                  *(data+b) = _XILI_ROUND_U32(values[i++]);
              }
	  }
          break;

	  case XIL_SIGNED_64:
          {
              Xil_signed64* data =
                  ((Xil_signed64*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              for(b=boffset; b<end_band; b++) {
                  *(data+b) = _XILI_ROUND_S64(values[i++]);
              }
	  }
          break;

	  case XIL_UNSIGNED_64:
          {
              Xil_unsigned64* data =
                  ((Xil_unsigned64*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              for(b=boffset; b<end_band; b++) {
                  *(data+b) = _XILI_ROUND_U64(values[i++]);
              }
	  }
          break;

	  case XIL_FLOAT_64:
          {
              Xil_float64* data =
                  ((Xil_float64*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              for(b=boffset; b<end_band; b++) {
                  *(data+b) = values[i++];
              }
	  }
          break;

	  case XIL_FLOAT_128:
          {
              Xil_float128* data =
                  ((Xil_float128*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              for(b=boffset; b<end_band; b++) {
                  *(data+b) = values[i++];
              }
	  }
          break;

	  case XIL_COMPLEX_FLOAT_32:
          {
              XilComplexFloat32* data =
                  ((XilComplexFloat32*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              //
              //  Ordering is real followed by imaginary
              //
              for(b=boffset; b<end_band; b++) {
                  (data+b)->real = values[i++];
                  (data+b)->img  = values[i++];
              }
	  }
          break;

	  case XIL_COMPLEX_FLOAT_64: 
          {
              XilComplexFloat64* data =
                  ((XilComplexFloat64*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              //
              //  Ordering is real followed by imaginary
              //
              for(b=boffset; b<end_band; b++) {
                  (data+b)->real = values[i++];
                  (data+b)->img  = values[i++];
              }
	  }
          break;

	  case XIL_COMPLEX_MAG_FLOAT_32:
          {
              XilComplexMagFloat32* data =
                  ((XilComplexMagFloat32*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              //
              //  Ordering is mag followed by phase
              //
              for(b=boffset; b<end_band; b++) {
                  (data+b)->mag   = values[i++];
                  (data+b)->phase = values[i++];
              }
	  }
          break;

	  case XIL_COMPLEX_MAG_FLOAT_64:
          {
              XilComplexMagFloat64* data =
                  ((XilComplexMagFloat64*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              //
              //  Ordering is mag followed by phase
              //
              for(b=boffset; b<end_band; b++) {
                  (data+b)->mag   = values[i++];
                  (data+b)->phase = values[i++];
              }
	  }
          break;
	}
    }

    return XIL_SUCCESS;
}


//
//  getPixel
//
XilStatus
XilDeviceStorageMemory::getPixel(XilStorage*  storage,
				 float*       values,
                                 unsigned int xoffset,
				 unsigned int yoffset,
                                 unsigned int boffset,
				 unsigned int nbands)
{
    if(! storage->isType(XIL_PIXEL_SEQUENTIAL)) {
        unsigned int i = 0;
        unsigned int b;
        unsigned int end_band = nbands+boffset;
        for(b=boffset; b<end_band; b++) {
            //
            //  bit, byte, short, float  are the ones that are fully
            //  implemented for XIL 1.3 the rest are untested and may
            //  be ifdef'd out for a later release.
            //
            switch(dataType) {
              case XIL_BIT:
              {
              Xil_unsigned8* data =
                  ((Xil_unsigned8*)storage->getDataPtr(b)) +
                  yoffset * storage->getScanlineStride(b);
              unsigned int   offset  = storage->getOffset(b);

	      values[i++] = (float)
                  (XIL_BMAP_TST(data, xoffset + offset) ? 1.0F : 0.0F);
              }
              break;

              case XIL_BYTE:
              {
                  Xil_unsigned8* data =
                      ((Xil_unsigned8*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  values[i++] = _XILI_B2F(*data);
              }
              break;

              case XIL_SHORT:
              {
                  Xil_signed16* data =
                      ((Xil_signed16*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  values[i++] = (float)(*data);
              }
              break;

              case XIL_FLOAT: {
                  Xil_float32* data =
                      ((Xil_float32*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  values[i++] = (float)(*data);
              }
              break;

              //
              //  Untested datatypes begin here
              //
              case XIL_UNSIGNED_4:
              {
                  Xil_unsigned8* data =
                      ((Xil_unsigned8*)storage->getDataPtr(b)) +
                      yoffset      * storage->getScanlineStride(b)  +
                      (xoffset>>1) * storage->getScanlineStride(b);
                  unsigned int   offset = storage->getOffset(b);

                  //
                  //  Set up the bits we want to set the pixel to.
                  //
                  values[i++] = (float) (*data & (0xF0 >> offset));
              }
              break;

              case XIL_SIGNED_8:
              {
                  Xil_signed8* data =
                      ((Xil_signed8*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  values[i++] = (float)(*data);
              }
              break;

              case XIL_UNSIGNED_16:
              {
                  Xil_unsigned16* data =
                      ((Xil_unsigned16*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  values[i++] = (float)(*data);
              }
              break;

              case XIL_SIGNED_32:
              {
                  Xil_signed32* data =
                      ((Xil_signed32*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  values[i++] = (float)(*data);
              }
              break;

              case XIL_UNSIGNED_32:
              {
                  Xil_unsigned32* data =
                      ((Xil_unsigned32*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  values[i++] = (float)(*data);
              }
              break;

              case XIL_SIGNED_64:
              {
                  Xil_signed64* data =
                      ((Xil_signed64*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  values[i++] = (float)(*data);
              }
              break;

              case XIL_UNSIGNED_64:
              {
                  Xil_unsigned64* data =
                      ((Xil_unsigned64*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

#ifdef _WINDOWS
                  values[i++] = (float) ((Xil_signed64)(*data));
#else
                  values[i++] = (float)(*data);
#endif
              }
              break;

              case XIL_FLOAT_64:
              {
                  Xil_float64* data =
                      ((Xil_float64*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  values[i++] = (float)(*data);
              }
              break;

              case XIL_FLOAT_128:
              {
                  Xil_float128* data =
                      ((Xil_float128*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  values[i++] = (float)(*data);
              }
              break;

              case XIL_COMPLEX_FLOAT_32:
              {
                  XilComplexFloat32* data =
                      ((XilComplexFloat32*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  //
                  //  Ordering is real followed by imaginary
                  //
                  values[i++] = (float)data->real;
                  values[i++] = (float)data->img;
              }
              break;

              case XIL_COMPLEX_FLOAT_64: 
              {
                  XilComplexFloat64* data =
                      ((XilComplexFloat64*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  //
                  //  Ordering is real followed by imaginary
                  //
                  values[i++] = (float)data->real;
                  values[i++] = (float)data->img;
              }
              break;

              case XIL_COMPLEX_MAG_FLOAT_32:
              {
                  XilComplexMagFloat32* data =
                      ((XilComplexMagFloat32*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  //
                  //  Ordering is mag followed by phase
                  //
                  values[i++] = (float)data->mag;
                  values[i++] = (float)data->phase;
              }
              break;

              case XIL_COMPLEX_MAG_FLOAT_64:
              {
                  XilComplexMagFloat64* data =
                      ((XilComplexMagFloat64*)storage->getDataPtr(b)) +
                      yoffset * storage->getScanlineStride(b) +
                      xoffset * storage->getPixelStride(b);

                  //
                  //  Ordering is mag followed by phase
                  //
                  values[i++] = (float)data->mag;
                  values[i++] = (float)data->phase;
              }
              break;
            }
        }
    } else { ///////////// XIL_PIXEL_SEQUENTIAL ////////////////////
        unsigned int i = 0;
        unsigned int b;
        unsigned int end_band = nbands+boffset;

	//
	//  byte, short, float  are the ones that are fully
	//  implemented for XIL 1.3 the rest are untested and may
	//  be ifdef'd out for a later release.
	//
	switch(dataType) {
	  case XIL_BYTE:
          {
              Xil_unsigned8* data =
                  ((Xil_unsigned8*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              for(b=boffset; b<end_band; b++) {
                  values[i++] = _XILI_B2F(*(data+b));
              }
	  }
          break;

	  case XIL_SHORT:
          {
              Xil_signed16* data =
                  ((Xil_signed16*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              for(b=boffset; b<end_band; b++) {
                  values[i++] = (float)*(data+b);
              }
	  }
          break;

	  case XIL_FLOAT: {
              Xil_float32* data =
                  ((Xil_float32*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              for(b=boffset; b<end_band; b++) {
                  values[i++] = (float)*(data+b);
              }
	  }
          break;

	  //
	  //  Untested datatypes begin here
	  //
	  case XIL_UNSIGNED_4:
          {
              //
              //  Making this loop implementation easier by writing GENERAL
              //  loop...
              //
              for(b=boffset; b<end_band; b++) {
                  Xil_unsigned8* data =
                      ((Xil_unsigned8*)storage->getDataPtr(b)) +
                      yoffset      * storage->getScanlineStride(b)  +
                      (xoffset>>1) * storage->getScanlineStride(b);
                  unsigned int   offset = storage->getOffset(b);

                  //
                  //  Set up the bits we want to set the pixel to.
                  //
                  values[i++] = (float) (*data & (0xF0 >> offset));
              }
	  }
          break;

	  case XIL_SIGNED_8:
          {
              Xil_signed8* data =
                  ((Xil_signed8*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              for(b=boffset; b<end_band; b++) {
                  values[i++] = (float)*(data+b);
              }
	  }
          break;

	  case XIL_UNSIGNED_16:
          {
              Xil_unsigned16* data =
                  ((Xil_unsigned16*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              for(b=boffset; b<end_band; b++) {
                  values[i++] = (float)*(data+b);
              }
	  }
          break;

	  case XIL_SIGNED_32:
          {
              Xil_signed32* data =
                  ((Xil_signed32*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              for(b=boffset; b<end_band; b++) {
                  values[i++] = (float)*(data+b);
              }
	  }
          break;

	  case XIL_UNSIGNED_32:
          {
              Xil_unsigned32* data =
                  ((Xil_unsigned32*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              for(b=boffset; b<end_band; b++) {
                  values[i++] = (float)*(data+b);
              }
	  }
          break;

	  case XIL_SIGNED_64:
          {
              Xil_signed64* data =
                  ((Xil_signed64*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              for(b=boffset; b<end_band; b++) {
                  values[i++] = (float)*(data+b);
              }
	  }
          break;

	  case XIL_UNSIGNED_64:
          {
              Xil_unsigned64* data =
                  ((Xil_unsigned64*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              for(b=boffset; b<end_band; b++) {
#ifdef _WINDOWS
                  values[i++] = (float) ((Xil_signed64)(*(data+b)));
#else
                  values[i++] = (float)(*(data+b));
#endif
              }
	  }
          break;

	  case XIL_FLOAT_64:
          {
              Xil_float64* data =
                  ((Xil_float64*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              for(b=boffset; b<end_band; b++) {
                  values[i++] = (float)(*(data+b));
              }
	  }
          break;

	  case XIL_FLOAT_128:
          {
              Xil_float128* data =
                  ((Xil_float128*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              for(b=boffset; b<end_band; b++) {
                  values[i++] = (float)(*(data+b));
              }
	  }
          break;

	  case XIL_COMPLEX_FLOAT_32:
          {
              XilComplexFloat32* data =
                  ((XilComplexFloat32*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              //
              //  Ordering is real followed by imaginary
              //
              for(b=boffset; b<end_band; b++) {
                  values[i++] = (float)(data+b)->real;
                  values[i++] = (float)(data+b)->img;
              }
	  }
          break;

	  case XIL_COMPLEX_FLOAT_64: 
          {
              XilComplexFloat64* data =
                  ((XilComplexFloat64*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              //
              //  Ordering is real followed by imaginary
              //
              for(b=boffset; b<end_band; b++) {
                  values[i++] = (float)(data+b)->real;
                  values[i++] = (float)(data+b)->img;
              }
	  }
          break;

	  case XIL_COMPLEX_MAG_FLOAT_32:
          {
              XilComplexMagFloat32* data =
                  ((XilComplexMagFloat32*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              //
              //  Ordering is mag followed by phase
              //
              for(b=boffset; b<end_band; b++) {
                  values[i++] = (float)(data+b)->mag;
                  values[i++] = (float)(data+b)->phase;
              }
	  }
          break;

	  case XIL_COMPLEX_MAG_FLOAT_64:
          {
              XilComplexMagFloat64* data =
                  ((XilComplexMagFloat64*)storage->getDataPtr()) +
                  yoffset * storage->getScanlineStride() +
                  xoffset * storage->getPixelStride();

              //
              //  Ordering is mag followed by phase
              //
              for(b=boffset; b<end_band; b++) {
                  values[i++] = (float)(data+b)->mag;
                  values[i++] = (float)(data+b)->phase;
              }
	  }
          break;
	}
    }

    return XIL_SUCCESS;
}

void*
XilDeviceStorageMemory::allocateChunk(unsigned int size)
{
    //
    //  Allocate the actual storage.
    //
    void* d;
    unsigned int data_offset = 0;
    unsigned int pagesize    = pageSize;

    //
    //  For debugging with Purify, it's better to not use mmap().
    //
#ifndef _XIL_RELEASE_BUILD
    static Xil_boolean xili_always_malloc_is_set = FALSE;
    static Xil_boolean xili_always_malloc;
    if(! xili_always_malloc_is_set) {
        if(getenv("XIL_STORAGE_USE_MALLOC") != NULL) {
            xili_always_malloc = TRUE;
        } else {
            xili_always_malloc = FALSE;
        }

        xili_always_malloc_is_set = TRUE;
    }
#else
    //
    //  Force the flag so it is ignored for release builds.
    //
#define xili_always_malloc 0
#endif

    //
    // In case of WINDOWS we will use malloc for now. If this is not
    // efficient we could use the xili_mmap_file logic to map the
    // system page file for memory. See CreateFileMapping() man page
    // on NT
    //
    if(size < pagesize || xili_always_malloc) {
        //
        //  Too small to allocate a whole page.
        //
        d = malloc(sizeof(XilMemoryDataInfo) + size);
    } else {
        //
        //  Compute how many pages we need to allocate and add one so we can
        //  be sure to be able to offset into it.
        //
        size = (unsigned int)
            roundup(sizeof(XilMemoryDataInfo) + size + pagesize, pagesize);

#ifdef _WINDOWS
        d = VirtualAlloc(NULL, size - pagesize,
                         MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
        if(devZeroFd < 0) {
            d = valloc(size - pagesize);
        } else {
            d = mmap(NULL, size,
                     PROT_READ|PROT_WRITE, MAP_PRIVATE, devZeroFd, 0);

            if(d == (caddr_t)-1) {
                d = valloc(size - pagesize);
            } else {
                data_offset = getNextPtrOffset();

                //
                //  Move the mmap data pointer ahead by data_offset.
                //  data_offset will always be >= sizeof(XilMemoryDataInfo).
                //
                d = ((char*)d) + data_offset;

                //
                //  Move it back by sizeof(XilMemoryDataInfo) so we can fill
                //  in the structure.
                //
                d = ((char*)d) - sizeof(XilMemoryDataInfo);
            }
        }
#endif /* _WINDOWS */
    }

    //
    //  Fill in the XilMemoryDataInfo field.
    //  d can be nil at this point if there is insufficient swap space
    //
    if ( d != NULL ) {
        XilMemoryDataInfo* xmdi = (XilMemoryDataInfo*)d;

        xmdi->dataOffset = data_offset;
        xmdi->dataSize   = size;

        //
        //  Move the data pointer to where the user-data will actually start.
        //
        d = ((char*)d) + sizeof(XilMemoryDataInfo);
    }

    return d;
}

XilStatus
XilDeviceStorageMemory::allocate(XilStorage*      storage,
                                 unsigned int     allocate_xsize,
                                 unsigned int     allocate_ysize,
                                 XilStorageType   type_requested,
                                 XilStorageAccess access,
                                 void*            attribs)
{
    access = access;
    attribs = attribs;

    unsigned int size;

    unsigned int xsize;
    unsigned int ysize;
    unsigned int nbands;
    XilDataType  data_type;
    image->getInfo(&xsize, &ysize, &nbands, &data_type);
    
    if(! xili_is_supported_datatype(data_type)) {
        XIL_ERROR(image->getSystemState(), XIL_ERROR_SYSTEM, "di-157", FALSE);
        return XIL_FAILURE;
    }

#ifndef _XIL_RELEASE_BUILD
    //
    //  If the type_requested is XIL_STORAGE_TYPE_UNDEFINED (any type
    //  acceptable) check an environment variable to tell us which type of
    //  storage to be created. 
    //
    //  TODO: 4/11/96 jlf  This environment variable is a temporary solution
    //                     for debugging purposes.
    //
    //  If the type_requested is XIL_STORAGE_TYPE_UNDEFINED and the
    //  environment variable is set, then the value contained in the
    //  environment variable is used. 
    //
    if(type_requested == XIL_STORAGE_TYPE_UNDEFINED) {
        char* env_type = getenv("XIL_STORAGE_TYPE");

        if(env_type != NULL) {
            if(strcmp(env_type, "XIL_GENERAL") == 0) {
                type_requested = XIL_GENERAL;
            } else if(strcmp(env_type, "XIL_BAND_SEQUENTIAL") == 0) {
                type_requested = XIL_BAND_SEQUENTIAL;
            } else if(strcmp(env_type, "XIL_PIXEL_SEQUENTIAL") == 0) {
                type_requested = XIL_PIXEL_SEQUENTIAL;
            }
        }                
    }
#endif

    //
    //  Setup the information based on the type of storage requested.
    //
    if(type_requested == XIL_GENERAL) {
        for(unsigned int i=0; i<nbands; i++) {
            switch(data_type) {
              case XIL_BIT:
                storage->setScanlineStride((allocate_xsize+XIL_BIT_ALIGNMENT-1)/
                                           XIL_BIT_ALIGNMENT*(XIL_BIT_ALIGNMENT/8), i);
                storage->setOffset(0, i);
                
                size = storage->getScanlineStride(i)*allocate_ysize;
                break;
                
              case XIL_UNSIGNED_4:
                storage->setScanlineStride((allocate_xsize*4+XIL_BIT_ALIGNMENT-1)/
                                           XIL_BIT_ALIGNMENT*(XIL_BIT_ALIGNMENT/8), i);
                storage->setOffset(0, i);

                size = storage->getScanlineStride(i)*allocate_ysize;
                break;
            
              default: // All full byte types
                unsigned int sstride = allocate_xsize;
#ifdef _WINDOWS
                sstride = (sstride + 7) & (~0x7);
#endif
                storage->setScanlineStride(sstride, i);
                storage->setPixelStride(1, i);

                size = sstride*allocate_ysize*xili_sizeof(data_type);
                break;
            }

            //
            //  Allocate data and tell storage object about it
            //
            storage->setDataPtr(allocateChunk(size), i);
    
            if(storage->getDataPtr() == NULL) {
                delete storage;

                XIL_ERROR(image->getSystemState(),
                          XIL_ERROR_RESOURCE, "di-1", TRUE);
                return XIL_FAILURE;
            }
        }
    } else if(type_requested == XIL_BAND_SEQUENTIAL) {
        switch(data_type) {
          case XIL_BIT:
            storage->setScanlineStride((allocate_xsize+XIL_BIT_ALIGNMENT-1)/
                                       XIL_BIT_ALIGNMENT*(XIL_BIT_ALIGNMENT/8));
            storage->setBandStride(storage->getScanlineStride()*allocate_ysize);
            storage->setOffset(0);

            size = (storage->getBandStride()*nbands);
            break;
            
          case XIL_UNSIGNED_4:
            storage->setScanlineStride((allocate_xsize*4+XIL_BIT_ALIGNMENT-1)/
                                       XIL_BIT_ALIGNMENT*(XIL_BIT_ALIGNMENT/8));
            storage->setBandStride(storage->getScanlineStride()*allocate_ysize);
            storage->setOffset(0);
        
            size = (storage->getBandStride()*nbands);
            break;
            
          default: // All full byte types
            unsigned int sstride = allocate_xsize;
#ifdef _WINDOWS
            sstride = (sstride + 7) & (~0x7);
#endif
            storage->setScanlineStride(sstride);
            storage->setBandStride(sstride*allocate_ysize);

            size = storage->getBandStride()*nbands*xili_sizeof(data_type);
            break;
        }

        //
        //  Allocate data and tell storage object about it
        //
        storage->setDataPtr(allocateChunk(size));
    
        if(storage->getDataPtr() == NULL) {
            delete storage;

            XIL_ERROR(image->getSystemState(),
                      XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }
    } else {
        switch(data_type) {
          case XIL_BIT:
            storage->setScanlineStride((allocate_xsize+XIL_BIT_ALIGNMENT-1)/
                                       XIL_BIT_ALIGNMENT*(XIL_BIT_ALIGNMENT/8));
            storage->setBandStride(storage->getScanlineStride()*allocate_ysize);
            storage->setOffset(0);

            size = (storage->getBandStride()*nbands);
            break;
            
          case XIL_UNSIGNED_4:
            storage->setScanlineStride((allocate_xsize*4+XIL_BIT_ALIGNMENT-1)/
                                       XIL_BIT_ALIGNMENT*(XIL_BIT_ALIGNMENT/8));
            storage->setBandStride(storage->getScanlineStride()*allocate_ysize);
            storage->setOffset(0);
        
            size = (storage->getBandStride()*nbands);
            break;
            
          default: // All full byte types
            unsigned int sstride = allocate_xsize * nbands;
#ifdef _WINDOWS
            sstride = (sstride + 7) & (~0x7);
#endif
            storage->setScanlineStride(sstride);
            storage->setPixelStride(nbands);
        
            size = sstride*allocate_ysize*xili_sizeof(data_type);
            break;
        }

        //
        //  Allocate data and tell storage object about it
        //
        storage->setDataPtr(allocateChunk(size));
    
        if(storage->getDataPtr() == NULL) {
            delete storage;

            XIL_ERROR(image->getSystemState(),
                      XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }
    }


    return XIL_SUCCESS;
}

XilStatus
XilDeviceStorageMemory::deallocate(XilStorage* storage)
{
    if(storage->isType(XIL_PIXEL_SEQUENTIAL) ||
       storage->isType(XIL_BAND_SEQUENTIAL)) {
        char*              dptr = (char*)storage->getDataPtr();
        XilMemoryDataInfo* xmdi =
            (XilMemoryDataInfo*)(dptr - sizeof(XilMemoryDataInfo));

        if(xmdi->dataOffset == 0) {
            //
            //  The allocation position is where the XilMemoryDataInfo
            //  structure starts.  We added sizeof(XilMemoryDataInfo) to our
            //  malloc'd data pointer prior to setting it on the storage
            //  object. 
            //
#ifdef _WINDOWS
            VirtualFree(xmdi, 0, MEM_RELEASE);
#else
            free(xmdi);
#endif /* _WINDOWS */
        } else {
#ifndef _WINDOWS
            if(munmap(dptr - xmdi->dataOffset, xmdi->dataSize) == -1) {
                //
                //  TODO: 4/12/96 jlf  An error.
                //
                return XIL_FAILURE;
            }
#endif /* _WINDOWS */
        }
    } else {
        //
        //  XIL_GENERAL storage...do it per-band
        //
        unsigned int bands = storage->getImage()->getNumBands();
        for(unsigned int i=0; i<bands; i++) {
            char*              dptr = (char*)storage->getDataPtr(i);
            XilMemoryDataInfo* xmdi =
                (XilMemoryDataInfo*)(dptr - sizeof(XilMemoryDataInfo));

            if(xmdi->dataOffset == 0) {
                //
                //  The allocation position is where the XilMemoryDataInfo
                //  structure starts.  We added sizeof(XilMemoryDataInfo) to
                //  our malloc'd data pointer prior to setting it on the
                //  storage object. 
                //
#ifdef _WINDOWS
                VirtualFree(xmdi, 0, MEM_RELEASE);
#else
                free(xmdi);
#endif /* _WINDOWS */
            } else {
#ifndef _WINDOWS
                if(munmap(dptr - xmdi->dataOffset, xmdi->dataSize) == -1) {
                    //
                    //  TODO: 4/12/96 jlf  An error.
                    //
                    return XIL_FAILURE;
                }
#endif /* _WINDOWS */
            }
        }
    }

    return XIL_SUCCESS;
}

XilStatus
XilDeviceStorageMemory::deallocateCobble(XilStorage* storage)
{
    return deallocate(storage);
}


//
//  Two-dimensional block copy function.  Uses bytes_per_line fields to move
//  between scanlines. Follows same calling API as the VIS g_copy functions.
//
//  This version does not handle all overlapping copies properly.
//
//  For cobble/decobble there is no need because we're always moving between
//  different buffers.
//
void
XilDeviceStorageMemory::copy2D(void*        dst,
                               unsigned int dst_linebytes,
                               unsigned int width_in_bytes,
                               unsigned int height,
                               void*        src,
                               unsigned int src_linebytes)
{
    Xil_unsigned8* src_line = (Xil_unsigned8*) src;
    Xil_unsigned8* dst_line = (Xil_unsigned8*) dst;
 
    while(height-- > 0) {
        xili_memcpy(dst_line, src_line, width_in_bytes);

        src_line += src_linebytes;
        dst_line += dst_linebytes;
    }
}

//
// Loop thru the tile list and pick the most common storage type
//
Xil_boolean
XilDeviceStorageMemory::diverseTileStorage(XilTileList*    tile_list,
                                           XilStorageType* dst_storage_type,
                                           Xil_boolean*    diverse_ps)
{

    //
    // Loop through the tiles first and determine if they all have
    // the same storage organization.
    // In the PIXEL_SEQUENTIAL case we also want to see if all of the
    // tiles have the same pixel stride. If so, we can retain that
    // pixel stride in the cobbled data and use memcpy to do the moves.
    // Otherwise we have to copy only the required bands from
    // the tiles with extra bands.
    //

    // Check storage type of all tiles; make a histogram
    int num_pix_seq_tiles  = 0;
    int num_band_seq_tiles = 0;
    int num_arb_tiles      = 0;
    int old_ps             = 0;
    int new_ps;
    XilStorage* tile_sd;

    *diverse_ps = FALSE;

    XilTile*    tile;
    Xil_boolean first_loop = TRUE;
    while(tile = tile_list->getNext()) {
        tile_sd = tile->getStorageDescription();

        if(tile_sd->isType(XIL_PIXEL_SEQUENTIAL)) {
            num_pix_seq_tiles++;
            new_ps = tile_sd->getPixelStride();
            if(! first_loop && new_ps != old_ps) {
                *diverse_ps = TRUE;
            }
            old_ps = new_ps;
        } else if(tile_sd->isType(XIL_BAND_SEQUENTIAL)) {
            num_band_seq_tiles++;
        } else if(tile_sd->isType(XIL_GENERAL)) {
            num_arb_tiles++;
        }

        first_loop = FALSE;
    }

    //
    // Force the output to be XIL_BAND_SEQUENTIAL unless
    // ALL of the tiles are XIL_PIXEL_SEQUENTIAL
    //
    Xil_boolean  diverse_storage;
    unsigned int num_tiles = tile_list->getNumTiles();
    if((unsigned int) num_pix_seq_tiles == num_tiles) {
        *dst_storage_type = XIL_PIXEL_SEQUENTIAL;
        diverse_storage = FALSE;
    } else if((unsigned int) num_band_seq_tiles == num_tiles) {
        *dst_storage_type = XIL_BAND_SEQUENTIAL;
        diverse_storage = FALSE;
    } else if((unsigned int) num_arb_tiles == num_tiles) {
        *dst_storage_type = XIL_GENERAL;
        diverse_storage = FALSE;
    } else {
        diverse_storage = TRUE;
        *dst_storage_type = XIL_GENERAL;
    }

    return diverse_storage;
}

//
// Intersect tile with box. Return the clipped
// intra-tile coordinates. Need to add absolute
// tile origin to get actual image space coordinates.
//
void
XilDeviceStorageMemory::intersectTile(XilTile*       tile,
                                      XilBox*        box,
                                      unsigned int*  x1, 
                                      unsigned int*  y1,
                                      unsigned int*  w, 
                                      unsigned int*  h)
{
    //
    // Get the tile size from the image
    //
    unsigned int txsize;
    unsigned int tysize;

    //
    // Get the box description (area to be cobbled).
    //
    int          box_x1;
    int          box_y1;
    unsigned int box_w;
    unsigned int box_h;
    int          box_band;
    getBoxStorageLocation(box,&box_x1, &box_y1, &box_w, &box_h, &box_band);

    int          box_x2 = box_x1 + box_w - 1;
    int          box_y2 = box_y1 + box_h - 1;

    //
    // Get the source tile coordinates
    //

    //
    // NOTE - txsize and tysize must be retrieved explicitly from the tile in
    // question because this routine is used while changing from one
    // tilesize to another, and the image's tilesize has not yet been
    // reset to reflect the new tilesize.
    //

    int src_x1;
    int src_y1;

    tile->getBox()->getAsRect(&src_x1,&src_y1,&txsize,&tysize);

    unsigned int src_x2 = src_x1 + txsize - 1;
    unsigned int src_y2 = src_y1 + tysize - 1;

    //
    // Clip tile against the bounding box
    // of the area to be cobbled.
    //

    // Clip tile start against box (left)
    if(src_x1 < box_x1) {
        src_x1 = box_x1 % txsize;
    } else {
        src_x1 = 0;
    }
    *x1 = src_x1;

    // Clip tile start against box (top)
    if(src_y1 < box_y1) {
        src_y1 = box_y1 % tysize;
    } else {
        src_y1 = 0;
    }
    *y1 = src_y1;

    // Clip tile end against box (right)
    if(src_x2 > (unsigned int) box_x2) {
        src_x2 = box_x2 % txsize;
    } else {
        src_x2 = txsize - 1;
    }
    *w = src_x2 - src_x1 + 1;

    // Clip tile end against box (bottom)
    if(src_y2 > (unsigned int) box_y2) {
        src_y2 = box_y2 % tysize;
    } else {
        src_y2 = tysize - 1;
    }
    *h = src_y2 - src_y1 + 1;

}

//
// Utility function to calculate the starting address of an arbitrary point
// within a storage region. Also returns the ps, ss, bs, and offset.
// This routine will work with any data type and storage organization.
//
Xil_unsigned8*
XilDeviceStorageMemory::calcAddress(XilStorage*   storage,
                                    unsigned int  band,
                                    unsigned int  x1,
                                    unsigned int  y1,
                                    unsigned int* ps,
                                    unsigned int* ss,
                                    unsigned int* bs,
                                    unsigned int* offset)
{
    void*        dataptr;
    unsigned int bit_width;
    Xil_boolean  bit_type;

    XilDataType  datatype  = storage->getImage()->getDataType();
    unsigned int unit_size = xili_sizeof(datatype);

    if(datatype == XIL_BIT) {
        bit_width = 1;
        bit_type = TRUE;
    } else if(datatype == XIL_UNSIGNED_4) {
        bit_width = 4;
        bit_type = TRUE;
    } else {
        bit_type = FALSE;
    }

    char* addr;
    if(storage->isType(XIL_BAND_SEQUENTIAL) || storage->isType(XIL_PIXEL_SEQUENTIAL)) {
        storage->getStorageInfo(ps, ss, bs, offset, &dataptr);
        addr = (char*)dataptr + unit_size * (band*(*bs) + y1*(*ss) + x1*(*ps)); 
    } else {
        storage->getStorageInfo(band, ps, ss, offset, &dataptr);
        addr = (char*)dataptr + unit_size * (y1*(*ss) + x1* (*ps)); 
        *bs = 0;
    }

    //
    // For BIT and UNSIGNED_4 types, calculate the starting
    // address and offset using the bit width.
    //
    if(bit_type) {
        addr = (char*)dataptr + band*(*bs) + y1*(*ss) +
            (*offset + x1*bit_width) / XIL_BIT_ALIGNMENT;

        *offset = (*offset + x1*bit_width) % XIL_BIT_ALIGNMENT;
    } 

    return (Xil_unsigned8*)addr;

}

//
//
// XilDeviceStorageMemory::cobble(
//     XilStorage*     dst_storage,   // Cobbled storage object returned
//     XilBox*         box,           // Box to be cobbled
//     XilStorageType  type_requested // storage type to be returned
//     unsigned int    num_tiles,     // # of tiles intersecting box
//     XilTile** tile_list            // List of tiles intersecting box
//     void*           attribs        // attributes passed through.
//
//  Reformat a box of storage from a tiled to a contiguous representation. The
//    storage will be formated to type_requested if possible.
//
//  If the type_requested is XIL_STORAGE_TYPE_UNDEFINED (default) then uniform
//    PIX_SEQ storage remains PIX_SEQ and everything else is converted to
//    BAND_SEQ, even GENERAL storage. 
//
//
XilStatus
XilDeviceStorageMemory::cobble(XilStorage*     dst_storage,
                               XilBox*         box,
                               XilTileList*    tile_list,
                               XilStorageType  type_requested,
                               void*           )
{
    //
    // Get the box description (area to be cobbled).
    //
    int          box_x1;
    int          box_y1;
    unsigned int box_w;
    unsigned int box_h;
    int          box_band;
    getBoxStorageLocation(box,
                          &box_x1, &box_y1, &box_w, &box_h, &box_band);

    //
    // Get image parameters - bands, datatype, tilesize, unit_size
    // These will be constant for all tiles
    //
    XilImage*     dst_image = dst_storage->getImage();
    unsigned int  nbands    = dst_image->getNumBands();
    XilDataType   datatype  = dst_image->getDataType();;
    unsigned int  unit_size = xili_sizeof(datatype);

    //
    // Examine the storage types of all the source tiles.
    // If they are all the same, then
    // get the storage description from the first tile in the list.
    // Unless all tiles are XIL_PIXEL_SEQUENTIAL, create the destination
    // storage as XIL_BAND_SEQUENTIAL.
    //
    XilStorage*       tile_sd;
    Xil_boolean       diverse_ps;
    XilStorageType    src_storage_type;
    XilStorageType    dst_storage_type;
    Xil_boolean       diverse_tiles;

    //
    //  src_storage_type will return as XIL_GENERAL unless the tiles are of
    //  the same type
    // 
    diverse_tiles = diverseTileStorage(tile_list,
                                       &src_storage_type, &diverse_ps);

    //
    //  If the caller doesn't care what type is returned, we only return
    //  PIXEL_SEQUENTIAL.  If all the src tiles are PIXEL_SEQUENTIAL,
    //  otherwise we return BAND_SEQUENTIAL. 
    //
    if(type_requested == XIL_STORAGE_TYPE_UNDEFINED) {
        if(src_storage_type == XIL_PIXEL_SEQUENTIAL) {
            dst_storage_type = XIL_PIXEL_SEQUENTIAL;
        } else {
            dst_storage_type = XIL_BAND_SEQUENTIAL;
        }
    } else {
        dst_storage_type = type_requested;
    }

    //
    //  You can't request XIL_PIXEL_SEQUENTIAL for BIT and UNSIGNED4 images.
    //
    if((dst_storage_type == XIL_PIXEL_SEQUENTIAL) &&
       ((datatype == XIL_BIT) || (datatype == XIL_UNSIGNED_4))) {
        //
        // TODO: maynard 3/8/96
        //       should this generate an error first?
        //
        return XIL_FAILURE;
    }

    //
    //  Next we calculate the parameters needed in order to allocate the dest
    //  storage 
    //
    unsigned int  dst_ps, dst_ss, dst_bs, dst_offset;
    unsigned int  bit_width;
    unsigned int  nbytes;

    //
    //  Describe the destination (cobbled) storage
    //  so that we can allocate the memory and
    //  fill out the storage object parameters.
    //  Use the pixel_stride of the source tiles for
    //  the destination, unless the stride varies among tiles.
    //
    if(dst_storage_type == XIL_PIXEL_SEQUENTIAL) {
        if((src_storage_type == XIL_PIXEL_SEQUENTIAL) && !diverse_ps && (box_band == 0)) {
            //
            // In this case, ALL src tiles are PIX_SEQ and the all have the
            // same pixel stride.
            //
            // It allows for a memcpy optimization - use ALL channels
            //
            // TODO: maynard/link 3/7/96
            //       There is a potential size tradeoff if the nbands is more
            //       than you really need 
            //
            tile_sd = tile_list->getTile(0)->getStorageDescription();
            dst_ps  = tile_sd->getPixelStride();
        } else {
            //
            // src tiles are of variable type - use only NBANDS channels
            //
            dst_ps = nbands;
        }

        dst_bs = 1;
        dst_ss = box_w*dst_ps;
        nbytes = dst_ss * box_h * unit_size;
    } else {
        //
        //  Handle everything else as BAND_SEQUENTIAL - even GENERAL can be
        //  viewed this way.
        //
        dst_ps = 1;
        switch(datatype) {
          case XIL_BIT:
            dst_ss = ((box_w + XIL_BIT_ALIGNMENT-1)/
                               XIL_BIT_ALIGNMENT*(XIL_BIT_ALIGNMENT/8));
            bit_width = 1;
            break;
          case XIL_UNSIGNED_4:
            dst_ss = ((box_w*4 + XIL_BIT_ALIGNMENT-1)/
                                 XIL_BIT_ALIGNMENT*(XIL_BIT_ALIGNMENT/8));
            bit_width = 4;
            break;
          default:
            dst_ss = box_w;
        }

        dst_bs = dst_ss * box_h;

        nbytes = dst_bs * nbands * unit_size;
    } 

    //
    //  Allocate the memory for the cobbled region
    //
    Xil_unsigned8* dst_dataptr = (Xil_unsigned8*)allocateChunk(nbytes);
    if(dst_dataptr == NULL) {
        XIL_ERROR(dst_image->getSystemState(),
                  XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    //
    //  Set storage description parameters for the cobbled (dst) storage
    //
    dst_storage->setPixelStride(dst_ps);
    dst_storage->setScanlineStride(dst_ss);
    dst_storage->setBandStride(dst_bs);
    dst_storage->setOffset(0); // New storage, so start at zero
    dst_storage->setDataPtr((void*)dst_dataptr);

    //
    // OK, we've described how the destination (cobbled) storage
    // will be organized. Now let's step through the tiles and
    // copy their data which intersects the box into the cobbled
    // storage buffer.
    //
    XilTile* tile;
    while(tile = tile_list->getNext()) {
        //
        // Intersect this tile with the box. This gets
        // the region to be copied into the cobbled area.
        //
        unsigned int src_x1;
        unsigned int src_y1;
        unsigned int src_w;
        unsigned int src_h;
        intersectTile(tile, box, &src_x1, &src_y1, &src_w, &src_h);

        //
        // Get the tile origin coords in order to calculate
        // the absolute coords of the sub-region to be copied.
        //
        int          tile_xorigin;
        int          tile_yorigin;
        unsigned int tmp_xs;
        unsigned int tmp_ys;
        tile->getBox()->getAsRect(&tile_xorigin,
                                  &tile_yorigin,
                                  &tmp_xs, &tmp_ys);

        //
        // Calculating the dst coords. Doing it this way avoids making any 
        // assumptions about the order of the tiles in the list.
        // Each one just goes in its proper place in the cobbled dst area
        //
        unsigned int dst_x1 = tile_xorigin + src_x1 - box_x1;
        unsigned int dst_y1 = tile_yorigin + src_y1 - box_y1;

        unsigned int src_ps, src_ss, src_bs, src_offset;

        //
        // Get this tile's storage description
        //
        tile_sd = tile->getStorageDescription();

        unsigned int line;
        unsigned int band;

        if(dst_storage_type == XIL_PIXEL_SEQUENTIAL) {
            if(tile_sd->isType(XIL_PIXEL_SEQUENTIAL)) {
                //
                // We can use memcpy here to copy contiguous data
                //
                // Note: For speed, this copies all bands of the source, 
                //       even if all are not used. The assumption is that
                //       cobbled areas will be small, like for convolution
                //       edge areas.
                //       I know that I will have allocated enough memory for this
                //       "optimized" case.
                // TODO: Test the size of the area. If the area is large,
                //       like when a whole image is being converted, we may
                //       only want to copy the actual bands.
                // 
                
                //
                // Get the starting addresses of the sub-region to be copied
                //
                Xil_unsigned8* src =
                    calcAddress(tile_sd, box_band, src_x1, src_y1, 
                                &src_ps, &src_ss, &src_bs, &src_offset);
                
                Xil_unsigned8* dst =
                    calcAddress(dst_storage, 0, dst_x1, dst_y1,
                                &dst_ps, &dst_ss, &dst_bs, &dst_offset);
                
                if(dst_ps == src_ps) {
                    copy2D(dst, dst_ss*unit_size,
                           src_w*src_ps*unit_size,
                           src_h, src, src_ss*unit_size);
                } else {
                    //
                    // Must copy byte-by-byte to deal with all data types,
                    // Slow, but diverse pixel_stride case should be rare.
                    //
                    int bytes_to_copy = nbands * unit_size;
                    for(line=0; line<src_h; line++) {
                        Xil_unsigned8* src_pixel = src;
                        Xil_unsigned8* dst_pixel = dst;
                        for(int samp=src_w+1; --samp; ) {
                            Xil_unsigned8* sptr = src_pixel;
                            Xil_unsigned8* dptr = dst_pixel;
                            for(int i=bytes_to_copy+1; --i; ) {
                                *dptr++ = *sptr++;
                            }
                            src_pixel += src_ps * unit_size;
                            dst_pixel += dst_ps * unit_size;
                        }
                        src += src_ss * unit_size;
                        dst += dst_ss * unit_size;
                    }
                }
            } else {
                //
                // Handle BAND_SEQ and GENERAL storage for source tiles
                // in essentially the same manner, except get GENERAL 
                // parameters band-wise, versus using constant 
                // parameters for BAND_SEQ.
                // Handle BYTE, SHORT and FLOAT with special cases.
                // Put all other types through the byte-by-byte code.
                // There are no BIT or UNSIGNED_4 PIXEL_SEQ images.
                //
                for(band=0; band<nbands; band++) {
                    Xil_unsigned8* src_addr = 
                        calcAddress(tile_sd, band + box_band, src_x1, src_y1, 
                                    &src_ps, &src_ss, &src_bs, &src_offset);
                    Xil_unsigned8* dst_addr = 
                        calcAddress(dst_storage, band, dst_x1, dst_y1,
                                    &dst_ps, &dst_ss, &dst_bs, &dst_offset);

                    switch(datatype) {
                      case XIL_BIT: 
                      case XIL_UNSIGNED_4:
                      {
                          //
                          // THIS SHOULD NEVER HAPPEN! I PRE-CHECK THE IMAGE.
                          //
                          break;
                      }

                      case XIL_BYTE:
                      case XIL_SIGNED_8:
                      {
                          Xil_unsigned8* src_scan = src_addr;
                          Xil_unsigned8* dst_scan = dst_addr;

                          for(line=0; line<src_h; line++) {
                              Xil_unsigned8* src_pixel = src_scan;
                              Xil_unsigned8* dst_pixel = dst_scan;

                              for(int samp=src_w+1; --samp; ) {
                                  *dst_pixel = *src_pixel++;

                                  dst_pixel += dst_ps;
                              }

                              src_scan += src_ss;
                              dst_scan += dst_ss;
                          }
                          break;
                      }

                      case XIL_SHORT:
                      case XIL_UNSIGNED_16:
                      {
                          Xil_signed16* src_scan = (Xil_signed16*)src_addr;
                          Xil_signed16* dst_scan = (Xil_signed16*)dst_addr;

                          for(line=0; line<src_h; line++) {
                              Xil_signed16* src_pixel = src_scan;
                              Xil_signed16* dst_pixel = dst_scan;

                              for(int samp=src_w+1; --samp; ) {
                                  *dst_pixel = *src_pixel++;

                                  dst_pixel += dst_ps;
                              }

                              src_scan += src_ss;
                              dst_scan += dst_ss;
                          }
                          break;
                      }

                      case XIL_FLOAT:
                      {
                          Xil_float32* src_scan = (Xil_float32*)src_addr;
                          Xil_float32* dst_scan = (Xil_float32*)dst_addr;

                          for(line=0; line<src_h; line++) {
                              Xil_float32* src_pixel = src_scan;
                              Xil_float32* dst_pixel = dst_scan;

                              for(int samp=src_w+1; --samp; ) {
                                  *dst_pixel = *src_pixel++;

                                  dst_pixel += dst_ps;
                              }

                              src_scan += src_ss;
                              dst_scan += dst_ss;
                          }
                          break;
                      }

                      default:
                      {
                          //
                          //  Byte-by-byte copy for extended types
                          //
                          Xil_unsigned8* src_scan = src_addr;
                          Xil_unsigned8* dst_scan = dst_addr;

                          for(line=0; line<src_h; line++) {
                              Xil_unsigned8* src_pixel = src_scan;
                              Xil_unsigned8* dst_pixel = dst_scan;

                              for(int samp=src_w+1; --samp; ) {
                                  Xil_unsigned8* src_byte = src_pixel;
                                  Xil_unsigned8* dst_byte = dst_pixel;

                                  for(int byte=unit_size+1; --byte; ) {
                                      *dst_byte++ = *src_byte++;
                                  }

                                  dst_pixel += dst_ps*unit_size;
                                  src_pixel += unit_size;
                              }

                              src_scan += src_ss*unit_size;
                              dst_scan += dst_ss*unit_size;
                          }
                          break;
                      }
                    } // End datatype switch for BAND_SEQ/GEN src storage type

                } // End band loop
                
            } // End BAND_SEQ/GEN src storage type case

        } else {  // End PIXEL_SEQ dst storage type case
            //
            // XIL_BAND_SEQ dst storage for all remaining cases.
            // Even fill in GENERAL storage as BAND_SEQUENTIAL whose bands just so happen
            // to be contiguous.
            // Switch on the src_storage_type.
            //
            if(tile_sd->isType(XIL_BAND_SEQUENTIAL)) {
                //
                // BAND_SEQ source tile. We can use memcpy here,
                // since both src and dst are BAND_SEQ
                //
                Xil_unsigned8* src =
                    calcAddress(tile_sd, box_band, src_x1, src_y1, 
                                &src_ps, &src_ss, &src_bs, &src_offset);

                Xil_unsigned8* dst =
                    calcAddress(dst_storage, 0, dst_x1, dst_y1,
                                &dst_ps, &dst_ss, &dst_bs, &dst_offset);

                for(band=0; band<nbands; band++) {
                    Xil_unsigned8* sptr = src;
                    Xil_unsigned8* dptr = dst;

                    for(line=0; line<src_h; line++) {
                        switch(datatype) {
                          case XIL_BIT:
                          case XIL_UNSIGNED_4:
                            xili_bit_memcpy(sptr, dptr, src_w*bit_width,
                                            src_offset, dst_offset);
                            break;
                          default:
                            xili_memcpy(dptr, sptr, src_w*unit_size);
                            break;
                        }

                        sptr += src_ss * unit_size;
                        dptr += dst_ss * unit_size;
                    }

                    src += src_bs * unit_size;
                    dst += dst_bs * unit_size;
                }
            } else {
                //
                // Handle PIXEL_SEQ and GENERAL storage for source tiles
                // in essentially the same manner, except get GENERAL 
                // parameters band-wise, versus using constant 
                // parameters for PIX_SEQ.
                // Handle BYTE, SHORT and FLOAT with special cases.
                // Put all other types through the byte-by-byte code.
                // There are no BIT or UNSIGNED_4 PIXEL_SEQ images.
                //
                for(band=0; band<nbands; band++) {
                    Xil_unsigned8* src_addr = 
                        calcAddress(tile_sd, band + box_band, src_x1, src_y1, 
                                    &src_ps, &src_ss, &src_bs, &src_offset);
                    Xil_unsigned8* dst_addr = 
                        calcAddress(dst_storage, band, dst_x1, dst_y1,
                                    &dst_ps, &dst_ss, &dst_bs, &dst_offset);

                    switch(datatype) {
                      case XIL_BIT: 
                      case XIL_UNSIGNED_4:
                      {
                        Xil_unsigned8* src_scan = src_addr;
                        Xil_unsigned8* dst_scan = dst_addr;

                        for(line=0; line<src_h; line++) {
                            xili_bit_memcpy(src_scan, dst_scan,
                                           src_w*bit_width, 
                                           src_offset, dst_offset);

                            src_scan += src_ss;
                            dst_scan += dst_ss;
                        }
                        break;
                      }
                      case XIL_BYTE:
                      case XIL_SIGNED_8:
                      {
                        Xil_unsigned8* src_scan = src_addr;
                        Xil_unsigned8* dst_scan = dst_addr;

                        for(line=0; line<src_h; line++) {
                            Xil_unsigned8* src_pixel = src_scan;
                            Xil_unsigned8* dst_pixel = dst_scan;

                            for(int samp=src_w+1; --samp; ) {
                                *dst_pixel++ = *src_pixel;

                                src_pixel += src_ps;
                            }

                            src_scan += src_ss;
                            dst_scan += dst_ss;
                        }
                        break;
                      }
                      case XIL_SHORT:
                      case XIL_UNSIGNED_16:
                      {
                        Xil_signed16* src_scan = (Xil_signed16*)src_addr;
                        Xil_signed16* dst_scan = (Xil_signed16*)dst_addr;

                        for(line=0; line<src_h; line++) {
                            Xil_signed16* src_pixel = src_scan;
                            Xil_signed16* dst_pixel = dst_scan;

                            for(int samp=src_w+1; --samp; ) {
                                *dst_pixel++ = *src_pixel;

                                src_pixel += src_ps;
                            }

                            src_scan += src_ss;
                            dst_scan += dst_ss;
                        }
                        break;
                      }
                      case XIL_FLOAT:
                      {
                        Xil_float32* src_scan = (Xil_float32*)src_addr;
                        Xil_float32* dst_scan = (Xil_float32*)dst_addr;

                        for(line=0; line<src_h; line++) {
                            Xil_float32* src_pixel = src_scan;
                            Xil_float32* dst_pixel = dst_scan;

                            for(int samp=src_w+1; --samp; ) {
                                *dst_pixel++ = *src_pixel;

                                src_pixel += src_ps;
                            }

                            src_scan += src_ss;
                            dst_scan += dst_ss;
                        }
                        break;
                      }
                      default:
                      {
                        //
                        // Byte-by-byte copy for extended types
                        //
                        Xil_unsigned8* src_scan = src_addr;
                        Xil_unsigned8* dst_scan = dst_addr;

                        for(line=0; line<src_h; line++) {
                            Xil_unsigned8* src_pixel = src_scan;
                            Xil_unsigned8* dst_pixel = dst_scan;

                            for(int samp=src_w+1; --samp; ) {
                                Xil_unsigned8* src_byte = src_pixel;
                                Xil_unsigned8* dst_byte = dst_pixel;

                                for(int byte=unit_size+1; --byte; ) {
                                    *dst_byte++ = *src_byte++;
                                }

                                dst_pixel += unit_size;
                                src_pixel += src_ps*unit_size;
                            }

                            src_scan += src_ss*unit_size;
                            dst_scan += dst_ss*unit_size;
                        }
                        break;
                      }

                    } // End datatype switch for PIX_SEQ/GEN src storage type

                } // End band loop

            } // End PIX_SEQ/GEN src storage type case
            
        } // End BAND_SEQ dst storage type case
        
    } // End tile_list loop
    
    return XIL_SUCCESS;
}


XilStatus
XilDeviceStorageMemory::decobble(XilStorage*     src_storage,
                                 XilBox*         box,
                                 XilTileList*    tile_list)
{
    //
    // Get box description
    //
    int          box_x1;
    int          box_y1;
    unsigned int box_w;
    unsigned int box_h;
    int          box_band;
    getBoxStorageLocation(box,&box_x1, &box_y1, &box_w, &box_h, &box_band);

    //
    // Get info about the image this storage is from.
    //
    XilImage*    src_image      = src_storage->getImage();
    XilDataType  datatype       = src_image->getDataType();    
    unsigned int unit_size      = xili_sizeof(datatype);
    unsigned int nbands         = src_image->getNumBands();

    unsigned int bit_width;
    switch(datatype) {
      case XIL_BIT:
        bit_width = 1;
        break;
      case XIL_UNSIGNED_4:
        bit_width = 4;
        break;
      default:
        bit_width = 8;
        break;
    }

    //
    // Loop thru tiles and copy the relevant area out of the
    // source storage rectangle into the destination tile
    //
    XilTile* tile;
    while(tile = tile_list->getNext()) {
        XilStorage* tile_sd = tile->getStorageDescription();

        // Intersect tile with box
        unsigned int dst_x1, dst_y1, dst_w, dst_h;
        intersectTile(tile, box, &dst_x1, &dst_y1, &dst_w, &dst_h);
        
        //
        // Calculate the src location where this tile's new data
        // will come from. Doing it this way removes any dependency
        // on the order of tiles in the list.
        //
        int          tile_xorigin;
        int          tile_yorigin;
        unsigned int tmp_xs;
        unsigned int tmp_ys;
        tile->getBox()->getAsRect(&tile_xorigin,
                                  &tile_yorigin,
                                  &tmp_xs, &tmp_ys);

        unsigned int src_x1 = tile_xorigin + dst_x1 - box_x1;
        unsigned int src_y1 = tile_yorigin + dst_y1 - box_y1;

        unsigned int src_ps, src_ss, src_bs, src_offset;
        unsigned int dst_ps, dst_ss, dst_bs, dst_offset;

        unsigned int line;
        int samp;
        unsigned int band;

        //
        //  This code was originally written assuming that the src type
        //  could not be of GENERAL type. It can.
        //  However the loop that handles dest tile storage of GENERAL
        //  type will also handle src GENERAL storage so use this
        //  if either storage is of type GENERAL. 
        //  This isn't a common use of decobble so performance
        //  shouldn't be a big issue.

        if(tile_sd->isType(XIL_GENERAL) || src_storage->isType(XIL_GENERAL)) {
            for(band=0; band<nbands; band++) {
                Xil_unsigned8* src_scan = 
                    calcAddress(src_storage, band, src_x1, src_y1, 
                                &src_ps, &src_ss, &src_bs, &src_offset);
                
                Xil_unsigned8* dst_scan = 
                    calcAddress(tile_sd, band + box_band, dst_x1, dst_y1,
                                &dst_ps, &dst_ss, &dst_bs, &dst_offset);
                
                for(line=0; line<dst_h; line++) {
                    Xil_unsigned8* src_pixel = src_scan;
                    Xil_unsigned8* dst_pixel = dst_scan;
                    
                    switch(datatype) {
                      case XIL_BIT:
                      case XIL_UNSIGNED_4:
                        xili_bit_memcpy(src_scan, dst_scan, 
                                        dst_w*bit_width,
                                        src_offset, dst_offset);
                        break;
                        
                      case XIL_BYTE:
                        for(samp=dst_w+1; --samp; ) {
                            *dst_pixel = *src_pixel;
                            
                            src_pixel += src_ps;
                            dst_pixel += dst_ps;
                        }
                        break;
                        
                      case XIL_SHORT:
                        for(samp=dst_w+1; --samp; ) {
                            *((Xil_signed16*)dst_pixel) = *((Xil_signed16*)src_pixel);
                            
                            src_pixel =
                                (Xil_unsigned8*)(((Xil_unsigned16*)src_pixel) + src_ps);
                            dst_pixel =
                                (Xil_unsigned8*)(((Xil_unsigned16*)dst_pixel) + dst_ps);
                        }
                        break;
                        
                      case XIL_FLOAT:
                        for(samp=dst_w+1; --samp; ) {
                            *((Xil_float32*)dst_pixel) = *((Xil_float32*)src_pixel);
                            
                            src_pixel =
                                (Xil_unsigned8*)(((Xil_float32*)src_pixel) + src_ps);
                            dst_pixel =
                                (Xil_unsigned8*)(((Xil_float32*)dst_pixel) + dst_ps);
                        }
                        break;
                        //
                        // Handle other types byte-by-byte
                        //
                      default:
                        for(samp=dst_w+1; --samp; ) {
                            Xil_unsigned8* src_byte = src_pixel;
                            Xil_unsigned8* dst_byte = dst_pixel;
                            
                            for(int count=unit_size+1; --count; ) {
                                *dst_byte++ = *src_byte++;
                            }
                            
                            src_pixel += src_ps * unit_size;
                            dst_pixel += dst_ps * unit_size;
                        }
                        break;
                    } // End switch(datatype)
                    src_scan += src_ss * unit_size;
                    dst_scan += dst_ss * unit_size;
                } //End line loop
                
            } // End band loop
            
        } else {
            //
            //  Since we know neither storage type is GENERAL
            //  we know that we can access all bands from the
            //  0th band and the band stride.
            //
            Xil_unsigned8* src_scan =
                calcAddress(src_storage, 0, src_x1, src_y1, 
                            &src_ps, &src_ss, &src_bs, &src_offset);
            
            Xil_unsigned8* dst_scan =
                calcAddress(tile_sd, box_band, dst_x1, dst_y1,
                            &dst_ps, &dst_ss, &dst_bs, &dst_offset);

            //
            //  Well we know that neither storage is of type GENERAL
            //  So now let's try for some other optimization before
            //  falling back on a generalized processing algorithm.
            //
            if( tile_sd->isType(XIL_PIXEL_SEQUENTIAL) &&
                src_storage->isType(XIL_PIXEL_SEQUENTIAL) && dst_ps==src_ps) {
                //
                // If the output type is PIXEL_SEQUENTIAL
                // do some optimization for src type that matches
                //
                copy2D(dst_scan, dst_ss*unit_size,
                       dst_w*dst_ps*unit_size, 
                       dst_h, src_scan, src_ss*unit_size);
            } else if(tile_sd->isType(XIL_BAND_SEQUENTIAL) &&
                      src_storage->isType(XIL_BAND_SEQUENTIAL)) {
                for(band=0; band<nbands; band++) {
                    Xil_unsigned8* sptr = src_scan;
                    Xil_unsigned8* dptr = dst_scan;
                    
                    for(line=0; line<dst_h; line++) {
                        switch(datatype) {
                          case XIL_BIT:
                          case XIL_UNSIGNED_4:
                            xili_bit_memcpy(sptr, dptr, dst_w*bit_width,
                                            src_offset, dst_offset);
                            break;
                            
                          default:
                            xili_memcpy(dptr, sptr, dst_w*unit_size);
                            break;
                        }
                        
                        sptr += src_ss * unit_size;
                        dptr += dst_ss * unit_size;
                    }
                    
                    src_scan += src_bs * unit_size;
                    dst_scan += dst_bs * unit_size;
                } //  End optimization of BS to BS storage
            } else {
                //
                //  We know that we have either some combination of
                //  PIXEL_SEQUENTIAL and BAND_SEQUENTIAL and we'll have
                //  to process with ps and bs and ss.
                //
                switch(datatype) {
                    //
                    //  We know that BIT and U4 cannot be PS ever,
                    //  and since they're both BS we can do a straight
                    //  copy...
                    //
                  case XIL_BIT:
                  case XIL_UNSIGNED_4:
                    xili_bit_memcpy(src_scan, dst_scan, 
                                    dst_w*bit_width,
                                    src_offset, dst_offset);
                    break;
                  case XIL_BYTE:
                    for(line=0; line<dst_h; line++) {
                        Xil_unsigned8* src_pixel = src_scan;
                        Xil_unsigned8* dst_pixel = dst_scan;
                        
                        for(samp=dst_w+1; --samp; ) {
                            Xil_unsigned8* src_band = src_pixel;
                            Xil_unsigned8* dst_band = dst_pixel;
                            
                            for(band=nbands+1; --band; ) {
                                *dst_band = *src_band;
                                
                                src_band += src_bs;
                                dst_band += dst_bs;
                            }
                            
                            src_pixel += src_ps;
                            dst_pixel += dst_ps;
                        }
                        
                        src_scan += src_ss;
                        dst_scan += dst_ss;
                    }
                    break;
                    
                  case XIL_SHORT:
                    for(line=0; line<dst_h; line++) {
                        Xil_signed16* src_pixel = (Xil_signed16*)src_scan;
                        Xil_signed16* dst_pixel = (Xil_signed16*)dst_scan;
                        
                        for(samp=dst_w+1; --samp; ) {
                            Xil_signed16* src_band = src_pixel;
                            Xil_signed16* dst_band = dst_pixel;
                            
                            for(band=nbands+1; --band; ) {
                                *dst_band = *src_band;
                                
                                src_band += src_bs;
                                dst_band += dst_bs;
                            }
                            
                            src_pixel += src_ps;
                            dst_pixel += dst_ps;
                        }
                        
                        src_scan = (Xil_unsigned8*)(((Xil_signed16*)src_scan) + src_ss);
                        dst_scan = (Xil_unsigned8*)(((Xil_signed16*)dst_scan) + dst_ss);
                    }
                    break;
                    
                  case XIL_FLOAT:
                    for(line=0; line<dst_h; line++) {
                        Xil_float32* src_pixel = (Xil_float32*)src_scan;
                        Xil_float32* dst_pixel = (Xil_float32*)dst_scan;
                        
                        for(samp=dst_w+1; --samp; ) {
                            Xil_float32* src_band = src_pixel;
                            Xil_float32* dst_band = dst_pixel;
                            
                            for(band=nbands+1; --band; ) {
                                *dst_band = *src_band;
                                
                                src_band += src_bs;
                                dst_band += dst_bs;
                            }
                            
                            src_pixel += src_ps;
                            dst_pixel += dst_ps;
                        }
                        
                        src_scan = (Xil_unsigned8*)(((Xil_float32*)src_scan) + src_ss);
                        dst_scan = (Xil_unsigned8*)(((Xil_float32*)dst_scan) + dst_ss);
                    }
                    break;
                    
                    //
                    // Copy extended types byte-by-byte
                    //
                  default:
                    int bytes_to_copy = nbands * unit_size;
                    for(line=0; line<dst_h; line++) {
                        Xil_unsigned8* src_pixel = src_scan;
                        Xil_unsigned8* dst_pixel = dst_scan;
                        
                        for(samp=dst_w+1; --samp; ) {
                            Xil_unsigned8* src_band = src_pixel;
                            Xil_unsigned8* dst_band = dst_pixel;
                            
                            for(band=nbands+1; --band; ) {
                                Xil_unsigned8* sptr = src_band;
                                Xil_unsigned8* dptr = dst_band;
                                
                                for(int byte=bytes_to_copy+1; --byte; ) {
                                    *dptr++ = *sptr++;
                                }
                                
                                src_band += src_bs;
                                dst_band += dst_bs;
                            }
                            
                            src_pixel += src_ps * unit_size;
                            dst_pixel += dst_ps * unit_size;
                        }
                        
                        src_scan += src_ss * unit_size;
                        dst_scan += dst_ss * unit_size;
                    }
                    break;
                }
                
            } // end loop to catch all non-optimal and non-GENERAL cases

        } // end loop to catch non-GENERAL cases
        
    } // End tile loop
    return XIL_SUCCESS;
    
} // End deCobbleStorage()
    
//
// These two routines should never be called,
// since this IS the memory storage device.
//

XilStatus
XilDeviceStorageMemory::copyFromMemory(XilStorage*  ,
                                       XilStorage*  ,
                                       unsigned int ,
                                       unsigned int )
{
    return XIL_FAILURE;
}

XilStatus
XilDeviceStorageMemory::copyToMemory(XilStorage*  ,
                                     XilStorage*  ,
                                     unsigned int ,
                                     unsigned int )
{
    return XIL_FAILURE;
}
