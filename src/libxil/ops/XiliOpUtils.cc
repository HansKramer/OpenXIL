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
//  File:	XiliOpUtils.cc
//  Project:	XIL
//  Revision:	1.22
//  Last Mod:	10:07:20, 03/10/00
//
//  Description: Utilities for use by the ops, verify op args
//	         check number of bands, datatypes if operation
//		 is valid for a given data type
//	
//	
//	
//	
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliOpUtils.cc	1.22\t00/03/10  "

//
//  System Includes
//
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <xil/xilGPI.hh>
#include "XiliUtils.hh"
#include "XiliOpUtils.hh"

//--------------------------------------------------------------
//
//  XilOp UTILITY Routines
//
//--------------------------------------------------------------

inline
int
same_datatypes(XilImage* im1, XilImage* im2)
{
    return (im1->getDataType() == im2->getDataType());
}

inline
int
same_bands(XilImage* im1, XilImage* im2)
{
    return (im1->getNumBands() == im2->getNumBands());
}

inline
int
same_size(XilImage* im1, XilImage* im2)
{
    unsigned int x1, y1, x2, y2;
    im1->getSize(&x1, &y1);
    im2->getSize(&x2, &y2);
    return (x1==x2 && y1==y2);
}


//
// Check to see if the op has been cached. If not
// cache it. This utility works for most of the operations
// which don't change datatypes.
//
XilOpNumber
xili_check_op_cache(char op_name[], XiliOpCache* op_cache, XilImage* dst, Xil_boolean generate_error)
{
    // Check if op number is cached.
    //
    //  Since the op_cache is static, we need a static mutex to ensure
    //  that it's not being updated and read at the same time.
    //
    XilDataType op_datatype = dst->getDataType();
    int         op_number   = op_cache->lookup(op_datatype);
    if(op_number < 0) {
        //
        //  Create a temporary buffer to sprintf the name into.  It's the
        //  length of the base + the max size of a datatype string + 1 for
        //  the intermediate characters and 1 for the NULL character at the
        //  end.
        //
        char* tmpbuf = new char[strlen(op_name) +
                               2 * xili_maxlen_of_datatype_string() + 2];

        sprintf(tmpbuf, "%s;%s", op_name,
                xili_datatype_to_string(op_datatype));
        
        //
        //  Lookup the compute op number and store the result in the
        //    given cache...
        //
        XilGlobalState* xgs = XilGlobalState::getXilGlobalState();
        if((op_number = op_cache->set(op_datatype,
                                      xgs->lookupOpNumber(tmpbuf))) < 0) {
            if( generate_error == TRUE ) {
                XIL_ERROR(dst->getSystemState(),
                          XIL_ERROR_CONFIGURATION,"di-5",TRUE);
            }
        }

        delete tmpbuf;
    }

    return op_number;
}

//
// Special version of op_cache for cast as it requires a mapping 
// from src to dst datatype
//
XilOpNumber
xili_check_op_cache_cast(char op_name[], 
                         XiliOpCache* op_cache, 
                         XilImage* dst,
                         XilImage* src)
{
    // Check if op number is cached.
    //
    //  Since the op_cache is static, we need a static mutex to ensure
    //  that it's not being updated and read at the same time.
    //
    XilDataType src_datatype = src->getDataType();
    XilDataType dst_datatype = dst->getDataType();
    int         op_number    = op_cache->lookup(src_datatype, dst_datatype);
    if(op_number < 0) {
        //
        //  Create a temporary buffer to sprintf the name into.  It's the
        //  length of the base + the max size of a datatype string + 1 for
        //  the intermediate characters and 1 for the NULL character at the
        //  end.
        //
        char* tmpbuf = new char[strlen(op_name) +
                               2 * xili_maxlen_of_datatype_string() + 4];

        sprintf(tmpbuf, "%s;%s->%s", op_name,
                xili_datatype_to_string(src_datatype),
                xili_datatype_to_string(dst_datatype));
        
        //
        //  Lookup the compute op number and store the result in the
        //    given cache...
        //
        XilGlobalState* xgs = XilGlobalState::getXilGlobalState();
        if((op_number = op_cache->set(src_datatype, dst_datatype,
                                      xgs->lookupOpNumber(tmpbuf))) < 0) {
            XIL_ERROR(dst->getSystemState(),
                      XIL_ERROR_CONFIGURATION,"di-5",TRUE);
        }

        delete tmpbuf;
    }

    return op_number;
}

