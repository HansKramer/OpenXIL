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
//  File:       Mpeg1DecompressorData.cc
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:14:52, 03/10/00
//
//  Description:
//
//    Implementation of Mpeg1DecompressorData Class
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Mpeg1DecompressorData.cc	1.6\t00/03/10  "

#include "Mpeg1DecompressorData.hh"
#include "XiliUtils.hh"


Mpeg1DecompressorData::Mpeg1DecompressorData()
 : Copy0(), Copy1(), CopyB()
{
    //
    // TODO: Check allocs of any new parent objects
    //
    if(!(Mpeg1Decoder::allocOk())) {
        isok = 0;
        return;
    }  

    compressionPattern.pattern = NULL;

    for(int i=1; i<=7; i++) {
        int k = 1 << (i-1);
        modulo[i][33] = k-1;
        modulo[i][34] = i-1;
        modulo[i][35] = k<<4;
        modulo[i][36] = k<<5;
        for(int j=0;j<33;j++) {
            modulo[i][j] = (j-16)*k;
        }
    }

    //
    // Set up the initial reference frame ptrs
    // These get set as needed whe the frames are decoded
    //
    last = NULL;
    next = NULL;
    bbbb = NULL;

    version = 0;
    resetDecompressorData();

    isok = 1;
}

Mpeg1DecompressorData::~Mpeg1DecompressorData()
{
    resetDecompressorData();
}

void Mpeg1DecompressorData::initIntraTable()
{
    //
    // Initialize quantization tables to default values
    //
    for(int i=0; i<64; i++) {
        quantintra[i] = quantIntraInit[i];
    }
}

void Mpeg1DecompressorData::initNonintraTable()
{
    //
    // Initialize quantization tables to default values
    //
    for(int i=0; i<64; i++) {
        quantnonin[i] = 16;
    }
}

void Mpeg1DecompressorData::deleteFrames()
{
}

void Mpeg1DecompressorData::resetDecompressorData()
{
    decoderValidCnt   = 0;
    validDitherState  = 0;
    rdptr             = NULL;

    version++;
    next_scan_id      = 0; 
    curr_display_id   = 0; 
    prev_nonbframe_id = -1; 
    group_base        = 0;
    subgroup_id       = -1;

    deleteFrames();

    xili_memset(current, 0, sizeof(current));


    aspectRatioC                = 0;
    pictureRateC                = 0;
    timeCodeC                   = 0x80000000;
    if(compressionPattern.pattern != NULL) {
        // Allocated by strdup - must use free
        free(compressionPattern.pattern);
    }
    compressionPattern.pattern  = NULL;
    bitsPerSecond               = 2880;
    intraQptr                   = 0;
    nonIntraQptr                = 0;
    slicesPerPicture            = 0;
    insertSequenceEnd           = 0;

    //
    // Reset the Reference Frame Buffers to their
    // original state, but don't delete them.
    // These will exist for the life of the CIS.
    //
    Copy0.reset(); 
    Copy1.reset(); 
    CopyB.reset(); 
    last = NULL;
    next = NULL;
    bbbb = NULL;

    //
    // These are a pair.
    // bframequality controls B frame decoding quality/speed. (value 2 or 7)
    // decompressionQuality  is the external attribute (value 1-100)
    //
    bframequality        = 7;
    decompressionQuality = 100;

    frametypeC = 0;


    //
    // This is not necessary.  There are no loaded default tables in mpeg.
    // This is here only to protect against a bad bitstream which does not
    // load any tables.
    //
    initIntraTable();
    initNonintraTable();
}

void Mpeg1DecompressorData::reset()
{

    resetDecompressorData();
#if 0 // TODO: restore this
    imager.unsetDestInfo();
#endif
    Mpeg1Decoder::reset();

}


//
// TODO: (lperry)
// Why not return an XilFormat object
//

int
Mpeg1DecompressorData::getOutputType(unsigned int* width, 
                                     unsigned int* height, 
                                     unsigned int* nbands,
                                     XilDataType*  datatype)
{
    *nbands   = 3;
    *datatype = XIL_BYTE;
    *width    = bitstreamWidth;
    *height   = bitstreamHeight;

    return 1;
}

void
Mpeg1DecompressorData::SetWidthHeightData(int horz,
                                          int vert)
{
    bitstreamWidth = horz;
    bitstreamHeight = vert;
 
    current[F_HORZ]   = horz;
    current[F_VERT]   = vert;
    current[F_HBLOCK] = (horz+15)>>4;
    current[F_TBLOCK] = ((horz+15)>>4)*((vert+15)>>4);
}
 

