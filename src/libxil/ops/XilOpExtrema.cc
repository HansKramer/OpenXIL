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
//  File:	XilOpExtrema.cc
//  Project:	XIL
//  Revision:	1.12
//  Last Mod:	10:07:25, 03/10/00
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
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilOpExtrema.cc	1.12\t00/03/10  "

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#ifndef _WINDOWS
#include <values.h>
#endif

#include <xil/xilGPI.hh>
#include "XilOpDataCollect.hh"
#include "XiliOpUtils.hh"

class XilOpExtrema : public XilOpDataCollect {
public:
    static XilOp*       create(char* function_name,
                               void* args[],
                               int count);
    virtual XilStatus	vReportResults(void* results[]);
    
protected:
                        XilOpExtrema(XilOpNumber  op_num,
                                     unsigned int nbands,
                                     XilDataType  dt,
                                     float*       max,
                                     float*       min);
    virtual             ~XilOpExtrema();

private:
    //
    //  Local copy of the number of image bands and image datatype
    //
    unsigned int	nbands;
    XilDataType         dataType;

    //
    //  Used to lock update of max and min values
    //
    XilMutex		mutex;

    //
    //  Copies of the pointers the user program passed in
    //
    float*		maxUser;
    float*		minUser;
};

XilOp*
XilOpExtrema::create(char  function_name[],
                     void* args[],
                     int)
{
    static XilOpCache	extrema_op_cache;
    XilOpNumber		opnum;
    XilImage*           src = (XilImage*)args[0];

    //
    // There is no dst image, but this call expects there
    // to be one, 1.2 did it this way
    //
    if((opnum = xili_verify_op_args(function_name,
                                    &extrema_op_cache,
                                    src, src)) == -1) {
        return NULL;
    }

    //
    //  Check the parameters for NULL
    //
    if((args[1] == NULL) || (args[2] == NULL)) {
	XIL_ERROR(src->getSystemState(), XIL_ERROR_USER, "di-146", TRUE);
	return NULL;
    }

    //
    //  Now create the op and set the required pieces
    //
    XilOpExtrema* op = new XilOpExtrema(opnum, 
					src->getNumBands(),
                                        src->getDataType(),
					(float*)args[1],
					(float*)args[2]);
    if(op == NULL) {
	XIL_ERROR(src->getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
	return NULL;
    }

    op->setSrc(1, src);

    return op;
}


//
// Constructor
//
XilOpExtrema::XilOpExtrema(XilOpNumber  op_num,
                           unsigned int nb,
                           XilDataType  dt,
			   float*       max,
                           float*       min) :
    XilOpDataCollect(op_num)
{
    //
    //  Make copies of the passed in values
    //
    nbands   = nb;
    dataType = dt;
    maxUser  = max;
    minUser  = min;

    //
    //  Initialize the max, min arrays
    //  This means that first time in the first
    //  pixel will set the max, min values
    //
    unsigned int i;
    for(i=0; i<nbands; i++) {
	maxUser[i] = -XIL_MAXFLOAT;
    }

    for(i=0; i<nbands; i++) {
	minUser[i] = XIL_MAXFLOAT;
    }
}

//
// Destructor
//
XilOpExtrema::~XilOpExtrema() 
{
}

//
//  Replace the XilOp virtual function, we don't need a
//  completeResults as we are incrementally updating the
//  users buffer.
// 
//  The compute routine passes back two arrays for max and min.
//  Their type matches that of the image type.
//
XilStatus
XilOpExtrema::vReportResults(void* results[])
{
    switch(dataType) {
      case XIL_BIT:
      case XIL_UNSIGNED_4:
      case XIL_BYTE:
      {
          unsigned int   i;
          Xil_unsigned8* min_in = (Xil_unsigned8*)results[0];
          Xil_unsigned8* max_in = (Xil_unsigned8*)results[1];

          mutex.lock();

          //
          //  Max value
          //
          for(i=0; i<nbands; i++) {
              if(_XILI_B2F(max_in[i]) > maxUser[i]) {
                  maxUser[i] = _XILI_B2F(max_in[i]);
              }
          }

          //
          //  Min value
          //
          for(i=0; i<nbands; i++) {
              if(_XILI_B2F(min_in[i]) < minUser[i]) {
                  minUser[i] = _XILI_B2F(min_in[i]);
              }
          }

          mutex.unlock();

          return XIL_SUCCESS;
      }

      case XIL_SHORT:
      {
          unsigned int i;
          Xil_signed16* min_in = (Xil_signed16*)results[0];
          Xil_signed16* max_in = (Xil_signed16*)results[1];

          mutex.lock();

          //
          //  Max value
          //
          for(i=0; i<nbands; i++) {
              if(max_in[i] > maxUser[i]) {
                  maxUser[i] = max_in[i];
              }
          }

          //
          //  Min value
          //
          for(i=0; i<nbands; i++) {
              if(min_in[i] < minUser[i]) {
                  minUser[i] = min_in[i];
              }
          }

          mutex.unlock();

          return XIL_SUCCESS;
      }

      case XIL_FLOAT:
      {
          unsigned int i;
          Xil_float32* min_in = (Xil_float32*)results[0];
          Xil_float32* max_in = (Xil_float32*)results[1];

          mutex.lock();

          //
          //  Max value
          //
          for(i=0; i<nbands; i++) {
              if(max_in[i] > maxUser[i]) {
                  maxUser[i] = max_in[i];
              }
          }

          //
          //  Min value
          //
          for(i=0; i<nbands; i++) {
              if(min_in[i] < minUser[i]) {
                  minUser[i] = min_in[i];
              }
          }

          mutex.unlock();

          return XIL_SUCCESS;
      }
        
      case XIL_SIGNED_8:
      {
          unsigned int i;
          Xil_signed8* min_in = (Xil_signed8*)results[0];
          Xil_signed8* max_in = (Xil_signed8*)results[1];

          mutex.lock();

          //
          //  Max value
          //
          for(i=0; i<nbands; i++) {
              if(max_in[i] > maxUser[i]) {
                  maxUser[i] = max_in[i];
              }
          }

          //
          //  Min value
          //
          for(i=0; i<nbands; i++) {
              if(min_in[i] < minUser[i]) {
                  minUser[i] = min_in[i];
              }
          }

          mutex.unlock();

          return XIL_SUCCESS;
      }

      case XIL_UNSIGNED_16:
      {
          unsigned int    i;
          Xil_unsigned16* min_in = (Xil_unsigned16*)results[0];
          Xil_unsigned16* max_in = (Xil_unsigned16*)results[1];

          mutex.lock();

          //
          //  Max value
          //
          for(i=0; i<nbands; i++) {
              if(max_in[i] > maxUser[i]) {
                  maxUser[i] = max_in[i];
              }
          }

          //
          //  Min value
          //
          for(i=0; i<nbands; i++) {
              if(min_in[i] < minUser[i]) {
                  minUser[i] = min_in[i];
              }
          }

          mutex.unlock();

          return XIL_SUCCESS;
      }

      case XIL_SIGNED_32:
      {
          unsigned int  i;
          Xil_signed32* min_in = (Xil_signed32*)results[0];
          Xil_signed32* max_in = (Xil_signed32*)results[1];

          mutex.lock();

          //
          //  Max value
          //
          for(i=0; i<nbands; i++) {
              if(max_in[i] > maxUser[i]) {
                  maxUser[i] = (float) max_in[i];
              }
          }

          //
          //  Min value
          //
          for(i=0; i<nbands; i++) {
              if(min_in[i] < minUser[i]) {
                  minUser[i] = (float) min_in[i];
              }
          }

          mutex.unlock();

          return XIL_SUCCESS;
      }

      case XIL_UNSIGNED_32:
      {
          unsigned int    i;
          Xil_unsigned32* min_in = (Xil_unsigned32*)results[0];
          Xil_unsigned32* max_in = (Xil_unsigned32*)results[1];

          mutex.lock();

          //
          //  Max value
          //
          for(i=0; i<nbands; i++) {
              if(max_in[i] > maxUser[i]) {
                  maxUser[i] = (float) max_in[i];
              }
          }

          //
          //  Min value
          //
          for(i=0; i<nbands; i++) {
              if(min_in[i] < minUser[i]) {
                  minUser[i] = (float) min_in[i];
              }
          }

          mutex.unlock();

          return XIL_SUCCESS;
      }

      case XIL_SIGNED_64:
      {
          unsigned int  i;
          Xil_signed64* min_in = (Xil_signed64*)results[0];
          Xil_signed64* max_in = (Xil_signed64*)results[1];

          mutex.lock();

          //
          //  Max value
          //
          for(i=0; i<nbands; i++) {
              if(max_in[i] > maxUser[i]) {
                  maxUser[i] = (float) max_in[i];
              }
          }

          //
          //  Min value
          //
          for(i=0; i<nbands; i++) {
              if(min_in[i] < minUser[i]) {
                  minUser[i] = (float) min_in[i];
              }
          }

          mutex.unlock();

          return XIL_SUCCESS;
      }

      case XIL_UNSIGNED_64:
      {
          unsigned int    i;
          Xil_unsigned64* min_in = (Xil_unsigned64*)results[0];
          Xil_unsigned64* max_in = (Xil_unsigned64*)results[1];

          mutex.lock();

          //
          //  Max value
          //
          for(i=0; i<nbands; i++) {
              if((Xil_signed64) max_in[i] > maxUser[i]) {
                  maxUser[i] = (float) ((Xil_signed64) max_in[i]);
              }
          }

          //
          //  Min value
          //
          for(i=0; i<nbands; i++) {
              if((Xil_signed64)min_in[i] < minUser[i]) {
                  minUser[i] = (float) ((Xil_signed64) min_in[i]);
              }
          }

          mutex.unlock();

          return XIL_SUCCESS;
      }

      case XIL_FLOAT_64:
      case XIL_FLOAT_128:
      case XIL_COMPLEX_FLOAT_32:
      case XIL_COMPLEX_FLOAT_64:
      case XIL_COMPLEX_MAG_FLOAT_32:
      case XIL_COMPLEX_MAG_FLOAT_64:
        return XIL_FAILURE;

      default:
        return XIL_FAILURE;
    }
}


