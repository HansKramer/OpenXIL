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
//  File:       Predictor.cc
//  Project:    XIL
//  Revision:   1.8
//  Last Mod:   10:15:00, 03/10/00
//
//  Description:
//
//    Implementation of the Predictor object, used in compression
//    to generate the prediction errors, which are then Huffman-encoded.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Predictor.cc	1.8\t00/03/10  "

#include "xil/xilGPI.hh"
#include "Predictor.hh"
#include "XiliUtils.hh"
#include "CompressInfo.hh"
  
//
// Constructor / Destructor
//
Predictor::Predictor(CompressInfo* ci, 
                     int           point_transform, 
                     int           pred_type, 
                     Xil_boolean   isInterleaved)
{
    isOKFlag = FALSE;

    datatype       = ci->image_datatype;
    precision      = xili_sizeof(datatype) * 8;
    width          = ci->image_box_width;
    nbands         = ci->image_nbands;
    base           = ci->image_dataptr;
    src_ps         = ci->image_ps;
    src_ss         = ci->image_ss;
    ptBuf          = NULL;
    diffBuf        = NULL;

    if(isInterleaved)  {
        bands_per_scan = nbands;
    } else {
        bands_per_scan = 1;
    }

    dst_ps         = bands_per_scan;
    dst_ss         = dst_ps * width;

    //
    // Allocate a one line buffer to hold the prediction errors
    //
    diffBuf = new Xil_signed16[width * bands_per_scan];
    if(diffBuf == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }

    //
    // Limit point transform shift to one less than precision
    //
    pt_transform = point_transform;
    if(pt_transform > (precision-1)) {
        XIL_ERROR(NULL, XIL_ERROR_USER, "di-71", TRUE);
        pt_transform = (precision-1);        // max
    }
    
    prediction_type = pred_type;

    first_pixel = 1 << (precision - pt_transform - 1);

    if(pt_transform != 0) {
        //
        // Allocate tmp buffer for the point transform
        // Make it two lines - one to hold current line
        // and one to retain the previous line.
        // Allocate and size it for shorts to get the
        // proper alignment. We'll cast it to byte later, as needed.
        //
        ptBuf = new Xil_signed16[2 * width * nbands];
        if(ptBuf == NULL) {
            XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
            return;
        }
        saveBuf[0] = ptBuf;
        saveBuf[1] = ptBuf + width * nbands;

        //
        // Make the saveBuf the source if the pt_transform is active
        //
        src_ps = bands_per_scan;
        src_ss = src_ps * width;
    }

    isOKFlag = TRUE;
}

Predictor::~Predictor()
{
    delete [] ptBuf;
    delete [] diffBuf;
}

Xil_boolean
Predictor::isOK()
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
//  Function:        Predictor::predict8
//  Created:        
//
//  Description:
//    This function calculates the prediction error for one scanline
//    of 8 bit data. It uses the specified predictor, except on the 
//    first line and RSTs. In that case it uses the ONE_D1 predictor.
//        
//------------------------------------------------------------------------

