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
//This line lets emacs recognize this as -*- C++ -*- Code
//------------------------------------------------------------------------
//
//  File:       XilDeviceCompressionJpeg.cc
//  Project:    XIL
//  Revision:   1.7
//  Last Mod:   10:14:33, 03/10/00
//
//  Description:
//
//    Class implementation for the derived Jpeg Compression device
//
//    This class is the definition of Jpeg compression and
//    decompression within XIL.  There is one of these objects per
//    Baseline Jpeg cis.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDeviceCompressionJpeg.cc	1.7\t00/03/10  "

#include "XilDeviceCompressionJpeg.hh"

XilDeviceCompressionJpeg::XilDeviceCompressionJpeg(
    XilDeviceManagerCompression* xdct, 
    XilCis*                      xcis)
: XilDeviceCompression(xdct, xcis, 0, FRAMES_PER_BUFFER)
{

    isOKFlag = FALSE;
    if(! deviceCompressionValid()) {
        //  Couldn't create internal base XilDeviceCompression object
        XIL_ERROR(system_state, XIL_ERROR_SYSTEM, "di-278", FALSE);
        return;
    }

    if(initValues() == XIL_FAILURE) {
        return;
    }

    cbm->setSeekToStartFrameFlag(TRUE);
    compData.buf_manager = cbm;
    decompData.cbm = cbm;

    colorConverter = NULL;
    ditherTable    = NULL;

    isOKFlag = TRUE;
}

// Constructor / destructor
//

XilDeviceCompressionJpeg::~XilDeviceCompressionJpeg()
{
    //
    // Destroy the ImageFormat object(s).
    //
    if(inputType != outputType) {
        outputType->destroy();
    }

    inputType->destroy();

}
 
int
XilDeviceCompressionJpeg::initValues()
{
    XilImageFormat* t = 
    system_state->createXilImageFormat(0U, 0U, 0U, XIL_BYTE);

    if(t == NULL) {
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE,"di-1",TRUE);
        return XIL_FAILURE;
    }

    outputType = inputType = t;

    inputOutputType = FALSE;
    imageWidth = 0;
    imageHeight = 0;
    random_access = TRUE;

    return XIL_SUCCESS;
}

 
void 
XilDeviceCompressionJpeg::setRandomAccess(Xil_boolean value)
{ 
    random_access = value; 
}

//------------------------------------------------------------------------
//
//  Function:        XilDeviceCompressionJpeg::seek
//
//  Description:
//        Since Jpeg compression may need to decode the first frame,
//        and may not be able to skip over frames without reading their
//        headers,
//        we must seek to the first frame and then burn forward to the actual 
//        frame.
//        We seek to JPEG_FRAME_TYPE, knowing that the first frame of the
//        cis will match any frame type.
//        This doesn't solve the problem of seeking backwards in a cis
//        in which jpeg tables change.
//        
//  Parameters:
//        int framenumber:  desired frame number.
//      Xil_boolean history_update:  set if seek must also update decompress
//                                   history.
//        
//  Returns:
//        void
//        
//------------------------------------------------------------------------

void
XilDeviceCompressionJpeg::seek(int         framenumber, 
                               Xil_boolean history_update)
{
    int frames_to_burn;

#ifdef MAYBE
    if(decompData.notSeekable) {
        int current_frame_id = cbm->getRFrameId();
        if(framenumber < current_frame_id) {
            if(cbm->seek(0) < 0) {
                return;
            }
            burnFrames(framenumber);
        } else
        burnFrames(framenumber - current_frame_id);
    } else {
        frames_to_burn = cbm->seek(framenumber, decompData.seekFrameType);

        if(frames_to_burn > 0) {
            burnFrames(frames_to_burn);
        }
    }
#endif // MAYBE
    if(decompData.notSeekable && decompData.ignoreHistory == FALSE) {
        /*
        * Prevent backward seek
        */
        int current_frame_id = cbm->getRFrameId();
        if(framenumber < current_frame_id) {
            XIL_ERROR(system_state, XIL_ERROR_USER, "di-304", TRUE);
            random_access = FALSE;
            return;
        }
    }

    if(history_update == TRUE) {
        frames_to_burn = cbm->seek(framenumber, decompData.seekFrameType);
    } else {
        frames_to_burn = cbm->seek(framenumber, XIL_CIS_NO_BURN_TYPE);
    }

    if(frames_to_burn > 0) {
        burnFrames(frames_to_burn);
    }
}

