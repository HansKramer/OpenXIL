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
//  File:       Reconstructor.cc
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:15:01, 03/10/00
//
//  Description:
//
//    Implementation of the Reconstructor object, used in compression
//    to generate the prediction errors, which are then Huffman-encoded.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Reconstructor.cc	1.5\t00/03/10  "

#include "xil/xilGPI.hh"
#include "Reconstructor.hh"
#include "XiliUtils.hh"
  
//
// Constructor / Destructor
//
Reconstructor::Reconstructor(JpegLLScanInfo* si)
{
    isOKFlag = FALSE;

    precision      = si->precision;
    width          = si->width;
    nbands         = si->nbands;
    base           = si->dataptr;

    bands_per_scan = si->nbands;

    dst_ps         = si->ps;
    dst_ss         = si->ss;

    //
    // Allocate a one line buffer to hold the prediction errors
    //
    diffBuf = new Xil_signed16[width * bands_per_scan];
    if(diffBuf == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }

    //
    // Fail if point transform is greater than precision
    //
    pt_transform = si->transform;
    if(pt_transform >= si->precision) {
        XIL_ERROR(NULL, XIL_ERROR_USER, "di-71", TRUE);
        return;
    }
    
    prediction_type = si->predictor;

    first_pixel = 1 << (precision - pt_transform - 1);

    isOKFlag = TRUE;
}

Reconstructor::~Reconstructor()
{
    delete [] diffBuf;
}

Xil_boolean
Reconstructor::isOK()
{
    if(this == NULL) {
        return FALSE;
    } else {
        if(isOKFlag == TRUE) {
            return TRUE;
        } else {
            delete this;
            return FALSE;
        }
    }
}

//------------------------------------------------------------------------
//
//  Function:        Reconstructor::reconstruct8
//  Created:        
//
//  Description:
//    This function reconstructs one scanline
//    of 8 bit data. It uses the specified predictor, except on the 
//    first line and RSTs. In that case it uses the ONE_D1 predictor.
//        
//------------------------------------------------------------------------

void Reconstructor::reconstruct8(Xil_unsigned8* dst,
                                 Xil_boolean    doRestart)
{
    //
    // Process one band at a time
    //
    for(int band=0; band<bands_per_scan; band++) {
        int a = -dst_ps;
        int b = -dst_ss;
        int c = -(dst_ss + dst_ps);

        int pred;
        int predictor = prediction_type;;
        if(doRestart) {
            //
            // Always use the ONE_D1 predictor on first line and RSTs
            // Also, use the half range value as the first pixel predictor
            predictor = ONE_D1;
            pred = first_pixel;
        } else {
            //
            // For first pixel of line, use pixel above as predictor
            //
            pred = dst[band + b];
        }

        //
        // Do the first pixel in the line
        //
        dst[band] = diffBuf[band] + pred;

        //
        // Init the src and dst ptrs to the second pixel on the line
        //
        Xil_unsigned8* pDst = dst + dst_ps + band;
        Xil_signed16*  pDiff = diffBuf + bands_per_scan + band;

        int w;
        switch(predictor) {
          case ONE_D1:
            for(w=width; --w; ) {
                pred = pDst[a];
                *pDst = pred + *pDiff;
                pDst += dst_ps;
                pDiff += bands_per_scan;
            }
            break;
          case ONE_D2:
            for(w=width; --w; ) {
                pred = pDst[b];
                *pDst = pred + *pDiff;
                pDst += dst_ps;
                pDiff += bands_per_scan;
            }
            break;
          case ONE_D3:
            for(w=width; --w; ) {
                pred = pDst[c];
                *pDst = pred + *pDiff;
                pDst += dst_ps;
                pDiff += bands_per_scan;
            }
            break;
          case TWO_D1:
            for(w=width; --w; ) {
                pred = (int)pDst[a] + (int)pDst[b] - (int)pDst[c];
                *pDst = pred + *pDiff;
                pDst += dst_ps;
                pDiff += bands_per_scan;
            }
            break;
          case TWO_D2:
            for(w=width; --w; ) {
                pred = (int)pDst[a] + (((int)pDst[b] - (int)pDst[c]) >> 1);
                *pDst = pred + *pDiff;
                pDst += dst_ps;
                pDiff += bands_per_scan;
            }
            break;
          case TWO_D3:
            for(w=width; --w; ) {
                pred = (int)pDst[b] + (((int)pDst[a] - (int)pDst[c]) >> 1);
                *pDst = pred + *pDiff;
                pDst += dst_ps;
                pDiff += bands_per_scan;
            }
            break;
          case TWO_D4:
            for(w=width; --w; ) {
                pred = ((int)pDst[a] + (int)pDst[b]) >> 1;
                *pDst = pred + *pDiff;
                pDst += dst_ps;
                pDiff += bands_per_scan;
            }
            break;
          default:
            break;

        } // End switch

    } // End band loop
}

