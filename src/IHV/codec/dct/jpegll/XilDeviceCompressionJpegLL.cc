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
//  File:       XilDeviceCompressionJpegLL.cc
//  Project:    XIL
//  Revision:   1.12
//  Last Mod:   10:15:05, 03/10/00
//
//  Description:
//
//    DeviceCompression for the Jpeg Lossless Codec
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDeviceCompressionJpegLL.cc	1.12\t00/03/10  "

#include "XilDeviceCompressionJpegLL.hh"
#include "XiliUtils.hh"

//
// If you have a compression where the number of bytes per frame
// varies a great deal, this should be large (to minimize wasted space).
// However, if MaxFrameSize is a good predictor of the actual space
// used, then this can be small.
//

XilDeviceCompressionJpegLL::XilDeviceCompressionJpegLL(
    XilDeviceManagerCompression* xdct,
    XilCis*                      incis)
 : XilDeviceCompression(xdct, incis, 0, FRAMES_PER_BUFFER)
{
    isOKFlag = FALSE;

    if(! deviceCompressionValid()) {
        //  Couldn't create internal base XilDeviceCompression object
        XIL_ERROR(system_state, XIL_ERROR_SYSTEM, "di-278", FALSE);
        return;
    }

    precision        = 8;
    version          = 0;
    bandinfo_changed = FALSE;
    banddata         = NULL;
    huffman_encoder  = NULL;
    parser           = NULL;
    decoder          = NULL;

    //
    // Create a JpegParser object
    //
    parser = new JpegParser(JPEGLL);
    if(parser == NULL) {
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }
    parser->reset();
    parser->useBufferManager(cbm);

    //
    // Create an array of JpegBandInfo objects
    // TODO: Do we really need to allow for the max # of bands (256)?
    //
    banddata = new JpegBandInfo[JPEG_MAX_NUM_BANDS];
    if(banddata == NULL) {
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE,"di-1",TRUE);
        return ;
    }
    for(int i=0; i<JPEG_MAX_NUM_BANDS; i++) {
        banddata[i].setId(i);
        banddata[i].setH(1);
        banddata[i].setV(1);
    }

    //
    // TODO:
    //   See if we can hold off instantiating these until
    //   either a compress or decompress is done, since
    //   we may never need both of them.

    //
    // Instantiate a JpegLL_Huffman_Encoder object
    //
    huffman_encoder = new JpegLL_Huffman_Encoder(8, 0);
    if(! huffman_encoder->isOK()) {
        XIL_ERROR(system_state,XIL_ERROR_RESOURCE,"di-280",TRUE);
        return ;
    }
    huffman_encoder->set_Marker(DHT);

    sbuffer      = NULL;

    for(i=0; i<MAX_NUM_TABLES; i++) {
        default_htables[i] = NULL;
    }

    if(createDefaultHTables() == XIL_FAILURE) {
        XIL_ERROR(system_state,XIL_ERROR_RESOURCE,"di-1",TRUE);
        return;
    }

    if(initValues() == XIL_FAILURE) {
        return;
    }


    isOKFlag = TRUE;  
}

int 
XilDeviceCompressionJpegLL::initValues()
{
    //
    // Set attributes to their default values
    //
    setAbbrFormat(TRUE);
    setEncodeInterleaved(TRUE);
    setOptHTables(FALSE);
    useDefaultHTables();
    useDefaultBandHTables();

    XilJpegLLBandPtTransform pt;
    XilJpegLLBandSelector sel;

    pt.PtTransform = 0;
    sel.selector = ONE_D1;
    for(int i=0; i<JPEG_MAX_NUM_BANDS; i++) {
        pt.band = i;
        setBandPtTransform(pt);
        sel.band = i;
        setBandSelector(sel);
    }

    XilImageFormat* t = 
        system_state->createXilImageFormat(0U, 0U, 0U, XIL_BYTE);
    if(t == NULL) {
        XIL_ERROR(system_state, XIL_ERROR_RESOURCE,"di-1",TRUE);
        return XIL_FAILURE;
    }

    outputType      = inputType = t;
    inputOutputType = NULL;
    return XIL_SUCCESS;
}  