int 
XilDeviceCompressionJpeg::findNextFrameBoundary()
{
    return decompData.findNextFrameBoundary();
}

//------------------------------------------------------------------------
//
//  Function:        XilDeviceCompressionJpeg::adjustStart(int framenumber)
//
//  Description:
//        Since Jpeg compression may need to decode the first frame,
//        and may not be able to skip over frames without reading their
//        headers,
//        we must seek to the desired start frame.  The seek will establish
//      the number of frames we must burn to keep the history intact,
//      so the seek type will be the JPEG_FRAME_TYPE.
//      This will protect against the situation where the read_frame has
//      a different position from the next_decompress_frame.  We must burn
//      any un-typed frames to record any H/Q table changes.  This is 
//      preferred to keeping these un-typed frames available in the cis,
//      since the burn is a comparatively cheap operation.
//        
//  Parameters:
//        int framenumber:  desired frame number.
//        
//  Returns:
//        XIL_SUCCESS or XIL_FAILURE
//        
//------------------------------------------------------------------------
int
XilDeviceCompressionJpeg::adjustStart(int framenumber)
{
    int frames_to_burn;
    int init_frame_id;

    // save current read frame
    init_frame_id = cbm->getRFrameId();

    // determine if there are any un-typed frames to get to new start frame
    frames_to_burn = cbm->seek(framenumber, JPEG_FRAME_TYPE);
    if(frames_to_burn <0) {
        // an error occurred
        return XIL_FAILURE;
    } else if(frames_to_burn>0) {
        // burn these frames to process any table changes
        burnFrames(frames_to_burn);
    }

    // update start of cis buffers
    cbm->adjustStart(framenumber);

    // restore original read frame if possible
    // NOTE:  the seek uses default=ANY_FRAME_TYPE since position known
    if(init_frame_id > framenumber) {
        cbm->seek(init_frame_id);
    }

    return(XIL_SUCCESS);
}


//
//  Decompressor Supporting Member Functions
//

void 
XilDeviceCompressionJpeg::burnFrames(int nframes)
{
    if(nframes > 0) {
        decompData.burnFrames(nframes);
    }
}

#define MAX_BITS_PER_PIXEL        26
#define MAX_BYTES_PER_FRAME_HEADER        2000
int 
XilDeviceCompressionJpeg::getMaxFrameSize()
{
    int max_bits_per_frame;

    max_bits_per_frame = MAX_BITS_PER_PIXEL * 
                         ((inputType->getWidth() + 7) & ~7) *
                         ((inputType->getHeight() + 7) & ~7) *
                         inputType->getNumBands();

    return ((max_bits_per_frame + 7) >> 3) + MAX_BYTES_PER_FRAME_HEADER;
}


void
XilDeviceCompressionJpeg::reset()
{
    if(inputType != outputType) {
        outputType->destroy();
    }
    inputType->destroy();

    initValues();
    compData.reset();
    decompData.reset();
    XilDeviceCompression::reset();
}

//
//  Routines to get references to the Compressor, Decompressor and
//  Attribute specific classes.
//
JpegCompressorData*    
XilDeviceCompressionJpeg::getJpegCompressorData(void)
{
    return &compData;
}

JpegDecompressorData*  
XilDeviceCompressionJpeg::getJpegDecompressorData(void)
{
    return &decompData;
}

//
//  Function to read header and fill in the header information --
//  specifically width and height
//
XilStatus 
XilDeviceCompressionJpeg::deriveOutputType()
{

    unsigned int w, h, nbands;
    XilDataType datatype;

    // input and output types are assumed to be the same
    if(inputOutputType == FALSE) {
        // type not initialized from bitstream
        if(!decompData.getOutputType(&w, &h, &nbands, &datatype)) {
            return XIL_FAILURE;
        }

        XilImageFormat* newtype = 
        system_state->createXilImageFormat(w, h, nbands, datatype);
        if(newtype == NULL) {
            XIL_ERROR(getCis()->getSystemState(),XIL_ERROR_RESOURCE,"di-1",TRUE);
            return XIL_FAILURE;
        }

        setInputType(newtype);  // sets outputType as a sideaffect
        newtype->destroy();
        inputOutputType = TRUE;
    }

    return XIL_SUCCESS;
}