//------------------------------------------------------------------------
//
//  Function:        Reconstructor::reconstruct16
//  Created:        
//
//  Description:
//    This function reconstructs one scanline
//    of 8 bit data. It uses the specified predictor, except on the 
//    first line and RSTs. In that case it uses the ONE_D1 predictor.
//        
//------------------------------------------------------------------------

void Reconstructor::reconstruct16(Xil_signed16* dst,
                                 Xil_boolean    doRestart)
{
    //
    // Process one band at a time
    //
    for(int band=0; band<bands_per_scan; band++) {
        int a = -dst_ps;
        int b = -dst_ss;
        int c = -(dst_ss + dst_ps);

        int pred;
        int predictor = prediction_type;;
        if(doRestart) {
            //
            // Always use the ONE_D1 predictor on first line and RSTs
            // Also, use the half range value as the first pixel predictor
            predictor = ONE_D1;
            pred = first_pixel;
        } else {
            pred = dst[b + band];
        }

        //
        // Do the first pixel in the line
        //
        dst[band] = diffBuf[band] + pred;

        //
        // Init the ptrs to the second pixel on the line
        //
        Xil_signed16* pDst  = dst + dst_ps + band;
        Xil_signed16* pDiff = diffBuf + bands_per_scan + band;

        int w;
        switch(predictor) {
          case ONE_D1:
            for(w=width; --w; ) {
                pred = pDst[a];
                *pDst = pred + *pDiff;
                pDst += dst_ps;
                pDiff += bands_per_scan;
            }
            break;
          case ONE_D2:
            for(w=width; --w; ) {
                pred = pDst[b];
                *pDst = pred + *pDiff;
                pDst += dst_ps;
                pDiff += bands_per_scan;
            }
            break;
          case ONE_D3:
            for(w=width; --w; ) {
                pred = pDst[c];
                *pDst = pred + *pDiff;
                pDst += dst_ps;
                pDiff += bands_per_scan;
            }
            break;
          case TWO_D1:
            for(w=width; --w; ) {
                pred = pDst[a] + pDst[b] - pDst[c];
                *pDst = pred + *pDiff;
                pDst += dst_ps;
                pDiff += bands_per_scan;
            }
            break;
          case TWO_D2:
            for(w=width; --w; ) {
                pred = pDst[a] + ((pDst[b] - pDst[c]) >> 1);
                *pDst = pred + *pDiff;
                pDst += dst_ps;
                pDiff += bands_per_scan;
            }
            break;
          case TWO_D3:
            for(w=width; --w; ) {
                pred = pDst[b] + ((pDst[a] - pDst[c]) >> 1);
                *pDst = pred + *pDiff;
                pDst += dst_ps;
                pDiff += bands_per_scan;
            }
            break;
          case TWO_D4:
            for(w=width; --w; ) {
                pred = (pDst[a] + pDst[b]) >> 1;
                *pDst = pred + *pDiff;
                pDst += dst_ps;
                pDiff += bands_per_scan;
            }
            break;
          default:
            break;

        } // End switch

    } // End band loop
}