XilDeviceCompressionJpegLL::~XilDeviceCompressionJpegLL()
{
    delete huffman_encoder;

    for(int i=0; i<4; i++) {
        delete default_htables[i];
        default_htables[i] = NULL;
    }

    delete []banddata;
    banddata = NULL;

    for(i=0; i<MAX_DECODER_TABLES; i++) {
        if(parser->header.decoder[i]) {
            parser->free_decode_table(parser->header.decoder[i]);
        }
    }


    if(parser->header.band_data != NULL) {
        delete(parser->header.band_data);
        parser->header.band_data = NULL;
    }

    delete parser;

    //
    // Destroy the ImageFormat object(s).
    //
    if(inputType != outputType) {
        outputType->destroy();
    }

    inputType->destroy();

}

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



//
//  MaxFrameSize is assumed to be the max number of bytes that
//  can be obtained for a single frame.  For a short or byte image that
//  would be: 
//
//    width * height * nbands * max_bytes_per_pixel + table_size
//
//  where max_bytes_per_pixel is the sum of the max Huffman code
//  length plus the max code length (the prediction) (all in bytes)
//  plus the table length in bytes of the Huffman tables.
//

int 
XilDeviceCompressionJpegLL::getMaxFrameSize()
{   
    if(inputType->getDataType() == XIL_SHORT) {
        return 
        (inputType->getWidth() * inputType->getHeight() *
         inputType->getNumBands() * (MAX_HUF_CODE_LENGTH + sizeof(Xil_signed16)) +
         HUFFMAN_TABLE_SIZE) ;
    } else {
        return 
        (inputType->getWidth() * inputType->getHeight() *
         inputType->getNumBands() * (MAX_HUF_CODE_LENGTH + sizeof(Xil_unsigned8)) +
         HUFFMAN_TABLE_SIZE) ;
    }
}

void 
XilDeviceCompressionJpegLL::burnFrames(int nframes)
{
    if(nframes > 0) {
        parser->burnFrames(nframes);
    }
}

XilStatus 
XilDeviceCompressionJpegLL::deriveOutputType()
{
    unsigned int w, h, nbands;
    XilDataType  datatype;

    //
    // Input and output types are assumed to be the same
    //
    if(inputOutputType == NULL) {
        if(parser->getOutputType(&w, &h, &nbands, &datatype)) {
            inputOutputType = 
                system_state->createXilImageFormat(w, h, nbands, datatype);
        } else {
            return XIL_FAILURE;
        }
    }

    //
    // Sets outputType as a sideaffect
    //
    setInputType(inputOutputType);
    return XIL_SUCCESS;

}


void 
XilDeviceCompressionJpegLL::reset()
{
    if(inputType != outputType) {
        outputType->destroy();
    }
    inputType->destroy();
    initValues();
    version = 0;

    //
    // Parser is only instantiated for a decompress op.
    // Since we might only be doing compression, check if
    // it exists first.
    //
    if(parser != NULL) {
        parser->reset();
    }

    XilDeviceCompression::reset();
}


void 
XilDeviceCompressionJpegLL::setAbbrFormat(int abbr)
{
    if(abbr == INTERCHANGE || abbr == ABBREVIATED_FORMAT) {
        abbr_format = abbr;
    } else {
        XIL_ERROR(system_state, XIL_ERROR_USER,"di-62",TRUE);
    }
    version++;
}

void 
XilDeviceCompressionJpegLL::setEncodeInterleaved(int inter)
{
    if(inter == TRUE || inter == FALSE) {
        encode_interleaved = inter ;
    } else {
        XIL_ERROR(system_state, XIL_ERROR_USER,"di-57",TRUE);
    }
}