void       
XilDeviceCompressionJpeg::setWidth(int w)                
{
    imageWidth = w;
}

void       
XilDeviceCompressionJpeg::setHeight(int h)                
{
    imageHeight = h;
}

//
// COMPRESSION Attribute Get/Set Methods
//

//  Abbreviated Format
//
void       
XilDeviceCompressionJpeg::setDataFormat(XilJpegCompressedDataFormat  format)
{
    if(format != INTERCHANGE && format != ABBREVIATED_FORMAT) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-62", TRUE);
    } else {
        compData.abbr_format = (format == ABBREVIATED_FORMAT);
    }

    compData.version++;
}

//
//  Encode Video
//
void       
XilDeviceCompressionJpeg::setEncodeVideo(Xil_boolean video)
{
    if(video != TRUE && video != FALSE) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-58", TRUE);
    } else {
        compData.encode_video = video;
    }

    compData.version++;
}

//
//  Encode Interleaved
//
void       
XilDeviceCompressionJpeg::setEncodeInterleaved(Xil_boolean inter)
{
    if(inter != TRUE && inter != FALSE) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-57", TRUE);
    } else {
        compData.encode_interleaved = inter;
    }   

    compData.version++;
}

//
//  Quantization Table
//
void       
XilDeviceCompressionJpeg::setQTable(XilJpegQTable& xil_qtable)
{

    int qtable_id = xil_qtable.table;

    if(qtable_id < 0 || qtable_id > MAX_JPEG_QUANT_INDEX) {

        // Jpeg bitstream error: invalid qtable identifier
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-91", TRUE);  
        return;
    }

    int found_bad_value = 0;
    /* XXX
    * In the future, the max value (set here to 255) should allow quantizers
    * with a precision of 16 bits (max would be 65535)
    */
    for(int i=0; i < 8; i++) {
        for(int j=0; j < 8; j++) {
            if(xil_qtable.value[i][j] < 1 || xil_qtable.value[i][j] > 255) {
                found_bad_value = 1;
            }
        }
    }

    if(found_bad_value) {
        // Jpeg bitstream error: invalid qtable contents
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-310", TRUE);  
        return;
    }


    compData.quantizer->Add_Table(xil_qtable.value , xil_qtable.table);

    if(compData.qflag) {
        compData.quantizer->Scale(compData.qvalue);
    }
    compData.version++;
}

//
//  Huffman Table
//
void       
XilDeviceCompressionJpeg::setHTable(XilJpegHTable& xil_htable)
{
    HTable* htable;

    if(xil_htable.type != 0 && xil_htable.type != 1) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-92", TRUE);
    } else if(xil_htable.table < 0 || xil_htable.table > 3) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-90", TRUE);
    } else {

        if(xil_htable.type == 0) {
            // type is DC, thus 16 elements
            htable = new HTable(16,16,0);

            if(htable==NULL) {      
                XIL_ERROR(system_state, XIL_ERROR_RESOURCE,"di-1",TRUE);      
                return;
            }        

            htable->Add_Code(16,(Huffman_Code*)xil_htable.value);
            compData.huffman_encoder->Add_Table(htable,
                                         (xil_htable.type<<4)|xil_htable.table);

        } else {
            // type is AC, thus 256 elements
            htable = new HTable(16,256,0);

            if(htable==NULL) {       
                XIL_ERROR(system_state, XIL_ERROR_RESOURCE,"di-1",TRUE);       
                return;
            }         

            htable->Add_Code(256,(Huffman_Code*)xil_htable.value);
            compData.huffman_encoder->Add_Table(htable,
                                         (xil_htable.type<<4)|xil_htable.table);
        }
    }
    compData.version++;
}