void Predictor::predict8(Xil_unsigned8* src,
                         Xil_boolean    doRestart)
{
    //
    // If the point transform is required, do it into the
    // saved buffer, so we don't perform it multiple times.
    // Here we're using two buffers and we toggle their usage.
    // After each line is processed, the currLine becomes the prevLine.
    //
    Xil_unsigned8* prevLine;
    Xil_unsigned8* currLine;
    if(pt_transform != 0) {
        if(doRestart) {
            // Reset counter at image start and RST sections
            rowCounter = 0;
        }

        //
        // Set the temporary buffer as the source
        // for the prediction op
        //
        prevLine = (Xil_unsigned8*)saveBuf[rowCounter];
        currLine = (Xil_unsigned8*)saveBuf[1 - rowCounter];

        //
        // Toggle rowCounter between 0 and 1 to alternate saved buffer usage
        //
        rowCounter ^= 1;

        //
        // Perform the point transform on all bands
        //
        for(int bb=0; bb<bands_per_scan; bb++) {
            Xil_unsigned8* pSrc = src + bb;
            Xil_unsigned8* pDst = currLine + bb;
            for(int i=0; i<width; i++) {
                *pDst = *pSrc >> pt_transform;
                pSrc += src_ps;
                pDst += dst_ps;
            }
        }

    } else {
        //
        // If no point transform, then set the image as the source
        //
        currLine = src;
        prevLine = src - src_ss; 
    }    


    //
    // Process one band at a time
    //
    for(int b=0; b<bands_per_scan; b++) {
        int a = -src_ps;
        int c = -src_ps;

        int pred;
        int predictor = prediction_type;;
        if(doRestart) {
            //
            // Always use the ONE_D1 predictor on first line and RSTs
            // Also, use the half range value as the first pixel predictor
            predictor = ONE_D1;
            pred = first_pixel;
        } else {
            pred = prevLine[b];
        }

        //
        // Do the first pixel in the line
        //
        diffBuf[b] = currLine[b] - pred;

        //
        // Init the src and dst ptrs to the second pixel on the line
        //
        Xil_unsigned8* pCurr = currLine + src_ps + b;
        Xil_unsigned8* pPrev = prevLine + src_ps + b;
        Xil_signed16*  pDiff = diffBuf + dst_ps + b;

        int w;
        switch(predictor) {
          case ONE_D1:
            for(w=width; --w; ) {
                int X = pCurr[0];
                int Ra = pCurr[a];
                pred = Ra;
                *pDiff = X - pred;
                pCurr += src_ps;
                pDiff += dst_ps;
            }
            break;
          case ONE_D2:
            for(w=width; --w; ) {
                int X = pCurr[0];
                int Rb = pPrev[0];
                pred = Rb;
                *pDiff = X - pred;
                pCurr += src_ps;
                pPrev += src_ps;
                pDiff += dst_ps;
            }
            break;
          case ONE_D3:
            for(w=width; --w; ) {
                int X = pCurr[0];
                int Rc = pPrev[c];
                pred = Rc;
                *pDiff = X - pred;
                pCurr += src_ps;
                pPrev += src_ps;
                pDiff += dst_ps;
            }
            break;
          case TWO_D1:
            for(w=width; --w; ) {
                int X = pCurr[0];
                int Ra = pCurr[a];
                int Rb = pPrev[0];
                int Rc = pPrev[c];
                pred = Ra + Rb - Rc;
                *pDiff = X - pred;
                pCurr += src_ps;
                pPrev += src_ps;
                pDiff += dst_ps;
            }
            break;
          case TWO_D2:
            for(w=width; --w; ) {
                int X = pCurr[0];
                int Ra = pCurr[a];
                int Rb = pPrev[0];
                int Rc = pPrev[c];
                pred = Ra + ((Rb - Rc)>>1);
                *pDiff = X - pred;
                pCurr += src_ps;
                pPrev += src_ps;
                pDiff += dst_ps;
            }
            break;
          case TWO_D3:
            for(w=width; --w; ) {
                int X = pCurr[0];
                int Ra = pCurr[a];
                int Rb = pPrev[0];
                int Rc = pPrev[c];
                pred = Rb + ((Ra - Rc) >> 1);
                *pDiff = X - pred;
                pCurr += src_ps;
                pPrev += src_ps;
                pDiff += dst_ps;
            }
            break;
          case TWO_D4:
            for(w=width; --w; ) {
                int X = pCurr[0];
                int Ra = pCurr[a];
                int Rb = pPrev[0];
                pred = (Ra + Rb) >> 1;
                *pDiff = X - pred;
                pCurr += src_ps;
                pPrev += src_ps;
                pDiff += dst_ps;
            }
            break;
          default:
            break;

        } // End switch

    } // End band loop
}

//------------------------------------------------------------------------
//
//  Function:        Predictor::predict16
//
//  Description:
//    This function calculates the prediction error for one scanline
//    of 16 bit data. It uses the specified predictor, except on the 
//    first line and RSTs. In that case it uses the ONE_D1 predictor.
//        
//------------------------------------------------------------------------

