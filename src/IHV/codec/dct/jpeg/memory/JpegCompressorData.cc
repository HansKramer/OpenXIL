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
//  File:       JpegCompressorData.cc
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:14:27, 03/10/00
//
//  Description:
//
//    Contains constructor for JpegCompressorData class.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)JpegCompressorData.cc	1.5\t00/03/10  "


#include "XilDeviceCompressionJpeg.hh"


// this routine should ONLY reset attributes

//------------------------------------------------------------------------
//
//  Function:JpegCompressorData::reset()
//  Created:92/10/ 9
//
//  Description:
//
//Resets jpeg compression attributes and realoads default tables.
//
//------------------------------------------------------------------------

void JpegCompressorData::reset()
{
    int i;

    version++;
    abbr_format = TRUE;
    qvalue = 50;
    encode_video = FALSE;
    encode_interleaved = TRUE;
    use_optimal_htables = FALSE;

    qflag = FALSE;
    bandinfo_changed = FALSE;
    using_411_bandinfo = FALSE;
    bytes_per_frame = 0;

    filter_image = FALSE;
    delete [] previous_image;
    previous_image = NULL;

    delete [] filteredBuf;
    filteredBuf = NULL;

    for(i=0; i<JPEG_MAX_NUM_BANDS; i++) {
        banddata[i].setId(i);
        banddata[i].setH(1);
        banddata[i].setV(1);
    }

    useDefaultBandQTables();
    useDefaultBandHTables();
    useDefaultQTables();
    useDefaultHTables();
}

//------------------------------------------------------------------------
//
//  Function:JpegCompressorData::JpegCompressorData()
//  Created:92/10/ 9
//
//  Description:
//
//Constructor
//
//------------------------------------------------------------------------

JpegCompressorData::JpegCompressorData()
{

    version = 0;
    quantizer = 0;
    huffman_encoder = 0;
    gob_getter = 0;
    gob = 0;
    allocSuccess = 0;
    previous_image = NULL;
    filteredBuf = NULL;

    banddata = new JpegBandInfo[JPEG_MAX_NUM_BANDS];

    if(banddata==NULL) {
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
        return;
    }

    quantizer = new Jpeg_Quantizer(4,BIT_PRECISION_8);

    if(quantizer==NULL) {
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE); 
        return;
    }

    quantizer->set_Marker(DQT);

    huffman_encoder = new JpegOptHuffmanEncoder(8,JPEG_MAX_NUM_BANDS);

    if(huffman_encoder==NULL) { 
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);  
        return;
    }

    huffman_encoder->set_Marker(DHT);

    gob_getter = new GobGetter();

    if(gob_getter==NULL) {  
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);  
        return;
    } 

    // no more than 6 blocks can be used to encode an image
    // (use 6 blocks with video format, otherwise #blocks == 1..4).
    gob = new int[8*8*6];

    if(gob==NULL) {   
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);   
        return;
    }    

    buf_manager = NULL;
    buffer = NULL;
    nbands = 0;
    if(!createDefaultHTables()) {
        return;
    }

    allocSuccess = 1;


    reset();

}

//------------------------------------------------------------------------
//
//  Function:JpegCompressorData::~JpegCompressorData()
//  Created:92/10/ 9
//
//  Description:
//
//    Deconstructor
//
//------------------------------------------------------------------------

JpegCompressorData::~JpegCompressorData()
{
    delete quantizer;
    delete huffman_encoder;
    delete gob_getter;
    delete [] banddata;
    delete [] gob;

    delete [] previous_image;
    delete [] filteredBuf;

    for(int i=0; i<4; i++) {
        delete default_htables[i];
    }
}

//------------------------------------------------------------------------
//
//  Function:JpegCompressorData::useDefaultQTables()
//  Created:92/10/ 9
//
//  Description:
//
//Loads default quantization tables
//
//------------------------------------------------------------------------

void JpegCompressorData::useDefaultQTables()
{
    //------------------------------------
    // Add Default QTables to Quantizer
    //------------------------------------

    quantizer->Add_Table(XilDeviceCompressionJpeg::qtable0,0);
    quantizer->Add_Table(XilDeviceCompressionJpeg::qtable1,1);

}

#define JPEG_DC_LUMA   0x00

#define JPEG_DC_CHROMA 0x01

#define JPEG_AC_LUMA   0x10

#define JPEG_AC_CHROMA 0x11


//------------------------------------------------------------------------
//
//  Function:JpegCompressorData::createDefaultHTables()
//  Created:92/10/ 9
//
//  Description:
//
//Creates default huffman tables
//
//------------------------------------------------------------------------

int JpegCompressorData::createDefaultHTables()
{
    //------------------------------------
    // Create Huffman Tables
    //------------------------------------

    default_htables[0] = new HTable(16,16);
    if(default_htables[0]==NULL) {   
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);    
        return 0;
    }     

    default_htables[1] = new HTable(16,16);
    if(default_htables[1]==NULL) {    
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);    
        return 0;
    }      

    default_htables[2] = new HTable(16,256);
    if(default_htables[2]==NULL) {     
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);     
        return 0;
    }       

    default_htables[3] = new HTable(16,256);  
    if(default_htables[3]==NULL) {      
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);      
        return 0;
    }        

    default_htables[0]->Add_Code(16,XilDeviceCompressionJpeg::dc_table0);
    default_htables[1]->Add_Code(16,XilDeviceCompressionJpeg::dc_table1);
    default_htables[2]->Add_Code(256,XilDeviceCompressionJpeg::ac_table0);
    default_htables[3]->Add_Code(256,XilDeviceCompressionJpeg::ac_table1);  

    useDefaultHTables();

    return 1;
}

//------------------------------------------------------------------------
//
//  Function:JpegCompressorData::useDefaultBandQTables()
//  Created:92/10/ 9
//
//  Description:
//
//Sets up default band / qtable relationships.
//
//------------------------------------------------------------------------

void JpegCompressorData::useDefaultBandQTables()
{
    banddata[0].setQtableId(0);

    for(int i=1; i<JPEG_MAX_NUM_BANDS; i++) {
        banddata[i].setQtableId(1);
    }

    bandinfo_changed = TRUE;
}

//------------------------------------------------------------------------
//
//  Function:JpegCompressorData::useDefaultBandHTables()
//  Created:92/10/ 9
//
//  Description:
//
//Sets up default band / htable relationships.
//
//------------------------------------------------------------------------

void JpegCompressorData::useDefaultBandHTables()
{
    banddata[0].setDcHtableId(JPEG_DC_LUMA);
    banddata[0].setAcHtableId(JPEG_AC_LUMA);

    for(int i=1; i<JPEG_MAX_NUM_BANDS; i++) {
        banddata[i].setDcHtableId(JPEG_DC_CHROMA);
        banddata[i].setAcHtableId(JPEG_AC_CHROMA);
    }

    bandinfo_changed = TRUE;
}

//------------------------------------------------------------------------
//
//  Function:JpegCompressorData::useDefaultHTables()
//  Created:92/10/ 9
//
//  Description:
//
//Load default htables
//
//------------------------------------------------------------------------

void JpegCompressorData::useDefaultHTables()
{
    huffman_encoder->Add_Table(default_htables[0],JPEG_DC_LUMA);
    huffman_encoder->Add_Table(default_htables[1],JPEG_DC_CHROMA);
    huffman_encoder->Add_Table(default_htables[2],JPEG_AC_LUMA);
    huffman_encoder->Add_Table(default_htables[3],JPEG_AC_CHROMA);
}