void 
XilDeviceCompressionJpegLL::setOptHTables(int opt_ht)
{
    if(opt_ht == TRUE || opt_ht == FALSE) {
        use_optimal_htables = opt_ht;
    } else {
        XIL_ERROR(system_state, XIL_ERROR_USER,"di-61",TRUE);
    }
}

void 
XilDeviceCompressionJpegLL::useDefaultHTables()
{
    huffman_encoder->Add_Table(default_htables[0],0);
    huffman_encoder->Add_Table(default_htables[1],1);
    huffman_encoder->Add_Table(default_htables[2],2);
    huffman_encoder->Add_Table(default_htables[3],3);
}

int 
XilDeviceCompressionJpegLL:: createDefaultHTables()
{
    default_htables[0] = new HTable(16,17);
    default_htables[1] = new HTable(16,17);
    default_htables[2] = new HTable(16,17);
    default_htables[3] = new HTable(16,17);


    if((default_htables[0] == NULL) || (default_htables[1] == NULL) ||
       (default_htables[2] == NULL) || (default_htables[3] == NULL) ) { 
        return XIL_FAILURE;
    } else {
        default_htables[0]->Add_Code(17,jpegll_table0);
        default_htables[1]->Add_Code(17,jpegll_table1);
        default_htables[2]->Add_Code(17,jpegll_table2);
        default_htables[3]->Add_Code(17,jpegll_table3);
        return XIL_SUCCESS;
    }
}

void 
XilDeviceCompressionJpegLL::useDefaultBandHTables()
{
    banddata[0].setHtableId(0);

    for(int i=1; i<JPEG_MAX_NUM_BANDS; i++) {
        banddata[i].setHtableId(1);
    }

    bandinfo_changed = TRUE;
    version++;
}

void 
XilDeviceCompressionJpegLL::setHTable(XilJpegHTable& xil_htable)
{
    if(xil_htable.type == 0) {
        // type is lossless, thus 17 elements
        HTable* htable = new HTable(16,17,0);

        if(htable == NULL) { 
            XIL_ERROR(system_state, XIL_ERROR_RESOURCE,"di-1",TRUE);  
            return;
        }

        htable->Add_Code(17,(Huffman_Code*)xil_htable.value);
        if(xil_htable.table < 4 && xil_htable.table >= 0) {
            huffman_encoder->Add_Table(htable,xil_htable.table);
        } else {
            // table index out of range
            XIL_ERROR(system_state, XIL_ERROR_USER, "di-94", TRUE);
            return;
        }

    } else {
        // type non-zero, unknown
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-92", TRUE);
        return;
    }
    version++;
}

void 
XilDeviceCompressionJpegLL::setBandHTable(XilJpegBandHTable& xil_band_htable)
{
    int band =  xil_band_htable.band;
    int table = xil_band_htable.table;
    int type =  xil_band_htable.type;

    if(band < 0 || band > JPEG_MAX_NUM_BANDS) {
        XIL_ERROR(system_state,XIL_ERROR_USER, "di-93", TRUE);
        return;
    }


    if(table < 0 || table > 4) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-94", TRUE);
        return;
    }

    if(type != 0) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-92", TRUE);
        return;
    }

    banddata[band].setHtableId(table);

    bandinfo_changed = TRUE;
    version++;
}

void 
XilDeviceCompressionJpegLL::setBandSelector(XilJpegLLBandSelector& bsel)
{
    int band;
    if(bsel.band < JPEG_MAX_NUM_BANDS && bsel.band >= 0) {
        band = bsel.band;
    } else {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-93", TRUE);
        return;
    }  

    if((bsel.selector < ONE_D1) || (bsel.selector > TWO_D4)) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-70", TRUE);
        return;
    }

    selector[band] = bsel.selector;
    version++;
}

