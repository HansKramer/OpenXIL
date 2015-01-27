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
//  File:       compress.cc
//  Project:    XIL
//  Revision:   1.7
//  Last Mod:   10:15:03, 03/10/00
//
//  Description:
//
//    TODO: Enter some descriptive text here
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)compress.cc	1.7\t00/03/10  "

#include "XilDeviceCompressionJpegLL.hh"
#include "Predictor.hh"
#include "XiliUtils.hh"

static void free_my_buffer(void* data)
{
    free(data);
}

static int get_grow_increment(int old_size, void*)
{
    return (int) (1.4F * old_size);
}

#define MAX_BYTES_PER_FRAME_HEADER      2000

#define COMPRESS_RATIO                        0.85F

XilStatus 
XilDeviceCompressionJpegLL::compress(XilOp*       op,
                                     unsigned int op_count,
                                     XilRoi*      roi,
                                     XilBoxList*  bl)
{
    CompressInfo ci(op, op_count, roi, bl);
    if(! ci.isOK()) {
        return XIL_FAILURE;
    }

    setInMolecule(op_count > 1);

    //
    // Create a resizable buffer object
    //
    sbuffer = new SingleBuffer;
    if(sbuffer == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return XIL_FAILURE;
    }

    //
    // Determine how large a buffer we need to hold the compressed frame
    // TODO: Why isn't this the same size that would be returned by 
    //       getMaxFrameSize()
    //
    unsigned int buffer_size = (int) (COMPRESS_RATIO * ci.image_data_quantity);
    buffer_size += MAX_BYTES_PER_FRAME_HEADER;
    if(!sbuffer->init(NULL, buffer_size, get_grow_increment, NULL)) {
        return XIL_FAILURE;
    }

    //
    // Check for band selectors, band pt_trans SAME.
    // Fallback to non-interleaved mode if more than 4 bands
    // or band selectors or point transforms differ.
    //
    int interleaved = encode_interleaved;
    if(interleaved) {        
        if(ci.image_nbands > 4) {
            // Fallback to non-interleaved
            XIL_ERROR(getSystemState(), XIL_ERROR_USER,"di-103",TRUE);  
            interleaved = FALSE;
        } else {
            //
            // Check for per band attributes same!  
            // Else force non-interleaved
            //
            for(int i=1; i<ci.image_nbands; i++) {
                if((selector[i] != selector[0]) || 
                   (pt_transform[i] != pt_transform[0])) {
                    XIL_ERROR(getSystemState(), XIL_ERROR_USER,"di-55",TRUE);  
                    interleaved = FALSE;
                    break;
                }
            }
        }
    }

    //
    //   Request Next Buffer
    //
    huffman_encoder->set_Buffer(sbuffer);

    //
    // Check to see if bandinfo changed
    //
    if(bandinfo_changed) {
        if(validateBandTableUsage(ci.image_nbands) == XIL_FAILURE) {
            return XIL_FAILURE;
        }
    }

    //
    // Output SOI Marker
    //
    sbuffer->addByte(MARKER);
    sbuffer->addByte(SOI);

    //
    // Optionally Output Table Information
    //
    if(abbr_format) {
        huffman_encoder->OutputChanges();
    } else {
        huffman_encoder->Output();
    }

    //
    // Output Frame Header Informantion
    //
    output_frame_header(&ci);

    //
    // (Finally !!), compress the frame
    //
    XilStatus status;
    if(interleaved) {
        status = compressInterleaved(&ci);
    } else {
        status = compressNonInterleaved(&ci);
    }

    huffman_encoder->Reset();
    output_trailer();

    Xil_unsigned8* data_ptr = sbuffer->getDataPtr();
    if(data_ptr == NULL) {
        return XIL_FAILURE;
    }

    if(status == XIL_SUCCESS) {
        putBitsPtr(sbuffer->getNumBytes(), 1, data_ptr, free_my_buffer);
    } else {
        free((void*)data_ptr);
    }

    return status;

}

//
// Compress with all bands interleaved in a single scan
//
XilStatus 
XilDeviceCompressionJpegLL::compressInterleaved(CompressInfo* ci)
{
    //
    // Create the Predictor object used to generate the
    // predictions to be encoded
    //
    Predictor* predictor = new Predictor(ci, pt_transform[0], 
                                         selector[0], TRUE);
    if(! predictor->isOK()) {
        delete predictor;
        return XIL_FAILURE;
    }

    //
    // Output Single Scan Header
    //
    output_scan_header(0U, ci->image_nbands);

    //
    // Set restart to TRUE for first scanline and RSTs
    //
    Xil_boolean doRestart = TRUE;        

    Xil_signed16*  diff;

    if(ci->image_datatype == XIL_BYTE) {
        Xil_unsigned8* src = (Xil_unsigned8*)ci->image_dataptr;
        for(int j=0; j<ci->image_box_height; j++) {
            predictor->predict8(src, doRestart);
            src += ci->image_ss;
            diff = predictor->getDiffs();
            huffman_encoder->Encode_ll(diff, ci->image_box_width, 
                                       ci->image_nbands, banddata);

            doRestart = FALSE;
        }
    } else {
        Xil_signed16* src = (Xil_signed16*)ci->image_dataptr;
        for(int j=0; j<ci->image_box_height; j++) {
            predictor->predict16(src, doRestart);
            src += ci->image_ss;
            diff = predictor->getDiffs();
            huffman_encoder->Encode_ll(diff, ci->image_box_width, 
                                       ci->image_nbands, banddata);

            doRestart = FALSE;
        }
    }

    //
    //   Flush Huffman Codes 
    //
    huffman_encoder->Flush_Codes( );

    delete predictor;

    return XIL_SUCCESS;
}

//
// Compress with a single band per scan
//
XilStatus 
XilDeviceCompressionJpegLL::compressNonInterleaved(CompressInfo* ci)
{
    for(unsigned int b=0; b<ci->image_nbands; b++) {

        Predictor* predictor = new Predictor(ci, pt_transform[b], 
                                             selector[b], FALSE);
        if(! predictor->isOK()) {
            delete predictor;
            return XIL_FAILURE;
        }
        
        //
        // Output Scan Header For Given Band
        //
        output_scan_header(b, 1U);

        //
        // Set restart True for first scanline and RSTs
        //
        Xil_boolean doRestart = TRUE;  

        Xil_signed16*  diff;

        if(ci->image_datatype == XIL_BYTE) {
            Xil_unsigned8* src = (Xil_unsigned8*)ci->image_dataptr + b;
            for(int j=0; j<ci->image_box_height; j++) {
                predictor->predict8(src, doRestart);
                src += ci->image_ss;
                diff = predictor->getDiffs();
                huffman_encoder->Encode_ll(diff, ci->image_box_width, 
                                           1U, banddata+b);

                doRestart = FALSE;
            }
        } else {
            Xil_signed16* src = (Xil_signed16*)ci->image_dataptr + b;
            for(int j=0; j<ci->image_box_height; j++) {
                predictor->predict16(src, doRestart);
                src += ci->image_ss;
                diff = predictor->getDiffs();
                huffman_encoder->Encode_ll(diff, ci->image_box_width, 
                                           1U, banddata+b);

                doRestart = FALSE;
            }
        }

        //
        //   Flush Huffman Codes
        //
        huffman_encoder->Flush_Codes( );

        delete predictor;
    }

    return XIL_SUCCESS;
}