//
//  Associate Band to Quantization Table
//
void       
XilDeviceCompressionJpeg::setBandQTable(XilJpegBandQTable& xil_band_qtable)
{
    int band = xil_band_qtable.band;
    int qtable_id = xil_band_qtable.table;

    if(band < 0 || band >= JPEG_MAX_NUM_BANDS) {

        // Jpeg bitstream error: invalid band number 
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-93", TRUE); 

        return;
    }

    if(qtable_id < 0 || qtable_id > MAX_JPEG_QUANT_INDEX) {

        // Jpeg bitstream error: invalid qtable identifier
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-91", TRUE);  

        return;
    }

    compData.banddata[band].setQtableId(qtable_id);
    compData.bandinfo_changed = TRUE;
    compData.version++;
}

//
//  Associate Band to Huffman Table 
//
void       
XilDeviceCompressionJpeg::setBandHTable(XilJpegBandHTable& xil_band_htable)
{
    int band =  xil_band_htable.band;
    int table = xil_band_htable.table;
    int type =  xil_band_htable.type;

    if(band < 0 || band >= JPEG_MAX_NUM_BANDS) {

        // Jpeg bitstream error: invalid band number 
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-93", TRUE);   

        return;
    }

    if(table < 0 || table > MAX_JPEG_HTABLE_INDEX) {

        // Jpeg bitstream error: invalid htable identifier  
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-90", TRUE);    

        return;
    }

    if(type != DC_HTABLE_TYPE && type != AC_HTABLE_TYPE) {

        // Jpeg bitstream error: invalid htable type 
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-92", TRUE);      

        return;
    }

    /*
    * Table Ids are:  high order 4 bits == type (AC or DC),
    *                     low order 4 bits == table index.
    */
    if(type == DC_HTABLE_TYPE) {
        compData.banddata[band].setDcHtableId((type<<4)|table);
    } else {
        compData.banddata[band].setAcHtableId((type<<4)|table);
    }

    compData.bandinfo_changed = TRUE;
    compData.version++;
}

//
//  Optimal Huffman Encoding
//
void       
XilDeviceCompressionJpeg::setOptHTables(Xil_boolean opt_ht)
{
    if(opt_ht != TRUE && opt_ht != FALSE) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-61", TRUE);
    } else if(compData.use_optimal_htables != opt_ht) {
        compData.use_optimal_htables = opt_ht;
        if(!opt_ht) {
            compData.useDefaultHTables();
        }
    }
    compData.version++;
}

//
//  Temporal Filtering of Images
//
void       
XilDeviceCompressionJpeg::setTemporalFilter(Xil_boolean flag)
{
    if(flag != TRUE && flag != FALSE) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-60", TRUE);
    } else
    compData.filter_image = flag;
    compData.version++;
}
int       
XilDeviceCompressionJpeg::getTemporalFilter()
{
    return compData.filter_image;
}

//
//  Compression Quality
//
//        Legal values are 1-100 inclusive.
//        We store these as 'qvalue' which has a reverse ordering with respect
//        to the quality value, e.g. 100 is the highest quality, but is the
//        lowest 'qvalue': 1.  ('qvalue' corresponds to CCUBE Q value)
//
void       
XilDeviceCompressionJpeg::setCompressionQuality(int value)
{
    if(value < 1 || value > 100) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-59", TRUE);
    } else {
        if(value != 50) {
            value = 101 - value;
        }

        compData.qvalue = value;
        compData.qflag = TRUE;

        compData.quantizer->Scale(value);
    }
    compData.version++;
}

//
//  Bytes in most recently compressed frame
//
int       
XilDeviceCompressionJpeg::getBytesPerFrame()
{
    return compData.bytes_per_frame;
}

//
//  DECOMPRESSION
//