//
// Special version of op_cache for tablewarp as it requires a mapping 
// using a warp table image which could potentially vary in data type.
// The src and dst images have the same data type.
//
XilOpNumber
xili_check_op_cache_tablewarp(char op_name[], 
                              XiliOpCache* op_cache, 
                              XilImage* dst,
                              XilImage* warp)
{
    // Check if op number is cached.
    //
    //  Since the op_cache is static, we need a static mutex to ensure
    //  that it's not being updated and read at the same time.
    //
    XilDataType warp_datatype = warp->getDataType();
    XilDataType op_datatype = dst->getDataType();
    int         op_number   = op_cache->lookup(op_datatype, warp_datatype);
    if(op_number < 0) {
        //
        //  Create a temporary buffer to sprintf the name into.  It's the
        //  length of the base + 3 * (the max size of a datatype string) + 4
        //  for the intermediate characters and 1 for the NULL character at
        //  the end.
        //
        char* tmpbuf = new char[strlen(op_name) +
                                3 * xili_maxlen_of_datatype_string() + 5];

        sprintf(tmpbuf, "%s;%s,%s->%s", op_name,
                xili_datatype_to_string(op_datatype),
                xili_datatype_to_string(warp_datatype),
                xili_datatype_to_string(op_datatype));
        
        //
        //  Lookup the compute op number and store the result in the
        //    given cache...
        //
        XilGlobalState* xgs = XilGlobalState::getXilGlobalState();
        if((op_number = op_cache->set(op_datatype, warp_datatype,
                                      xgs->lookupOpNumber(tmpbuf))) < 0) {
            XIL_ERROR(dst->getSystemState(),
                      XIL_ERROR_CONFIGURATION,"di-5",TRUE);
        }

        delete tmpbuf;
    }

    return op_number;
}

//------------------------------------------------------------------------
//
//  Function:        XilVerifyOpArgs
//
//  Description:
//        
//  Used to verify the common attributes of images are correct and
//  lookup the op number and place it into a given cache.
//
//  The cache only makes sense to keep in a static array.  So, the
//  user is required to pass in a mutex which is used to guarentee the
//  cache is locked correctly for multiple threads.
//
//  MT-level:  Safe
//
//  Returns:
//      op number or -1 on failure
//        
//  Side Effects:
//        fills in given op cache with op number
//        
//------------------------------------------------------------------------
XilOpNumber
xili_verify_op_args(char         op_name[],
                    XiliOpCache* op_cache,
                    XilImage*    dst,
                    XilImage*    src1,
                    XilImage*    src2,
                    XilImage*    src3,
                    Xil_boolean  generate_error)
{
    XilSystemState*  state;

    //  check for NULL dst
    if(dst == NULL) {
        if(src1) {
            state = src1->getSystemState();
        } else if(src2) {
            state = src2->getSystemState();
        } else if(src3) {
            state = src3->getSystemState();
        } else {
            state = NULL;
        }
        XIL_ERROR(state, XIL_ERROR_USER, "di-207", TRUE);

        return -1;
    }

    //
    //  Check for invalid images being used as sources.
    //
    if((src1) && !src1->isValid()) {
        XIL_OBJ_ERROR(src1->getSystemState(), XIL_ERROR_USER, "di-327", TRUE, src1);
        return -1;
    }
    if((src2) && !src2->isValid()) {
        XIL_OBJ_ERROR(src2->getSystemState(), XIL_ERROR_USER, "di-327", TRUE, src2);
        return -1;
    }
    if((src3) && !src3->isValid()) {
        XIL_OBJ_ERROR(src3->getSystemState(), XIL_ERROR_USER, "di-327", TRUE, src3);
        return -1;
    }

    // check datatype/bands of all existing sources against destination's.
    if((src1) && (!same_datatypes(dst, src1))) {
        // invalid source image 1
        XIL_OBJ_ERROR(src1->getSystemState(), XIL_ERROR_USER, "di-434", TRUE, src1);
        return -1;
    }
    if((src2) && (!same_datatypes(dst, src2))) {
        // invalid source image 2
        XIL_OBJ_ERROR(src2->getSystemState(), XIL_ERROR_USER, "di-435", TRUE, src2);
        return -1;
    }
    if((src3) && (!same_datatypes(dst, src3))) {
        // invalid source image 3
        XIL_OBJ_ERROR(src3->getSystemState(), XIL_ERROR_USER, "di-436", TRUE, src3);
        return -1;
    }

    // check datatype/bands of all existing sources against destination's.
    if((src1) && (!same_bands(dst, src1))) {
        // invalid source image 1
        XIL_OBJ_ERROR(src1->getSystemState(), XIL_ERROR_USER, "di-2", TRUE, src1);
        return -1;
    }
    if((src2) && (!same_bands(dst, src2))) {
        // invalid source image 2
        XIL_OBJ_ERROR(src2->getSystemState(), XIL_ERROR_USER, "di-3", TRUE, src2);
        return -1;
    }
    if((src3) && (!same_bands(dst, src3))) {
        // invalid source image 3
        XIL_OBJ_ERROR(src3->getSystemState(), XIL_ERROR_USER, "di-4", TRUE, src3);
        return -1;
    }

    return xili_check_op_cache(op_name, op_cache, dst, generate_error);
}

