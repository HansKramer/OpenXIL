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
//  File:       decompress.cc
//  Project:    XIL
//  Revision:   1.15
//  Last Mod:   10:15:06, 03/10/00
//
//  Description:
//
//    Jpeg Lossless decompression implementation
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)decompress.cc	1.15\t00/03/10  "

#include <xil/xilGPI.hh>
#include "XilDeviceCompressionJpegLL.hh"
#include "Reconstructor.hh"

XilStatus
XilDeviceCompressionJpegLL::decompress(XilOp*       op,
                                       unsigned int op_count,
                                       XilRoi*      roi,
                                       XilBoxList*  bl)
{
    DecompressInfo di(op, op_count, roi, bl);
    if( ! di.isOK()) {
        return XIL_FAILURE;
    }

    setInMolecule(op_count > 1);

    //
    // Seek to the proper frame. Deferred execution may have
    // caused other frames to be decompressed before this one.
    //
    seek(di.frame_number);

    //
    // Test whether the cis image and the dst image are the same
    // size and that the dst image has no ROI. This allows simpler 
    // decompression, without the need for a temporary image.
    //
    if(cisFitsInDst(roi)) {
        //
        // We can decompress directly into the dst image.
        //
        if(decompressFrame(&di) == XIL_FAILURE) {
            return XIL_FAILURE;
        }

    } else {
        //
        // We need to decompress into a temporary buffer first.
        // Use the special DecompressInfo constructor to create one.
        //
        DecompressInfo tmp_di(&di);
        if(! tmp_di.isOK()) {
            return XIL_FAILURE;
        }

        //
        // Decompress into the tmp buffer
        //
        if(decompressFrame(&tmp_di) == XIL_FAILURE) {
            return XIL_FAILURE;
        }

        //
        // Now copy the rects into the dst image
        //
        di.copyRects(&tmp_di);

    }

    return XIL_SUCCESS; 
}