//
//  Decompression Quality
//
void 
XilDeviceCompressionJpeg::setIgnoreHistory(Xil_boolean flag)
{
    if(flag != TRUE && flag != FALSE) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-318", TRUE);
    } else {
        decompData.ignoreHistory = flag;
        if(flag == TRUE) {
            /*
            * Set seek frame type to JPEG_FRAME_TYPE so that we never
            * skip over any unprocessed frames, so that we never fail to
            * notice that a cis is not seekable
            */
            decompData.seekFrameType = JPEG_FRAME_TYPE;
            decompData.historyIgnored = TRUE;
            random_access = TRUE;
        } else {
            /*
            * If the attribute was ever set TRUE and this cis is not
            * seekable, then set the search frame type to a value that
            * is not in the cis (so that we will not skip frames when
             * we skip forward).  Otherwise, just go back to the usual
            * search frame type of JPEG_FRAME_TYPE, which prevents skipping
            * forward through unprocessed frames, but allows skipping
            * through processed frames, and allows direct seeking backward
            * to a frame.  (If the cis is not seekable, we will never
             * exectute a backward seek when this attribute is FALSE)
            */
            if(decompData.historyIgnored && decompData.notSeekable) {
                decompData.seekFrameType = JPEG_FRAME_TYPE+1;
            } else {
                decompData.seekFrameType = JPEG_FRAME_TYPE;
            }

            if(decompData.notSeekable) {
                random_access = FALSE;
            }
        }
    }
    decompData.version++;
}

//
//  Decompression Quality
//
void 
XilDeviceCompressionJpeg::setDecompressionQuality(int value)
{
    int kount;

    if(value < 1 || value > 100) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-56", TRUE);
    } else {
        // value ranges from 1-100, with 100 meaning highest decompression quailty
        //  kount ranges from 0-63, with 63 meaning highest decompression quailty
        if(value == 100) {
            kount = 63;
        } else  {
            kount = (int) ((float)(value-1) * ((float)63.0/(float)99.0));
        }

        decompData.setmaxkount(kount);
    }
    decompData.version++;
}

inline Xil_boolean 
OkCisType(XilDeviceCompressionJpeg* dc, 
          unsigned int              w,
          unsigned int              h,
          unsigned int              nbands)
{
    XilImageFormat* cis_outtype = dc->getOutputType();
    unsigned int    width_cis   = cis_outtype->getWidth();
    unsigned int    height_cis  = cis_outtype->getHeight();
    unsigned int    nbands_cis  = cis_outtype->getNumBands();

    // Compare height/width/nbands of dest_image to compressed image
    return (width_cis  == w && 
           height_cis == h && 
           nbands_cis == nbands);
}


//------------------------------------------------------------------------
//
//  Function:output_header
//
//  Description:
//
//    Outputs header information (see comments in code)
//
//  Parameters:
//
//    SingleBuffer* buffer:       pointer to File Output object
//    int width:                  width of image
//    int height:                 heigth of image
//    int bands:                  number of bands in image
//    JpegBandInfo* bandinfo:     pointer to JpegBandInfo object containing
//                                YUV table associations (etc)
//
//------------------------------------------------------------------------

void 
XilDeviceCompressionJpeg::output_header(SingleBuffer* buffer, 
                                        int width, 
                                        int height, 
                                        int bands, 
                                        int format, 
                                        JpegBandInfo* bandinfo)
{
    int i;
    // Frame header marker segment length = 8 + 3*nbands bytes
    int fhlen = 8 + bands + 2 * bands;

    buffer->addByte(MARKER);
    buffer->addByte(SOF(format));//  baseline format
    buffer->addShort(fhlen);   //  frame header length
    buffer->addByte(8);   //  baseline percision
    buffer->addShort(height);//  Number of lines in image
    buffer->addShort(width);   //  Number of pixels in line
    buffer->addByte(bands);//  Number of bands per pixel 

    for(i = 0; i < bands; i++) {
        buffer->addByte(bandinfo[i].getId());
        buffer->addByte((bandinfo[i].getH() << 4) + bandinfo[i].getV());
        buffer->addByte(bandinfo[i].getQtableId());
    }
}

//------------------------------------------------------------------------
//
//  Function:output_scan_header
//
//  Description:
//
//    Outputs scan information (see comments in code)
//
//  Parameters:
//
//      SingleBuffer* buffer:   pointer to File Output object
//      int bands:              number of bands in image
//      JpegBandInfo* bandinfo: pointer to JpegBandInfo object containing
//                               band/table associations (etc)
//
//------------------------------------------------------------------------