//
// Utility code to take the base function name and return
// a name with one of the types of geometric interpolations
// addded.
//
Xil_boolean
xili_get_geometric_function_name(XilSystemState*       state,
                                 const char*           function_name,
                                 XiliInterpolationType interp, 
                                 char*                 func_name)
{
    switch(interp) {
      case XiliNearest:
        sprintf(func_name, "%s_nearest", function_name);
        break;
      case XiliBilinear:
        sprintf(func_name, "%s_bilinear", function_name);
        break;
      case XiliBicubic:
        sprintf(func_name, "%s_bicubic", function_name);
        break;
      case XiliGeneral:
        sprintf(func_name, "%s_general", function_name);
        break;
      default:
        XIL_ERROR(state, XIL_ERROR_USER, "di-416", TRUE);
        return FALSE;
    }

    return TRUE;
}


// Check to see if the datatype for logicals is
// valid.
Xil_boolean
xili_verify_op_logicals(XilImage* image)
{
    switch(image->getDataType()) {
      // The following are valid for logical operations
      case XIL_BIT:
      case XIL_BYTE:
      case XIL_SHORT:
      case XIL_UNSIGNED_4:
      case XIL_SIGNED_8:
      case XIL_UNSIGNED_16:
      case XIL_SIGNED_32:
      case XIL_UNSIGNED_32:
      case XIL_SIGNED_64:
      case XIL_UNSIGNED_64:
        return TRUE;

      // The following are invalid for logical operations
      case XIL_FLOAT:
      case XIL_FLOAT_64:
      case XIL_FLOAT_128:
      case XIL_COMPLEX_FLOAT_32:
      case XIL_COMPLEX_FLOAT_64:
      case XIL_COMPLEX_MAG_FLOAT_32:
      case XIL_COMPLEX_MAG_FLOAT_64:
        return FALSE;

      default:
        return FALSE;
    }
}

//
// Verify that the warp image is the correct bands and
// is short or float
//
XilOpNumber
xili_verify_op_tablewarp(char          op_name[],
                         XiliOpCache*  op_cache,
                         XilImage*     dst,
                         XilImage*     src1,
                         XilImage*     warp,
                         unsigned int  warp_bands)
{
    XilSystemState*  state;

    //  check for NULL dst
    if(dst == NULL) {
        if(src1) {
            state = src1->getSystemState();
        } else if(warp) {
            state = warp->getSystemState();
        } else {
            state = NULL;
        }
        XIL_ERROR(state, XIL_ERROR_USER, "di-207", TRUE);
        return -1;
    }

    // check for invalid images being used
    if((src1) && !src1->isValid()) {
        XIL_OBJ_ERROR(src1->getSystemState(), XIL_ERROR_USER, "di-327", TRUE, src1);
        return -1;
    }

    if((warp) && !warp->isValid()) {
        XIL_OBJ_ERROR(warp->getSystemState(), XIL_ERROR_USER, "di-327", TRUE, warp);
        return -1;
    }

    // check datatype/bands of all existing sources against destination's.
    if((src1) && (!same_datatypes(dst, src1))) {
        // invalid source image 1
        XIL_OBJ_ERROR(src1->getSystemState(), XIL_ERROR_USER, "di-434", TRUE, src1);
        return -1;
    }

    // check datatype/bands of all existing sources against destination's.
    if((src1) && (!same_bands(dst, src1))) {
        // invalid source image 1
        XIL_OBJ_ERROR(src1->getSystemState(), XIL_ERROR_USER, "di-2", TRUE, src1);
        return -1;
    }

    // Check that the warp image has the required number of bands
    if(warp->getNumBands() != warp_bands) {
        XIL_OBJ_ERROR(warp->getSystemState(), XIL_ERROR_USER, "di-3", TRUE, warp);
        return -1;
    }
    if((warp->getDataType() != XIL_SHORT) &&
       (warp->getDataType() != XIL_FLOAT)) {
        XIL_OBJ_ERROR(warp->getSystemState(), XIL_ERROR_USER, "di-435", TRUE, warp);
        return -1;
    }
    
    return xili_check_op_cache_tablewarp(op_name, op_cache, dst, warp);
}

