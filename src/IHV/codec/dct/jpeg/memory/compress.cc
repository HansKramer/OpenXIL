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
//  Revision:   1.14
//  Last Mod:   10:14:31, 03/10/00
//
//  Description:
//
//    The actual compression function for Jpeg
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)compress.cc	1.14\t00/03/10  "

#include <xil/xilGPI.hh>
#include "XilDeviceCompressionJpeg.hh"
#include "CompressInfo.hh"
#include "Dct.hh"

//
// MAX_BITS_PER_PIXEL_411 is 2 bits/pixel for highest quality jpeg.
//
// TODO: (lperry) I'd question this value. If the caller specified
//       a high quality setting the bits_per_pixel could be up to 4.
//
const Xil_float32  MAX_BITS_PER_PIXEL_411     = 2.0F;

//
// MAX_BITS_PER_BAND is MAX_BITS_PER_PIXEL_411 
// adjusted for no subsampling
//
const Xil_float32  MAX_BITS_PER_BAND          = 1.35F;
const unsigned int MAX_BYTES_PER_FRAME_HEADER = 2000;

//
// TODO: Get these into the class !!!
//
static void
freeSingleBuffer(void* data)
{
    free(data);
}

static int 
getGrowIncrement(int   old_size, void*)
{
    return (int) (1.4F * old_size);
}


XilStatus
XilDeviceCompressionJpeg::compress(XilOp*        op,
                                   unsigned int  op_count,
                                   XilRoi*       roi,
                                   XilBoxList*   bl)
{
    //
    // Create an object to hold most of the info
    // for this frame compression
    //
    CompressInfo ci(op, op_count, roi, bl);
    if (! ci.isOK()) {
        return XIL_FAILURE;
    }

    setInMolecule(op_count > 1);

    //
    // If the image is no larger than 640x480x3,
    // use the CBM facilities. Otherwise,
    // use the SingleBuffer class, which will do
    // realloc() ops as necessary.
    //
    int            buffer_size;
    Xil_unsigned8* cbm_buffer_space;
    if(ci.image_nbands*ci.image_box_width*ci.image_box_height<= 640*480*3) {
        cbm_buffer_space = cbm->nextBufferSpace();
        if(!cbm_buffer_space) {
            return XIL_FAILURE;
        }
        buffer_size = cbm->getFrameSize();
    } else {
        cbm_buffer_space = 0;
        if(compData.bytes_per_frame <= 0) {
            unsigned int bits_per_pixel;
            unsigned int max_bits_per_frame;

            if(compData.encode_video && ci.image_nbands == 3) {
                bits_per_pixel = MAX_BITS_PER_PIXEL_411;
            } else {
                bits_per_pixel = MAX_BITS_PER_BAND * ci.image_nbands;
            }
            max_bits_per_frame = bits_per_pixel *
                                 ((ci.image_box_width + 7) & ~7) * 
                                 ((ci.image_box_height + 7) & ~7);

            buffer_size = ((max_bits_per_frame + 7) >> 3) + 
            MAX_BYTES_PER_FRAME_HEADER;
        } else {
            buffer_size = (int) (compData.bytes_per_frame * 1.4F);
        }
    }

    SingleBuffer buffer;
    if(buffer.init(cbm_buffer_space, buffer_size, 
                   getGrowIncrement, NULL) == NULL) {
        return XIL_FAILURE;
    }
    compData.buffer = &buffer;

    //
    // Pass the CompressInfo object to the setup routine.
    // This routine will, in turn, call the compression function
    // for the specific mode (411, interleaved, non-interleaved).
    //
    XilStatus retval =  compressFrame(&ci);

    //
    // Post-compression stuff. Update the cbm  info
    //
    Xil_unsigned8* data_ptr;
    if(!(data_ptr = buffer.getDataPtr())) {
        return XIL_FAILURE;
    }

    if(retval == XIL_SUCCESS) {
        compData.bytes_per_frame = buffer.getNumBytes();
        if(cbm_buffer_space) {
            cbm->doneBufferSpace(compData.bytes_per_frame);
        } else {
            putBitsPtr(compData.bytes_per_frame, 1, 
            data_ptr, freeSingleBuffer);
        }
    } else {
        if(cbm_buffer_space) {
            cbm->doneBufferSpace(-1);
        } else {
            free((void*)data_ptr);
        }
    }

    return retval;
}