void 
XilDeviceCompressionJpeg::output_scan_header(SingleBuffer* buffer, 
                                             int bands, 
                                             JpegBandInfo* bandinfo)
{
    int i;
    // Scan header marker segment length = 6 + 2*nbands bytes
    int shlen = 6 + 2 * bands;

    // start of scan marker
    buffer->addByte(MARKER);
    buffer->addByte(SOS);

    // scan header length  
    buffer->addShort(shlen);

    // Number of bands in scan
    buffer->addByte(bands);

    // Output band identifiers and huffman table identifiers
    for(i = 0; i < bands; i++) {
        buffer->addByte(bandinfo[i].getId());
        buffer->addByte((bandinfo[i].getDcHtableId() << 4)
        + (0xf & bandinfo[i].getAcHtableId()));
    }

    // first spectral component  
    buffer->addByte(0);

    //  last spectral component
    buffer->addByte(63);

    //  successive approximation data 
    buffer->addByte(0);
}

//------------------------------------------------------------------------
//
//  Function:output_scan_header_for_band
//
//  Description:
//
//    Outputs scan information (see comments in code)
//
//  Parameters:
//
//      SingleBuffer* buffer:   pointer to File Output object
//      int band:               band to output header for
//      JpegBandInfo* bandinfo: pointer to JpegBandInfo object containing
//                              band/table associations (etc)
//
//------------------------------------------------------------------------

void 
XilDeviceCompressionJpeg::output_scan_header_for_band(SingleBuffer* buffer, 
                                                      int band, 
                                                      JpegBandInfo* bandinfo)
{
    // Scan header marker segment length = 6 bytes + 2 bytes of band info
    int shlen = 6 + 2;

    // start of scan marker
    buffer->addByte(MARKER);
    buffer->addByte(SOS);

    // scan header length  
    buffer->addShort(shlen);

    // Number of bands in scan
    buffer->addByte(1);

    buffer->addByte(bandinfo[band].getId());
    buffer->addByte((bandinfo[band].getDcHtableId() << 4)
    + (0xf & bandinfo[band].getAcHtableId()));


    // first spectral component  
    buffer->addByte(0);

    //  last spectral component
    buffer->addByte(63);

    //  successive approximation data 
    buffer->addByte(0);
}

//------------------------------------------------------------------------
//
//  Function:output_trailer
//
//  Description:
//
//   Outputs trailer (end of image) marker EOI
//
//  Parameters:
//
//   SingleBuffer* buffer:       pointer to File Output object
//
//------------------------------------------------------------------------

void 
XilDeviceCompressionJpeg::output_trailer(SingleBuffer* buffer)
{
    buffer->addByte(MARKER);
    buffer->addByte(EOI);
}




//
// Function to retrieve or create dither tables.
// This is located in the DeviceCompression because
// different CIS'es may be using different dither matrices.
// So each CIS must have its own table.
//
XiliOrderedDitherLut*
XilDeviceCompressionJpeg::getDitherTable(XilLookupColorcube* cmap,
                                         XilDitherMask*      dmask,
                                         float*              scale,
                                         float*              offset)
{
    //
    // We can potentially use the existing table.
    // But first, check if the cmap and dmask versions have changed.
    // If so, then we need to create a new table.
    // This is also true for the first call.
    // Use a mutex lock to serialize access here.
    //
    //
    mutex.lock();
    if(ditherTable == NULL                        ||
       !cmap->isSameAs(&current_cmap_version)     ||
       !dmask->isSameAs(&current_dmask_version)   ||
       scale[0]  != current_scale[0]              ||
       scale[1]  != current_scale[1]              ||
       scale[2]  != current_scale[2]              ||
       offset[0] != current_offset[0]             ||
       offset[1] != current_offset[1]             ||
       offset[2] != current_offset[2] ) {

        ditherTable = new XiliOrderedDitherLut(cmap, dmask, 
                                               scale, offset, 128);
        if(! ditherTable->isOK()) {
            mutex.unlock();
            return NULL;
        }

        //
        // Record new version info
        //
        cmap->getVersion(&current_cmap_version);
        dmask->getVersion(&current_dmask_version);

        //
        // Record new scale, offset info
        //
        current_scale[0]  = scale[0];
        current_scale[1]  = scale[1];
        current_scale[2]  = scale[2];
        current_offset[0] = offset[0];
        current_offset[1] = offset[1];
        current_offset[2] = offset[2];
    }

    mutex.unlock();

    return ditherTable;
}