//
// Utility code to copy an API float array into the
// the correct type for the image.
//
void*
xili_round_op_values(XilDataType  dtype,
                     float*       array,
                     unsigned int nbands)
{
    switch(dtype) {
      // TESTED TYPES
      case XIL_BIT: {
          Xil_unsigned8*    rarray;
          rarray = new Xil_unsigned8[nbands];

          if(rarray == NULL)
              return NULL;

          for(unsigned int i=0; i<nbands; i++) {
              if(array[i] < 0.5) {
                  rarray[i] = 0;
              } else {
                  rarray[i] = 1;
              }
          }
          return rarray;
      }
      case XIL_BYTE: {
          Xil_unsigned8*   rarray;
          
          rarray = new Xil_unsigned8[nbands];
          if(rarray == NULL)
              return NULL;

          for(unsigned int i=0; i<nbands; i++) {
              rarray[i] = _XILI_ROUND_U8(array[i]);
          }
          return rarray;
      }
      case XIL_SHORT: {
          Xil_signed16*    rarray;

          rarray = new Xil_signed16[nbands];
          if(rarray == NULL)
              return NULL;
          
          for(unsigned int i=0; i<nbands; i++) {
              rarray[i] = _XILI_ROUND_S16(array[i]);
          }
          return rarray;
      }
      case XIL_FLOAT: {
          Xil_float32*    rarray;
          
          rarray = new Xil_float32[nbands];
          if(rarray == NULL)
              return NULL;
          
          for(unsigned int i = 0; i<nbands; i++) {
              rarray[i] = array[i];
          }
          return rarray;
      }
      // UNTESTED TYPES
      case XIL_UNSIGNED_4: {
          Xil_unsigned8    *rarray;
          
          rarray = new Xil_unsigned8[nbands];
          if(rarray == NULL)
              return NULL;

          for(unsigned int i=0; i<nbands; i++) {
              rarray[i] = _XILI_ROUND_U4(array[i]);
          }
          return rarray;
      }
      case XIL_SIGNED_8: {
          Xil_signed8*    rarray;
          
          rarray = new Xil_signed8[nbands];
          if(rarray == NULL)
              return NULL;

          for(unsigned int i=0; i<nbands; i++) {
              rarray[i] = _XILI_ROUND_S8(array[i]);
          }
          return rarray;
      }
      case XIL_UNSIGNED_16: {
          Xil_unsigned16*   rarray;

          rarray = new Xil_unsigned16[nbands];
          if(rarray == NULL)
              return NULL;
          
          for(unsigned int i=0; i<nbands; i++) {
              rarray[i] = _XILI_ROUND_U16(array[i]);
          }
          return rarray;
      }
      case XIL_SIGNED_32: {
          Xil_signed32*   rarray;

          rarray = new Xil_signed32[nbands];
          if(rarray == NULL)
              return NULL;
          
          for(unsigned int i=0; i<nbands; i++) {
              rarray[i] = _XILI_ROUND_S32(array[i]);
          }
          return rarray;
      }
      case XIL_UNSIGNED_32: {
          Xil_unsigned32*   rarray;

          rarray = new Xil_unsigned32[nbands];
          if(rarray == NULL)
              return NULL;
          
          for(unsigned int i=0; i<nbands; i++) {
              rarray[i] = _XILI_ROUND_U32(array[i]);
          }
          return rarray;
      }
      case XIL_FLOAT_64: {
          Xil_float64*    rarray;
          
          rarray = new Xil_float64[nbands];
          if(rarray == NULL)
              return NULL;
          
          for(unsigned int i=0; i<nbands; i++) {
              rarray[i] = array[i];
          }
          return rarray;
      }
      case XIL_FLOAT_128: {
          Xil_float128*    rarray;
          
          rarray = new Xil_float128[nbands];
          if(rarray == NULL)
              return NULL;
          
          for(unsigned int i=0; i<nbands; i++) {
              rarray[i] = array[i];
          }
          return rarray;
      }
      case XIL_COMPLEX_FLOAT_32: {
          XilComplexFloat32*    rarray;
          int                   index;
          
          rarray = new XilComplexFloat32[nbands];
          if(rarray == NULL)
              return NULL;

          index = 0;
          for(unsigned int i=0; i<(nbands*2); i+=2) {
              rarray[index].real = array[i];
              rarray[index].img = array[i+1];
              index++;
          }
          return rarray;
      }
      case XIL_COMPLEX_FLOAT_64: {
          XilComplexFloat64*    rarray;
          int                   index;
          
          rarray = new XilComplexFloat64[nbands];
          if(rarray == NULL)
              return NULL;

          index = 0;
          for(unsigned int i=0; i<(nbands*2); i+=2) {
              rarray[index].real = array[i];
              rarray[index].img = array[i+1];
              index++;
          }
          return rarray;
      }
      case XIL_COMPLEX_MAG_FLOAT_32: {
          XilComplexMagFloat32*   rarray;
          int                     index;
          
          rarray = new XilComplexMagFloat32[nbands];
          if(rarray == NULL)
              return NULL;

          index = 0;
          for(unsigned int i=0; i<(nbands*2); i+=2) {
              rarray[index].mag = array[i];
              rarray[index].phase = array[i+1];
              index++;
          }
          return rarray;
      }
      case XIL_COMPLEX_MAG_FLOAT_64: {
          XilComplexMagFloat64* rarray;
          int                   index;
          
          rarray = new XilComplexMagFloat64[nbands];
          if(rarray == NULL)
              return NULL;

          index = 0;
          for(unsigned int i=0; i<(nbands*2); i+=2) {
              rarray[index].mag = array[i];
              rarray[index].phase = array[i+1];
              index++;
          }
          return rarray;
      }
    }
    return NULL;
}