void 
XilDeviceCompressionJpegLL::setBandPtTransform(XilJpegLLBandPtTransform& bpt)
{
    int band;
    if(bpt.band < JPEG_MAX_NUM_BANDS && bpt.band >= 0) {
        band = bpt.band;
    } else {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-93", TRUE);
        return;
    }
    if((bpt.PtTransform < 0) || (bpt.PtTransform > 15)) {
        XIL_ERROR(system_state, XIL_ERROR_USER, "di-71", TRUE);
        return;
    }
    pt_transform[band] = bpt.PtTransform;
    version++;
}

int 
XilDeviceCompressionJpegLL::validateBandTableUsage(unsigned int nbands)
{
    huffman_encoder->resetTableUsage();

    // parse through bandinfo resetting table usage
    for(int i=0; i<nbands; i++) {

        int loaded_flg = huffman_encoder->tableLoaded(banddata[i].getHtableId());
        if(loaded_flg) {
            huffman_encoder->usingTable(banddata[i].getHtableId());
        } else {
            XIL_ERROR(system_state, XIL_ERROR_USER,"di-88",TRUE);  
            return XIL_FAILURE;
        }

    }

    return XIL_SUCCESS;  
}

void 
XilDeviceCompressionJpegLL::resetDecompress() {
    parser->header.width     = 0;
    parser->header.height    = 0;
    parser->header.bands     = 0;
    parser->header.scanbands = 0;
    parser->rdptr            = NULL ;
}
 

void
XilDeviceCompressionJpegLL::initDecompress()
{
    already_readtoscan = FALSE;
}
 
 
 
 
//------------------------------------------------------------------------
//        
//    Outputs the frame header (SOF)
//        
//------------------------------------------------------------------------

void 
XilDeviceCompressionJpegLL::output_frame_header(CompressInfo* ci)
{
    //
    // Frame header length
    //
    int fhlen = 8 + 3 * ci->image_nbands;

    sbuffer->addByte(MARKER);
    sbuffer->addByte(SOF(JPEG_LOSSLESS));
    sbuffer->addShort(fhlen);
    sbuffer->addByte(8 * xili_sizeof(ci->image_datatype));
    sbuffer->addShort(ci->image_box_height);
    sbuffer->addShort(ci->image_box_width);
    sbuffer->addByte(ci->image_nbands);

    for(int i=0; i<ci->image_nbands; i++) {
        sbuffer->addByte(banddata[i].getId());
        sbuffer->addByte((banddata[i].getH() << 4) + banddata[i].getV());
        //
        // No quantization table for lossless
        //
        sbuffer->addByte(0);
    }
}

//------------------------------------------------------------------------
//
// Output scan header for the mult-band interleaved case
//
//------------------------------------------------------------------------

void 
XilDeviceCompressionJpegLL::output_scan_header(unsigned int start_band, 
                                               unsigned int nbands)
{
    //
    // Scan header length
    //
    int shlen = 6 + 2 * nbands;

    // start of scan marker
    sbuffer->addByte(MARKER);
    sbuffer->addByte(SOS);

    // scan header length          
    sbuffer->addShort(shlen);

    // Number of bands in scan        
    sbuffer->addByte(nbands);                

    // Output band identifiers and huffman table identifiers
    for(int i=start_band; i<start_band+nbands; i++) {
        sbuffer->addByte(banddata[i].getId());
        sbuffer->addByte(banddata[i].getHtableId() << 4);
    }

    // Output predictor selection
    // (Must all be the same for interleaved)
    sbuffer->addByte(selector[start_band]); 

    //  last spectral component (used only in DCT jpeg)
    sbuffer->addByte(0);

    //
    //  Zero in 4 MSBs (for Jpeg DCT Successive Approximation)
    //  Point transform  in 4 LSBs
    //
    sbuffer->addByte(pt_transform[start_band]);                
}

//------------------------------------------------------------------------
//        
//   Outputs trailer (end of image) marker EOI
//        
//------------------------------------------------------------------------
void 
XilDeviceCompressionJpegLL::output_trailer()
{
    sbuffer->addByte(MARKER);
    sbuffer->addByte(EOI);
}