XilStatus
XilDeviceCompressionJpegLL::decompressFrame(DecompressInfo* di)
{
    Xil_unsigned8* dataptr = (Xil_unsigned8*)di->image_dataptr;
    unsigned int   dst_ps  = di->image_ps;
    unsigned int   dst_ss  = di->image_ss;
    unsigned int   dst_bs  = di->image_bs;

    //
    // Reset the bitstream parser state
    //
    parser->reset();

    //
    // If a HuffmanDecoder has not been instantiated, create a new one
    //
    if(decoder == NULL) {
        decoder = new JpegLLHuffmanDecoder();
        if(decoder == NULL) {
            XIL_ERROR(system_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }
    }

    // 
    // Init the decoder
    //
    decoder->initdecode();

    //
    // Get the next frame from the buffer manager
    //
    parser->rdptr = cbm->nextFrame(&parser->endOfBuffer);
    if(parser->rdptr == NULL) {
        XIL_ERROR( NULL, XIL_ERROR_SYSTEM,"di-95",TRUE);  
        return XIL_FAILURE;
    }

    //
    // Parse the bitstream to read the SOF frame info 
    // and the first SOS scan info
    //
    if(! parser->readtoscan()) {
        XIL_ERROR(NULL, XIL_ERROR_USER,"di-285",FALSE);
        return XIL_FAILURE;
    }

    //
    // We don't handle sub-sampled Jpeg Lossless yet,
    // only 1x1 sampling
    //
    if(parser->header.maxh != 1 || parser->header.maxv != 1) {
        //
        // TODO: add handle for horiz/vertical sampling factors
        //  decompressSubsampledFrame( ... )
        //
        XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-95", TRUE);
        return (XIL_FAILURE);
    }

    frameWidth  = parser->header.width;
    frameHeight = parser->header.height;
    frameNbands = parser->header.bands;

    //
    // Loop over all scans in the frame
    // There can be variable number of components in each scan
    // So we need to populate each image band regardless of how
    // it was packed into scans.
    //
    while(1) {

        JpegLLScanInfo scanInfo;

        int component = parser->header.scancomponents[0];

        //
        // Load up the scanInfo structure with info about current scan
        //
        scanInfo.width = parser->header.width;
        scanInfo.height = parser->header.height;
        scanInfo.nbands = parser->header.scanbands;
        scanInfo.ps     = dst_ps;
        scanInfo.ss     = dst_ss;
        scanInfo.bs     = dst_bs;
        scanInfo.precision = parser->header.precision;
        if(scanInfo.precision == 8) {
            scanInfo.dataptr = dataptr + component;
        } else {
            scanInfo.dataptr = dataptr + component*sizeof(Xil_signed16);
        }
        scanInfo.predictor = parser->header.scan_selection;
        scanInfo.transform = parser->header.scan_pt_transform;
        scanInfo.restartInterval = parser->header.restartInterval /
                                   parser->header.width;


        if(scanInfo.nbands > 1) {
            decompressInterleavedScan(&scanInfo);
        } else {
            decompressNonInterleavedScan(&scanInfo);
        }

        //
        // Apply the point transform, if necesary
        //
        if(scanInfo.transform != 0) {
            if(scanInfo.precision == 8) {
                Xil_unsigned8* pScan = dataptr + component;
                for(int y=0; y<scanInfo.height; y++) {
                    Xil_unsigned8* pPixel = pScan;
                    for(int x=0; x<scanInfo.width; x++) {
                        Xil_unsigned8* pBand = pPixel;
                        for(int b=0; b<scanInfo.nbands; b++) {
                            *pBand <<= scanInfo.transform;
                            pBand++;
                        }
                        pPixel += dst_ps;
                    }
                    pScan += dst_ss;
                }
            } else {
                Xil_signed16* pScan = (Xil_signed16*)dataptr + component;
                for(int y=0; y<scanInfo.height; y++) {
                    Xil_signed16* pPixel = pScan;
                    for(int x=0; x<scanInfo.width; x++) {
                        Xil_signed16* pBand = pPixel;
                        for(int b=0; b<scanInfo.nbands; b++) {
                            *pBand <<= scanInfo.transform;
                            pBand++;
                        }
                        pPixel += dst_ps;
                    }
                    pScan += dst_ss;
                }
            }
        }

        
        if((frameNbands -= scanInfo.nbands) > 0) {
            if(! parser->readtoscan()) {
                XIL_ERROR(NULL, XIL_ERROR_USER,"di-285",FALSE);
                return XIL_FAILURE;
            }
        } else {
            break;
        }

    }  // end while

    decoder->finishdecode(&parser->rdptr, parser->endOfBuffer);
    cbm->decompressedFrame(parser->rdptr);

    return XIL_SUCCESS;
}

XilStatus
XilDeviceCompressionJpegLL::decompressInterleavedScan(JpegLLScanInfo* si)
{
    // 
    // Make a reconstructor object for this scan
    //
    Reconstructor* recon = new Reconstructor(si);
    if(! recon->isOK()) {
        delete recon;
        return XIL_FAILURE;
    }


    //
    // Restart on first line
    //
    Xil_boolean doRestart = TRUE;
    int         MCUcount  = 0;
    Xil_unsigned8* pDst = (Xil_unsigned8*)si->dataptr;

    for(int y=0; y<si->height; y++) {

        Xil_signed16* diff = recon->diffBuf;

        //
        // Decode a line (containing N bands) into the difference buffer
        //
        for(int i=0; i<si->width; i++) {
            for(int b=0; b<si->nbands; b++) {
                int component = parser->header.scancomponents[b];
                decoder->decode_ll(&parser->rdptr,
                       parser->header.band_data[component].table,
                       diff, 1, parser->endOfBuffer);
                diff++;
            }
        }

        //
        // Reconstruct the line (add prediction errors back in)
        //
        if(si->precision == 8) {
            recon->reconstruct8(pDst, doRestart);
            pDst += si->ss;
        } else {
            recon->reconstruct16((Xil_signed16*)pDst, doRestart);
            pDst += sizeof(Xil_signed16) * si->ss;
        }

        //
        // Check for end of restart interval (restart count must
        // be an integral multiple of rows)
        // (Bias performance toward restartInterval == 0)
        //
        if((si->restartInterval != 0) && (++MCUcount == si->restartInterval)) {
            MCUcount = 0;
            decoder->initdecode();
            decoder->finishRstInterval(&parser->rdptr,parser->endOfBuffer);
            doRestart = TRUE;
        } else {
            doRestart = FALSE;
        }

    }
    delete recon;
    return XIL_SUCCESS;
}


XilStatus
XilDeviceCompressionJpegLL::decompressNonInterleavedScan(JpegLLScanInfo* si)
{
    // 
    // Make a reconstructor object for this scan
    //
    Reconstructor* recon = new Reconstructor(si);
    if(! recon->isOK()) {
        delete recon;
        return XIL_FAILURE;
    }

    // 
    // Re-initialize the decoder
    //
    decoder->initdecode();

    Xil_boolean doRestart = TRUE;
    int         MCUcount  = 0;
    int component = parser->header.scancomponents[0];
    Xil_unsigned8* pDst = (Xil_unsigned8*)si->dataptr;
    for(int y=0; y<si->height; y++) {

        //
        // Decode a line of prediction errors for one band
        //
        decoder->decode_ll(&parser->rdptr,
                           parser->header.band_data[component].table,
                           recon->diffBuf, si->width, parser->endOfBuffer);

        //
        // Reconstruct the line (add prediction errors back in)
        //
        if(si->precision == 8) {
            recon->reconstruct8(pDst, doRestart);
            pDst += si->ss;
        } else {
            recon->reconstruct16((Xil_signed16*)pDst, doRestart);
            pDst += sizeof(Xil_signed16) * si->ss;
        }

        //
        // Check for end of restart interval (restart count must
            // be an integral multiple of rows)
        // (Bias performance toward restartInterval == 0)
        //
        if(si->restartInterval && (++MCUcount == si->restartInterval)) {
            MCUcount = 0;
            decoder->initdecode();
            decoder->finishRstInterval(&parser->rdptr,parser->endOfBuffer);
            doRestart = TRUE;
        } else {
            doRestart = FALSE;
        }

    }
    delete recon;
    return XIL_SUCCESS;
}