//
// Clamping for logicals
//
void*
xili_clamp_op_logical(XilDataType   type,
                      unsigned int* values,
                      unsigned int  nbands)
{
    switch(type) {
      // Tested cases
      case XIL_BIT: {
          Xil_unsigned8* rarray;
          
          rarray = new Xil_unsigned8[nbands];
          if(rarray == NULL)
              return NULL;
        
          for(unsigned int i=0; i<nbands; i++) {
              if(values[i] >= 1) {
                  rarray[i] = 1;
              } else {
                  rarray[i] = 0;
              }
          }
          return rarray;
      }
      case XIL_BYTE: {
          Xil_unsigned8* rarray;
        
          rarray = new Xil_unsigned8[nbands];
          if(rarray == NULL)
              return NULL;
        
          for(unsigned int i=0; i<nbands; i++) {
              if(values[i] > XIL_MAXBYTE) {
                  rarray[i] = XIL_MAXBYTE;
              } else {
                  rarray[i] = values[i];
              }
          }
          return rarray;
      }
      case XIL_SHORT: {
          Xil_unsigned16* rarray;
        
          rarray = new Xil_unsigned16[nbands];
          if(rarray == NULL)
              return NULL;
        
          for(unsigned int i=0; i<nbands; i++) {
              if(values[i] > XIL_MAXSHORT) {
                  rarray[i] = XIL_MAXSHORT;
              } else {
                  rarray[i] = values[i];
              }
          }
          return rarray;
      }

      // Untested cases
      case XIL_UNSIGNED_4: {
          Xil_unsigned8* rarray;
          
          rarray = new Xil_unsigned8[nbands];
          if(rarray == NULL)
              return NULL;
        
          for(unsigned int i=0; i<nbands; i++) {
              if(values[i] > 15) {
                  rarray[i] = 15;
              } else {
                  rarray[i] = values[i];
              }
          }
          return rarray;
      }
      case XIL_SIGNED_8: {
          Xil_signed8* rarray;
        
          rarray = new Xil_signed8[nbands];
          if(rarray == NULL)
              return NULL;
        
          for(unsigned int i=0; i<nbands; i++) {
              if(values[i] > 127) {
                  rarray[i] = 127;
              } else {
                  rarray[i] = values[i];
              }
          }
          return rarray;
      }
      case XIL_UNSIGNED_16: {
          Xil_unsigned16* rarray;
        
          rarray = new Xil_unsigned16[nbands];
          if(rarray == NULL)
              return NULL;
        
          for(unsigned int i=0; i<nbands; i++) {
              if(values[i] > 65535) {
                  rarray[i] = 65535;
              } else {
                  rarray[i] = values[i];
              }
          }
          return rarray;
      }
      case XIL_UNSIGNED_32: {
          Xil_unsigned32* rarray;
          
          rarray = new Xil_unsigned32[nbands];
          if(rarray == NULL)
              return NULL;
        
          for(unsigned int i=0; i<nbands; i++) {
              rarray[i] = values[i];
          }
          return rarray;
      }
      case XIL_SIGNED_32: {
          Xil_signed32* rarray;
          
          rarray = new Xil_signed32[nbands];
          if(rarray == NULL)
              return NULL;
        
          for(unsigned int i=0; i<nbands; i++) {
              if(values[i] > 0x7fffffff) {
                  rarray[i] = 0x7fffffff;
              } else {
                  rarray[i] = values[i];
              }
          }
          return rarray;
      }
    }
    return NULL;
}