//------------------------------------------------------------------------
//
//  Function:        XilDeviceCompressionJpeg::compressFrame()
//
//  Description:
//        
//    Set up compression by validating attributes and doing what is
//    common to the three different compress routines.
//        
//------------------------------------------------------------------------

XilStatus 
XilDeviceCompressionJpeg::compressFrame(CompressInfo* ci)
{
   
    //
    // Perform temporal filtering, if selected by attribute.
    // Maintains a buffer to hold the latest filtered results.
    //
    if(compData.filter_image && (ci->image_nbands == 3)) {
        if(filterSetup(ci) == XIL_FAILURE) {
            XIL_ERROR(system_state, XIL_ERROR_RESOURCE, "di-1", FALSE);
            return XIL_FAILURE;
        }
    } else {
        compData.gob_getter->useImage(ci);
    }

    compData.nbands = ci->image_nbands;

    //
    // Validate that 411 encoding is possible. Must be 3 bands
    //
    Xil_boolean encode_411 = (compData.encode_video && (ci->image_nbands == 3));
    use411Sampling(encode_411);

    //
    // Validate that interleaved encoding is possible. Must be <=4 bands
    //
    Xil_boolean interleaved = compData.encode_interleaved && 
                              (ci->image_nbands <= 4);


    //------------------------------------
    //   Set Buffer
    //------------------------------------

    compData.huffman_encoder->set_Buffer(compData.buffer);
    compData.quantizer->set_Buffer(compData.buffer);

    //------------------------------------
    // Check to see if bandinfo changed
    //------------------------------------
    if(compData.bandinfo_changed) {
        if(validateBandTableUsage() == XIL_FAILURE) {
            return XIL_FAILURE;
        }
    }

    //------------------------------------
    // Output SOI Marker
    //------------------------------------

    compData.buffer->addByte(MARKER);
    compData.buffer->addByte(SOI);

    //------------------------------------
    // Optionally Output Table Information
    //------------------------------------
    if(compData.abbr_format) {
        if(!compData.use_optimal_htables) {
            compData.huffman_encoder->OutputChanges();
        }
        compData.quantizer->OutputChanges();
    } else {
        if(!compData.use_optimal_htables) {
            compData.huffman_encoder->Output();
        }
        compData.quantizer->Output();
    }
    compData.qflag = FALSE;

    //------------------------------------
    // Output Frame Header Informantion
    //------------------------------------

    XilStatus success;

    if(compData.huffman_encoder->numTablesInUse()<=4) {
        output_header(compData.buffer, ci->image_box_width, ci->image_box_height, 
                      ci->image_nbands, JPEG_BASELINE, compData.banddata);
    } else {
        output_header(compData.buffer, ci->image_box_width, ci->image_box_height, 
                      ci->image_nbands, JPEG_EXTENDED, compData.banddata);
    }

    if(encode_411) {
        success = compress411(ci->image_box_width, ci->image_box_height);
    } else if(interleaved) {
        success = compressInterleaved(ci->image_box_width, ci->image_box_height);
    } else {
        success = compressNonInterleaved(ci->image_box_width, ci->image_box_height);
    }

    if(success == XIL_SUCCESS) {
        compData.huffman_encoder->Reset();
        output_trailer( compData.buffer );
    }

    return success;
}

//------------------------------------------------------------------------
//
//  Function:   XilDeviceCompressionJpeg::compressInterleaved(
//                 unsigned w, unsigned h)
//
//  Description:
//        
//        Compress into interleaved format (1:1:1:etc)
//        
//------------------------------------------------------------------------