void Predictor::predict16(Xil_signed16* src,
                          Xil_boolean   doRestart)
{
    //
    // If the point transform is required, do it into the
    // saved buffer, so we don't perform it multiple times.
    // Here we're using two buffers and we toggle their usage.
    // After each line is processed, the currLine becomes the prevLine.
    //
    Xil_signed16* prevLine;
    Xil_signed16* currLine;
    if(pt_transform != 0) {
        if(doRestart) {
            // Reset counter at image start and RST sections
            rowCounter = 0;
        }

        //
        // Set the temp buffer as the source for the prediction op
        //
        prevLine = (Xil_signed16*)saveBuf[rowCounter];
        currLine = (Xil_signed16*)saveBuf[1 - rowCounter];

        //
        // Toggle rowCounter between 0 and 1 to alternate saved buffer usage
        //
        rowCounter ^= 1;

        //
        // Perform the point transform on all bands
        //
        for(int b=0; b<bands_per_scan; b++) {
            Xil_signed16* pSrc = src + b;
            Xil_signed16* pDst = currLine + b;
            for(int i=0; i<width; i++) {
                *pDst = *pSrc >> pt_transform;
                pSrc += src_ps;
                pDst += dst_ps;
            }
        }

    } else {
        //
        // Set the image as the source
        //
        currLine = src;
        prevLine = src - src_ss; 
    }    


    for(int b=0; b<bands_per_scan; b++) {
        int a = -src_ps;
        int c = -src_ps;

        int pred;
        int predictor = prediction_type;
        if(doRestart) {
            //
            // Always use the ONE_D1 predictor on first line and RSTs
            // Also, use the half range value as the first pixel predictor
            predictor = ONE_D1;
            pred = first_pixel;
        } else {
            pred = prevLine[b];
        }

        //
        // Do the first pixel in the line
        //
        diffBuf[b] = currLine[b] - pred;

        //
        // Init the src and dst ptrs to the second pixel on the line
        //
        Xil_signed16* pCurr = currLine + src_ps + b;
        Xil_signed16* pPrev = prevLine + src_ps + b;
        Xil_signed16* pDiff = diffBuf + dst_ps + b;

        int w;
        switch(predictor) {
          case ONE_D1: // Predictor 1
            for(w=width; --w; ) {
                int X = pCurr[0];
                int Px = pCurr[a];
                *pDiff = X - Px;
                pCurr += src_ps;
                pDiff += dst_ps;
            }
            break;
          case ONE_D2: // Predictor 2
            for(w=width; --w; ) {
                int X = pCurr[0];
                int Px = pPrev[0];
                *pDiff = X - Px;
                pCurr += src_ps;
                pPrev += src_ps;
                pDiff += dst_ps;
            }
            break;
          case ONE_D3: // Predictor 3
            for(w=width; --w; ) {
                int X = pCurr[0];
                int Px = pPrev[c];
                *pDiff = X - Px;
                pCurr += src_ps;
                pPrev += src_ps;
                pDiff += dst_ps;
            }
            break;
          case TWO_D1: // Predictor 4
            for(w=width; --w; ) {
                int X = pCurr[0];
                int Ra = pCurr[a];
                int Rb = pPrev[0];
                int Rc = pPrev[c];
                *pDiff = X - (Ra + Rb - Rc);
                pCurr += src_ps;
                pPrev += src_ps;
                pDiff += dst_ps;
            }
            break;
          case TWO_D2: // Predictor 5
            for(w=width; --w; ) {
                int X = pCurr[0];
                int Ra = pCurr[a];
                int Rb = pPrev[0];
                int Rc = pPrev[c];
                *pDiff = X - (Ra + ((Rb - Rc) >> 1));
                pCurr += src_ps;
                pPrev += src_ps;
                pDiff += dst_ps;
            }
            break;
          case TWO_D3: // Predictor 6
            for(w=width; --w; ) {
                int X = pCurr[0];
                int Ra = pCurr[a];
                int Rb = pPrev[0];
                int Rc = pPrev[c];
                *pDiff = X - (Rb + ((Ra - Rc) >> 1));
                pCurr += src_ps;
                pPrev += src_ps;
                pDiff += dst_ps;
            }
            break;
          case TWO_D4: // Predictor 7
            for(w=width; --w; ) {
                int X = pCurr[0];
                int Ra = pCurr[a];
                int Rb = pPrev[0];
                *pDiff = X - ((Ra + Rb) >> 1);
                pCurr += src_ps;
                pPrev += src_ps;
                pDiff += dst_ps;
            }
            break;
          default:
            break;

        } // End switch

    } // End band loop
}

//
// Return the line of prediction errors
//
Xil_signed16*
Predictor::getDiffs()
{
    return diffBuf;
}