XilStatus
XilDeviceCompressionJpeg::compressInterleaved(unsigned w, 
                                              unsigned h)
{
    int i,j,b;

    //-----------------------------------------
    // Cache object pointers (into registers)
    //-----------------------------------------

    Jpeg_Quantizer* quantizer = compData.quantizer;
    JpegOptHuffmanEncoder* huffman_encoder = compData.huffman_encoder;
    GobGetter* gob_getter = compData.gob_getter;
    int* gob = compData.gob;
    JpegBandInfo* banddata = compData.banddata;
    Xil_boolean use_optimal_htables = compData.use_optimal_htables;
    int nbands =  compData.nbands;

    if(!use_optimal_htables) {

        //------------------------------------
        // Output Single Scan Header
        //------------------------------------

        output_scan_header(compData.buffer, nbands, banddata);
    }

    //------------------------------------
    // Start Parsing Image
    //------------------------------------

    for(j = 0; j < h; j += BLOCK_SIZE) {
        for(i = 0; i < w; i += BLOCK_SIZE) {

            //------------------------------------
            //   Fill MacroBlocks
            //------------------------------------

            gob_getter->getInterleavedGob( i, j, nbands, gob );

            //------------------------------------
            //   Extract, Quantize, and Encode
            //   Interleaved Blocks
            //------------------------------------
            for(b=0; b<nbands; b++) {
                int* pBlk = gob + b*64;
                Dct8x8( pBlk );
                quantizer->Quantize( pBlk, banddata[b].getQtableId());
                if(use_optimal_htables) {
                    huffman_encoder->Encode(pBlk, b, banddata[b].getDcHtableId(),
                    banddata[b].getAcHtableId());
                } else {
                    huffman_encoder->Jpeg_Huffman_Encoder::Encode(pBlk, b,
                    banddata[b].getDcHtableId(),
                    banddata[b].getAcHtableId());  
                }
            }
        }
    }

    if(use_optimal_htables) {
        // generate the tables
        ((JpegOptHuffmanEncoder*)huffman_encoder)->generateTables();

        // output the tables
        huffman_encoder->Output();

        //------------------------------------
        // Output Single Scan Header
        //------------------------------------

        output_scan_header(compData.buffer, nbands, banddata);
    }

    huffman_encoder->Flush_Codes();

    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:        XilDeviceCompressionJpeg::compressNonInterleaved(unsigned w, unsigned h)
//  Created:        92/09/15
//
//  Description:
//        
//        Compress into a noninterleaved format. Each band compressed completely
//      before the next one.
//        
//------------------------------------------------------------------------

XilStatus 
XilDeviceCompressionJpeg::compressNonInterleaved(unsigned w, 
                                                 unsigned h)
{
    int i,j,b;

    //-----------------------------------------
    // Cache object pointers (into registers)
    //-----------------------------------------

    Jpeg_Quantizer* quantizer = compData.quantizer;
    JpegOptHuffmanEncoder* huffman_encoder = compData.huffman_encoder;
    GobGetter* gob_getter = compData.gob_getter;
    int* gob = compData.gob;
    JpegBandInfo* banddata = compData.banddata;
    Xil_boolean use_optimal_htables = compData.use_optimal_htables;
    int nbands =  compData.nbands;

    for(b=0; b<nbands; b++) {

        if(!use_optimal_htables) {  

            //------------------------------------
            // Output Scan Header For Given Band
            //------------------------------------

            output_scan_header_for_band(compData.buffer, b, banddata);
        }

        //---------------------
        // Start Parsing Image
        //---------------------

        for(j = 0; j < h; j += BLOCK_SIZE) {
            for(i = 0; i < w; i += BLOCK_SIZE) {

                //---------------------
                //   Fill MacroBlocks
                //---------------------

                gob_getter->getNonInterleavedGob( i, j, b, gob );

                //-----------------------------------------
                //   Extract, DCT transform, Quantize, and Encode Blocks
                //-----------------------------------------
                Dct8x8( gob );
                quantizer->Quantize( gob,banddata[b].getQtableId());
                if(use_optimal_htables) {
                    huffman_encoder->Encode( gob, b, banddata[b].getDcHtableId(),
                    banddata[b].getAcHtableId());
                } else  {
                    huffman_encoder->Jpeg_Huffman_Encoder::Encode( gob, b,
                    banddata[b].getDcHtableId(),
                    banddata[b].getAcHtableId());
                }

            }
        }

        if(use_optimal_htables) {

            // generate the tables
            ((JpegOptHuffmanEncoder*)huffman_encoder)->generateTables(banddata[b].getDcHtableId(),banddata[b].getAcHtableId());

            // output the tables for this band only
            huffman_encoder->Output(banddata[b].getDcHtableId());
            huffman_encoder->Output(banddata[b].getAcHtableId());

            //------------------------------------
            // Output Scan Header For Given Band
            //------------------------------------

            output_scan_header_for_band(compData.buffer, b, banddata);
        }

        //------------------------
        //   Flush Huffman Codes 
        //------------------------

        huffman_encoder->Flush_Codes( );
    }        

    return XIL_SUCCESS;  
}

//------------------------------------------------------------------------
//
//  Function:        XilDeviceCompressionJpeg::compress411( unsigned w, unsigned h )
//  Created:        92/09/15
//
//  Description:
//        
//        Compress into video (4:1:1) format.
//        
//------------------------------------------------------------------------

XilStatus 
XilDeviceCompressionJpeg::compress411(unsigned w, 
                                          unsigned h)
{
    int i, j;

    //-----------------------------------------
    // Cache object pointers (into registers)
    //-----------------------------------------

    Jpeg_Quantizer* quantizer              = compData.quantizer;
    JpegOptHuffmanEncoder* huffman_encoder = compData.huffman_encoder;
    GobGetter* gob_getter                  = compData.gob_getter;
    int* gob                               = compData.gob;
    JpegBandInfo* banddata                 = compData.banddata;
    Xil_boolean use_optimal_htables        = compData.use_optimal_htables;
    int nbands                             = compData.nbands;

    if(!use_optimal_htables) {

        //------------------------------------
        // Output Scan Header
        //------------------------------------

        output_scan_header(compData.buffer, nbands, banddata);
    }

    //------------------------------------
    // Start Parsing Image
    //------------------------------------

    for(j = 0; j < h; j += MACRO_BLOCK_SIZE_411) {
        for(i = 0; i < w; i += MACRO_BLOCK_SIZE_411) {

            //------------------------------------
            //   Fill MacroBlocks
            //------------------------------------

            gob_getter->getInterleaved411Gob( i, j, gob );

            //------------------------------------
            //   Extract, Quantize, and Encode
            //   Interleaved Blocks
            //------------------------------------

            //
            // TODO: This might be a great place to multi-thread.
            //       Each thread would be responsible for one
            //       block, which it would dct, quantize and encode.
            //       Or is that too little work for a thread?
            //

            Dct8x8( gob + Y0*64 );
            Dct8x8( gob + Y1*64 );
            Dct8x8( gob + Y2*64 );
            Dct8x8( gob + Y3*64 );
            Dct8x8( gob + U0*64 );
            Dct8x8( gob + V0*64 );

            quantizer->Quantize( gob+Y0*64,banddata[YBAND].getQtableId());
            quantizer->Quantize( gob+Y1*64,banddata[YBAND].getQtableId());
            quantizer->Quantize( gob+Y2*64,banddata[YBAND].getQtableId());
            quantizer->Quantize( gob+Y3*64,banddata[YBAND].getQtableId());
            quantizer->Quantize( gob+U0*64,banddata[UBAND].getQtableId());
            quantizer->Quantize( gob+V0*64,banddata[VBAND].getQtableId());

            if(use_optimal_htables) {
                huffman_encoder->Encode( gob+Y0*64, YBAND,
                banddata[YBAND].getDcHtableId(),
                banddata[YBAND].getAcHtableId());

                huffman_encoder->Encode( gob+Y1*64, YBAND,
                banddata[YBAND].getDcHtableId(),
                banddata[YBAND].getAcHtableId());

                huffman_encoder->Encode( gob+Y2*64, YBAND,
                banddata[YBAND].getDcHtableId(),
                banddata[YBAND].getAcHtableId());

                huffman_encoder->Encode( gob+Y3*64, YBAND,
                banddata[YBAND].getDcHtableId(),
                banddata[YBAND].getAcHtableId());

                huffman_encoder->Encode( gob+U0*64, UBAND,
                banddata[UBAND].getDcHtableId(),
                banddata[UBAND].getAcHtableId());

                huffman_encoder->Encode( gob+V0*64, VBAND,
                banddata[VBAND].getDcHtableId(),
                banddata[VBAND].getAcHtableId());
            } else {
                huffman_encoder->Jpeg_Huffman_Encoder::Encode( gob+Y0*64, YBAND,
                banddata[YBAND].getDcHtableId(),
                banddata[YBAND].getAcHtableId());

                huffman_encoder->Jpeg_Huffman_Encoder::Encode( gob+Y1*64, YBAND,
                banddata[YBAND].getDcHtableId(),
                banddata[YBAND].getAcHtableId());

                huffman_encoder->Jpeg_Huffman_Encoder::Encode( gob+Y2*64, YBAND,
                banddata[YBAND].getDcHtableId(),
                banddata[YBAND].getAcHtableId());

                huffman_encoder->Jpeg_Huffman_Encoder::Encode( gob+Y3*64, YBAND,
                banddata[YBAND].getDcHtableId(),
                banddata[YBAND].getAcHtableId());

                huffman_encoder->Jpeg_Huffman_Encoder::Encode( gob+U0*64, UBAND,
                banddata[UBAND].getDcHtableId(),
                banddata[UBAND].getAcHtableId());

                huffman_encoder->Jpeg_Huffman_Encoder::Encode( gob+V0*64, VBAND,
                banddata[VBAND].getDcHtableId(),
                banddata[VBAND].getAcHtableId());
            }
        }
    }

    if(use_optimal_htables) {

        // generate the tables
        huffman_encoder->generateTables();

        // output the tables
        huffman_encoder->Output();

        //------------------------------------
        // Output Scan Header
        //------------------------------------

        output_scan_header(compData.buffer, nbands, banddata);
    }

    // flush huffman encoder
    huffman_encoder->Flush_Codes( );

    return XIL_SUCCESS;  
}          


//------------------------------------------------------------------------
//
//  Function:        XilDeviceCompressionJpeg::use411Sampling(Xil_boolean use_411)
//  Created:        92/09/15
//
//  Description:
//        
//        Sets up band sampling for 411 or 1:1:1...
//        
//------------------------------------------------------------------------

void 
XilDeviceCompressionJpeg::use411Sampling(Xil_boolean use_411)
{
    if(use_411 != compData.using_411_bandinfo) {

        int hvsample = (use_411) ? 2 : 1;

        compData.banddata[0].setH(hvsample);
        compData.banddata[0].setV(hvsample);

        compData.using_411_bandinfo = use_411;
    }
}

//------------------------------------------------------------------------
//
//  Function:        XilDeviceCompressionJpeg::validateBandTableUsage()
//  Created:        92/09/15
//
//  Description:
//        
//        Validates band table usage. Since one can set a band to a table
//      before a table is actually loaded, this routine checks at the
//      time of compress that tables used by bands are actually loaded.
//        
//------------------------------------------------------------------------

XilStatus
XilDeviceCompressionJpeg::validateBandTableUsage()
{
    int i;

    //-----------------------------------------
    // Cache object pointers (into registers)
    //-----------------------------------------

    Jpeg_Quantizer* quantizer = compData.quantizer;
    JpegOptHuffmanEncoder* huffman_encoder = compData.huffman_encoder;
    JpegBandInfo* banddata = compData.banddata;
    Xil_boolean use_optimal_htables = compData.use_optimal_htables;
    int nbands =  compData.nbands;

    quantizer->resetTableUsage();
    huffman_encoder->resetTableUsage();

    // parse through bandinfo resetting table usage
    for(i=0; i<nbands; i++) {


        if( quantizer->tableLoaded(banddata[i].getQtableId()) ) {
            quantizer->usingTable(banddata[i].getQtableId());
        } else {

            // Jpeg bitstream error: attempted use of non loaded qtable 
            XIL_ERROR( NULL, XIL_ERROR_USER, "di-89", TRUE);     

            return XIL_FAILURE;
        }


        if(use_optimal_htables || 
           huffman_encoder->tableLoaded(banddata[i].getDcHtableId())) {
            huffman_encoder->usingTable(banddata[i].getDcHtableId());
        } else {

            // Jpeg bitstream error: attempted use of non loaded dc table  
            XIL_ERROR( NULL, XIL_ERROR_USER, "di-89", TRUE);     

            return XIL_FAILURE;
        }


        if(use_optimal_htables ||       
           huffman_encoder->tableLoaded(banddata[i].getAcHtableId())) {
            huffman_encoder->usingTable(banddata[i].getAcHtableId());
        } else {

            // Jpeg bitstream error: attempted use of non loaded ac table   
            XIL_ERROR( NULL, XIL_ERROR_USER, "di-88", TRUE);     

            return XIL_FAILURE;
        }
    }

    return XIL_SUCCESS;  
}


//
// Utility to copy image storage to a private buffer
//
void
XilDeviceCompressionJpeg::copyImageToBuf(CompressInfo* ci,
                                         Xil_unsigned8* prvBuf)
{
    unsigned int prv_ss = ci->image_box_width*3;
    unsigned int prv_ps = 3;

    unsigned int cur_ss = ci->image_ss;
    unsigned int cur_ps = ci->image_ps;
 
    Xil_unsigned8* prv_scan = prvBuf;
    Xil_unsigned8* cur_scan = (Xil_unsigned8*)ci->image_dataptr;
    for(int r=0; r<(int)ci->image_box_height; r++) {
        Xil_unsigned8* prv_pixel = prv_scan;
        Xil_unsigned8* cur_pixel = cur_scan;
        for(int c=0; c<(int)ci->image_box_width; c++) {
            prv_pixel[0] = cur_pixel[0];
            prv_pixel[1] = cur_pixel[1];
            prv_pixel[2] = cur_pixel[2];
 
            cur_pixel += cur_ps;
            prv_pixel += prv_ps;
        }
        cur_scan += cur_ss;
        prv_scan += prv_ss;
    }
}

//
// Setup to perform temporal filtering.
// On the first frame, allocate a private buffer and
// copy the incoming image into it. 
// On succeeding frames, perform the filtering into
// another private buffer and specify that this buffer
// is to be used by the GOB getter
//
XilStatus 
XilDeviceCompressionJpeg::filterSetup(CompressInfo* ci)
{

    if(compData.previous_image == NULL) {
        //
        // Create a buffer to hold the first frame
        //
        compData.previous_image = 
            new Xil_unsigned8[ci->image_box_width*ci->image_box_height*ci->image_nbands];
        if(compData.previous_image == NULL) {
            XIL_ERROR(system_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }

        // 
        // Copy the incoming image into the previous_image buffer
        //
        copyImageToBuf(ci, compData.previous_image);

        //
        // Tell the GOB_getter about the storage layout
        // of the incoming image.
        //
        compData.gob_getter->useImage(ci);

    } else {
        //
        //  Create a temporary buffer to store the final filtered image.
        //
        compData.filteredBuf = 
            new Xil_unsigned8[ci->image_box_width*ci->image_box_height*ci->image_nbands];
        if(compData.filteredBuf == NULL) {
            XIL_ERROR(system_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
            return XIL_FAILURE;
        }

        //
        // Do the temporal filtering, placing the results in
        // filteredBuf.
        //
        if(temporalFilterImages(compData.previous_image,
                                ci, compData.filteredBuf,
                                16, 32) == XIL_FAILURE) {
            return XIL_FAILURE;
        }
        //
        // Tell the GOB_getter about the storage layout of
        // the filtered image, specifying the parameters explicitly.
        //
        compData.gob_getter->useImage(ci, compData.filteredBuf);
    }

    return XIL_SUCCESS;
}
