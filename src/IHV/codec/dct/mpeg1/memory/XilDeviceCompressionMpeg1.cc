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
//  File:       XilDeviceCompressionMpeg1.cc
//  Project:    XIL
//  Revision:   1.11
//  Last Mod:   10:14:54, 03/10/00
//
//  Description:
//
//   DeviceCompression class for Mpeg1 
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDeviceCompressionMpeg1.cc	1.11\t00/03/10  "


#include <string.h>
#include "XilDeviceCompressionMpeg1.hh"

// TODO: Placeholder value
#define FRAMES_PER_BUFFER        2

XilDeviceCompressionMpeg1::XilDeviceCompressionMpeg1(
    XilDeviceManagerCompression* xdct, 
    XilCis*                      xcis)
: XilDeviceCompression(xdct, xcis, 0, FRAMES_PER_BUFFER)
{

    if(! deviceCompressionValid()) {
        return;
    }

    initValues();
    getCisBufferManager()->setSeekToStartFrameFlag(TRUE);
    decompData.cbm = getCisBufferManager();

}

XilDeviceCompressionMpeg1::~XilDeviceCompressionMpeg1()
{
    //
    // Destroy the ImageFormat object(s).
    //
    if(inputType != outputType) {
        outputType->destroy();
    }

    inputType->destroy();

}

void 
XilDeviceCompressionMpeg1::setCAspectRatio(XilMpeg1PelAspectRatio value)
{
    if(value < 0 || value > 14) {
        XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-324", TRUE);
    } else {
        decompData.aspectRatioC = (Xil_unsigned32) value;
        decompData.version++;
    }
}

XilMpeg1PelAspectRatio 
XilDeviceCompressionMpeg1::getCAspectRatio()
{
    return (XilMpeg1PelAspectRatio)decompData.aspectRatioC;
}


void 
XilDeviceCompressionMpeg1::setCPictureRate(XilMpeg1PictureRate value)
{
    if(value < 0 || value > 8) {
        XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-324", TRUE);
    } else {
        decompData.pictureRateC = (Xil_unsigned32) value;
        decompData.version++;
    }
}

XilMpeg1PictureRate 
XilDeviceCompressionMpeg1::getCPictureRate()
{
    return (XilMpeg1PictureRate)decompData.pictureRateC;
}


Xil_unsigned32 
XilDeviceCompressionMpeg1::setCTimeCode(XilMpeg1TimeCode *code)
{
    decompData.version++;
    if(code) {
        decompData.timeCodeC =  code->drop_frame_flag ? 0x1000000 : 0;
        decompData.timeCodeC |=  (code->hours&0x1f) << 19;
        decompData.timeCodeC |=  (code->minutes&0x1f) << 13;
        decompData.timeCodeC |=  (code->seconds&0x3f) << 6;
        decompData.timeCodeC |=  (code->pictures&0x3f);
        return decompData.timeCodeC;
    } else {
        return decompData.timeCodeC = 0x80000000; // indicates null default
    }
}

XilMpeg1TimeCode* 
XilDeviceCompressionMpeg1::getCTimeCode()
{
    // Null default
    if(decompData.timeCodeC & 0x80000000) {
        return NULL;
    }

    XilMpeg1TimeCode* code = new XilMpeg1TimeCode;
    if (code == NULL) {
        XIL_ERROR(getSystemState(), XIL_ERROR_RESOURCE, "di-1", TRUE);
        return NULL;
    }

    code->drop_frame_flag = decompData.timeCodeC & 0x1000000;
    code->hours = (decompData.timeCodeC & 0xf80000) >> 19;
    code->minutes = (decompData.timeCodeC & 0x7e000) >> 13;
    code->seconds = (decompData.timeCodeC & 0xfc0) >> 6;
    code->pictures = decompData.timeCodeC & 0x3f;
    return code;
}

void 
XilDeviceCompressionMpeg1::setPattern(XilMpeg1Pattern *pattern)
{
    decompData.compressionPattern.pattern = NULL;

    if(pattern != NULL) {
        if(pattern->pattern != NULL) {
            decompData.compressionPattern.pattern = strdup(pattern->pattern);
        }
        decompData.compressionPattern.repeat_count = pattern->repeat_count;
    } 

    decompData.version++;
}

XilMpeg1Pattern* 
XilDeviceCompressionMpeg1::getPattern()
{
    XilMpeg1Pattern* pattern = NULL;

    if(decompData.compressionPattern.pattern != NULL) {
        pattern = new XilMpeg1Pattern;
        if(pattern == NULL){
            return NULL;
        }
        pattern->pattern = NULL;
        pattern->pattern = strdup(decompData.compressionPattern.pattern);
        if(pattern->pattern == NULL){
            return NULL;
        }
        pattern->repeat_count = decompData.compressionPattern.repeat_count;
    } 

    return pattern;
}

int 
XilDeviceCompressionMpeg1::setBitsPerSecond(int value)
{
    if(value < 0 || value > 104856800) {
        XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-324", TRUE);
        return XIL_FAILURE;
    } else {
        decompData.version++;
        return decompData.bitsPerSecond = value/400 + (value%400 ? 1 : 0);
    }
}

int 
XilDeviceCompressionMpeg1::getBitsPerSecond()
{
    return decompData.bitsPerSecond*400;
}

void 
XilDeviceCompressionMpeg1::setIntraQTable(Xil_unsigned8 *qtable)
{
    int i;

    decompData.version++;
    if(!qtable) {
        decompData.intraQptr = 0;
    } else {
        for(i=0; i<64; i++) {
            if(qtable[i] <= 0 || qtable[i] > 255) {
                decompData.intraQTable[i] = 16;
                XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-324", TRUE);
            } else {
                decompData.intraQTable[i] = qtable[i];
            }
        }
        decompData.intraQptr = &decompData.intraQTable[0];
    }
}

Xil_unsigned8* 
XilDeviceCompressionMpeg1::getIntraQTable()
{
    int i;

    if(decompData.intraQptr) {

        Xil_unsigned8* qtable = new Xil_unsigned8[64];

        for(i=0; i<64; i++) {
            qtable[i] = decompData.intraQTable[i];
        }

        return qtable;
    } else {
        return 0;
    }
}

void 
XilDeviceCompressionMpeg1::setNonIntraQTable(Xil_unsigned8 *qtable)
{
    int i;

    decompData.version++;
    if(!qtable) {
        decompData.nonIntraQptr = 0;
    } else {
        for(i=0; i<64; i++) {
            if(qtable[i] <= 0 || qtable[i] > 255) {
                decompData.nonIntraQTable[i] = 16;
                XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-324", TRUE);
            } else {
                decompData.nonIntraQTable[i] = qtable[i];
            }
        }
        decompData.nonIntraQptr = decompData.nonIntraQTable;
    }
}

Xil_unsigned8* 
XilDeviceCompressionMpeg1::getNonIntraQTable()
{
    int i;

    if(decompData.nonIntraQptr) {
        Xil_unsigned8* qtable = new Xil_unsigned8[64];

        for(i=0; i<64; i++) {
            qtable[i] = decompData.nonIntraQTable[i];
        }
        return qtable;
    } else {
        return 0;
    }
}

void 
XilDeviceCompressionMpeg1::setSlicesPerPicture(int value)
{
    if(value <= 0) {
        XIL_ERROR(getSystemState(), XIL_ERROR_USER, "di-324", TRUE);
    } else {
        decompData.slicesPerPicture = value;
        decompData.version++;
    }

}

int 
XilDeviceCompressionMpeg1::getSlicesPerPicture()
{
    return decompData.slicesPerPicture;

}

void 
XilDeviceCompressionMpeg1::setInsertSequenceEnd(Xil_boolean value)
{
    decompData.insertSequenceEnd = value;
    decompData.version++;
}

Xil_boolean 
XilDeviceCompressionMpeg1::getInsertSequenceEnd()
{
    return decompData.insertSequenceEnd;
}


void 
XilDeviceCompressionMpeg1::setDecompressorQuality(int value)
{
    decompData.decompressionQuality = value;
    if(value >= 100) {
        decompData.bframequality = 7;        // full quality B frame decoding.
    } else {
        decompData.bframequality = 2;        // fast B frame decoding.
    }
    decompData.version++;

}

int 
XilDeviceCompressionMpeg1::getDecompressorQuality()
{
    return decompData.decompressionQuality;
}

int 
XilDeviceCompressionMpeg1::getDAspectRatio()
{
    float        rvalue;
    int                ivalue;

    switch (decompData.aspectRatioD) {
        case 0x1:        rvalue = (float)1.0;           break;
        case 0x2:        rvalue = (float)0.6735;        break;
        case 0x3:        rvalue = (float)0.7031;        break;
        case 0x4:        rvalue = (float)0.7615;        break;
        case 0x5:        rvalue = (float)0.8055;        break;
        case 0x6:        rvalue = (float)0.8437;        break;
        case 0x7:        rvalue = (float)0.8935;        break;
        case 0x8:        rvalue = (float)0.9157;        break;
        case 0x9:        rvalue = (float)0.9815;        break;
        case 0xa:        rvalue = (float)1.0255;        break;
        case 0xb:        rvalue = (float)1.0695;        break;
        case 0xc:        rvalue = (float)1.0950;        break;
        case 0xd:        rvalue = (float)1.1575;        break;
        case 0xe:        rvalue = (float)1.2015;        break;
        default:         rvalue = (float)0.0;           // this is an error
    }
    ivalue = *(int *)&rvalue;
    return ivalue;
}

int 
XilDeviceCompressionMpeg1::getDPictureRate()
{
    float        rvalue;
    int                ivalue;

    switch (decompData.pictureRateD) {
        case 0x1:        rvalue = (float)23.96;        break;
        case 0x2:        rvalue = (float)24.0;        break;
        case 0x3:        rvalue = (float)25.0;        break;
        case 0x4:        rvalue = (float)29.97;        break;
        case 0x5:        rvalue = (float)30.0;        break;
        case 0x6:        rvalue = (float)50.0;        break;
        case 0x7:        rvalue = (float)59.94;        break;
        case 0x8:        rvalue = (float)60.0;        break;
        default:        rvalue = (float)0.0;                // this is an error
    }
    ivalue = *(int *)&rvalue;
    return ivalue;
}

XilMpeg1TimeCode* 
XilDeviceCompressionMpeg1::getDTimeCode()
{
    XilMpeg1TimeCode* code = new XilMpeg1TimeCode;

    code->drop_frame_flag = decompData.timeCodeD & 0x1000000;
    code->hours           = (decompData.timeCodeD & 0xf80000) >> 19;
    code->minutes         = (decompData.timeCodeD & 0x7e000) >> 13;
    code->seconds         = (decompData.timeCodeD & 0xfc0) >> 6;
    code->pictures        = decompData.timeCodeD & 0x3f;

    return code;
}

Xil_boolean 
XilDeviceCompressionMpeg1::getClosedFlag()
{
    return decompData.closedGop;
}

Xil_boolean 
XilDeviceCompressionMpeg1::getLinkFlag()
{
    return decompData.brokenLink;
}

Xil_unsigned32 
XilDeviceCompressionMpeg1::getTemporalReference()
{
    return decompData.temporalReference;
}

XilMpeg1FrameType 
XilDeviceCompressionMpeg1::getFrameType()
{
    XilMpeg1FrameType        ftype;

    return ftype = (XilMpeg1FrameType)decompData.frameType;
}

void
XilDeviceCompressionMpeg1::initValues()
{
    inputType  = getSystemState()->createXilImageFormat(0U, 0U, 3U, XIL_BYTE);
    outputType = inputType;

    inputOutputType     = FALSE;
    frameWidth          = 0;
    frameHeight         = 0;
    eos_frame_id        = -1;

    setRandomAccess(TRUE);

    mpeg1_eos_marker[0] = 0x00;
    mpeg1_eos_marker[1] = 0x00;
    mpeg1_eos_marker[2] = 0x01;
    mpeg1_eos_marker[3] = 0xb7;
    external_flush      = (PMFV_XilDeviceManager)NULL;
    external_device     = NULL;
}


void
XilDeviceCompressionMpeg1::reset()
{
    if(inputType != outputType) {
        outputType->destroy();
    }
    inputType->destroy();

    initValues();
    decompData.reset();
    XilDeviceCompression::reset();
}


//
//  Routines to get references to the Compressor, Decompressor and
//  Attribute specific classes.
//

Mpeg1DecompressorData*  
XilDeviceCompressionMpeg1::getMpeg1DecompressorData(void)
{
    return &decompData;
}

int 
XilDeviceCompressionMpeg1::findNextFrameBoundary()
{
    return decompData.findNextFrameBoundary();
}

void 
XilDeviceCompressionMpeg1::burnFrames(int nframes)
{
    if(nframes > 0) {
        decompData.burnFrames(nframes);
    }
}

// TODO: Is this correct for Mpeg?
#define MAX_BITS_PER_PIXEL      26
#define MAX_BYTES_PER_FRAME_HEADER      1000
int 
XilDeviceCompressionMpeg1::getMaxFrameSize()
{
    int max_bits_per_frame;

    max_bits_per_frame = MAX_BITS_PER_PIXEL *
    ((inputType->getWidth() + 7) & ~7) *
    ((inputType->getHeight() + 7) & ~7) *
    3;

    return ((max_bits_per_frame + 7) >> 3) + MAX_BYTES_PER_FRAME_HEADER;
}

//
//  Function to read header and fill in the header information --
//  specifically width and height
//
XilStatus 
XilDeviceCompressionMpeg1::deriveOutputType()
{

    unsigned int w;
    unsigned int h;
    unsigned int nbands;
    XilDataType  datatype;

    //
    // Input and output types are assumed to be the same
    //
    if(inputOutputType == FALSE) {
        // type not initialized from bitstream
        if(!decompData.getOutputType(&w, &h, &nbands, &datatype)) {
            return XIL_FAILURE;
        }

        XilImageFormat* newtype = getSystemState()->createXilImageFormat(
                              w, h, nbands, datatype);
        if(newtype == NULL) {
            XIL_ERROR(getSystemState(),XIL_ERROR_RESOURCE,"di-1",TRUE);
            return XIL_FAILURE;
        }
        setInputType(newtype);  // sets outputType as a sideaffect
        newtype->destroy();
        inputOutputType = TRUE;
    }

    return XIL_SUCCESS;
}

    

#if 0 // TODO: Add an equivalent
Xil_boolean
XilDeviceCompressionMpeg1::isOK()
{
    _XIL_ISOK_TEST();
}
#endif

//------------------------------------------------------------------------
//
//  Function:        XilDeviceCompressionMpeg1::seek
//
//  Description:
//        
//  Parameters:
//        
//        
//  Returns:
//        void
//        
//------------------------------------------------------------------------

void
XilDeviceCompressionMpeg1::seek(int framenumber,Xil_boolean )
{
    //    for now, ignore history update, since Mpeg seeking is always
    //    scanning ahead for display id and frame type anyway!
    decompData.seek(framenumber);
    return;
}


void*
XilDeviceCompressionMpeg1::getBitsPtr(int * nbytes, int* nframes)
{
    int curr_id, curr_display_id, prev_id, next_display_id, new_id, start_id;
    Xil_unsigned8 *curr_buf;
    int byte_start_id, byte_end_id;
    Xil_unsigned8 *ptr;
    mpeg_user_data_t *user_ptr;

    XilCisBufferManager* theCBM = getCisBufferManager();

    ptr = NULL;
    *nbytes = 0;
    *nframes = 0;

    if(hasFrame() == TRUE) {
        user_ptr = (mpeg_user_data_t *)theCBM->getRFrameUserPtr();
        if(user_ptr) {
            start_id = theCBM->getSFrameId();
            curr_id = theCBM->getRFrameId();
            curr_display_id = user_ptr->display_id;
            curr_buf = theCBM->getRBuffer();
            if(theCBM->getRFrameType() == MPEG1_BFRAME_TYPE) {
                if(curr_id > start_id) {
                    prev_id = curr_id-1;
                    theCBM->seek(prev_id, XIL_CIS_ANY_FRAME_TYPE);
                }
                if(theCBM->getRFrameType() == MPEG1_BFRAME_TYPE) {
                    byte_start_id = byte_end_id = curr_id;
                } else {
                    if(theCBM->getRBuffer() != curr_buf) {
                        // special case !!!
                        if(theCBM->moveEndStartOneBuffer() == XIL_FAILURE)
                        return (void *)(ptr);
                        curr_buf = theCBM->getRBuffer();
                    }
                    byte_start_id = prev_id;
                    byte_end_id = curr_id;
                }
                next_display_id = curr_display_id+1;
                seek(next_display_id);
                while(hasFrame()==TRUE) {
                    new_id = theCBM->getRFrameId();
                    if(new_id > byte_end_id) {
                        if(theCBM->getRBuffer() != curr_buf) {
                            // moved forward into next buffer
                            break;
                        } else {
                            byte_end_id = new_id;
                        }
                    }
                    next_display_id++;
                    seek(next_display_id);
                }
            } else {
                if(curr_id == curr_display_id) {
                    byte_start_id = byte_end_id = curr_id;
                } else {
                    byte_start_id = byte_end_id = -1;
                }
                next_display_id = curr_display_id+1;
                seek(next_display_id);
                while((theCBM->getRBuffer() == curr_buf) && (hasFrame()==TRUE)) {
                    new_id = theCBM->getRFrameId();
                    if(byte_start_id == -1) {
                        if(theCBM->getRFrameType()==MPEG1_BFRAME_TYPE) {
                            byte_start_id = new_id-1;
                        }
                    }
                    if(new_id > byte_end_id) {
                        byte_end_id = new_id;
                    }
                    next_display_id++;
                    seek(next_display_id);
                }
            }
            *nframes = next_display_id - curr_display_id;
            if(byte_start_id == -1) {
                ptr = NULL;
                *nbytes = 0;
            } else {
                theCBM->seek(byte_start_id,XIL_CIS_ANY_FRAME_TYPE);
                ptr  = theCBM->getNumBytesToFrame(byte_end_id,nbytes);
                seek(next_display_id);
            }
        } else {
            //  ERROR--no user_ptr established for frame by seek!!!
            ptr = NULL;
            *nbytes = 0;
            *nframes = 0;
        }
    } else {
        //  no frame available
        ptr = NULL;
        *nbytes = 0;
        *nframes = 0;
    }
    return (void *)(ptr);   
}


int 
XilDeviceCompressionMpeg1::numberOfFrames()
{
    int frame_count;
    int curr_id, next_id;
    mpeg_user_data_t *user_ptr;

    frame_count = 0;
    if(hasFrame()) {

        user_ptr = (mpeg_user_data_t *) 
                   getCisBufferManager()->getRFrameUserPtr();
        if(user_ptr) {

            curr_id = user_ptr->display_id;
            next_id = curr_id;

            while(hasFrame()) {
                next_id +=1;
                seek(next_id);
            }

            frame_count = next_id - curr_id;
            seek(curr_id);
        } else {
            //  should never be at valid read frame with NULL user_ptr for MPEG
            XIL_CIS_ERROR( XIL_ERROR_RESOURCE,"di-319",TRUE,this,TRUE,FALSE);
            return (0);
        }

    }     

    return (frame_count);
}

//------------------------------------------------------------------------
//
//  Function:        XilDeviceCompressionMpeg1::hasData()
//
//  Description:
//        
//    This routine is called when the user issues xil_cis_has_data() command.
//      It is to determine how much data is present from the read frame to
//      the end of the cis.  MPEG contains out of order frames which are
//      predictive frames for the following frames, as in the sequence
//      IPBB, where the B's are display frames BEFORE the P frame.
//      There must be logic in hasData which deals with the out of order
//      frames.  The notes in the code below document the behavior
//      for the 4 cases
//          1. read frame is in-order predictive frame (P,I)
//          2. read frame is 1st bidirection frame (B) of subgroup
//          3. read frame is 2nd or higher B of subgroup
//          4. read frame is out-of-order P,I of subgroup
//
//  Parameters:
//        none
//
//  Return:
//      number of bytes from read frame forward
//------------------------------------------------------------------------

int 
XilDeviceCompressionMpeg1::hasData()
{
    int curr_id, curr_display_id, prev_id, nonb_id, start_id;
    mpeg_user_data_t *user_ptr;
    int nbytes = 0;

    XilCisBufferManager* theCBM = getCisBufferManager();


    //    here if there is data available at current read frame

    //    NOTE: since mpeg has out-of-order frames, we must check
    //    type of this frame.  If curr B frame, then we are within 
    //    a subgroup; hasData must include the number of bytes in the 
    //    subgroup's predictive frame (which came BEFORE the B frame)

    //    As shown in case 1, curr read frame the non-B frame, in-order
    //
    //    _________curr read frame = 0
    //    |
    //    v
    //    I0 P3 B1 B2         
    //    \........./
    //         |
    //    hasData includes bytes I0 + P3 + B1 + B2 (no skip)
    //
    //    For the case 2, curr read frame 1st B frame in subgroup
    //
    //          _________curr read frame = 1
    //          |
    //          v
    //    I0 P3 B1 B2         
    //    \...../
    //       |
    //    hasData includes all these bytes P3 + B1 + B2
    //
    //    As read frame advances, the hasData handles the data "gap"
    //    in the subgroup.
    //    As shown in case 3, curr read frame NOT 1st B frame in subgroup
    //
    //    _________curr read frame = 2
    //    |
    //    v
    //    I0 P3 B1 B2         
    //    \./  \./
    //     |____|
    //        |
    //    hasData includes bytes P3 + B2   (skips B1)
    //
    //    As shown in case 4, curr read frame is the non-B frame in subgroup
    //
    //    _________curr read frame = 3
    //    |
    //    v
    //    I0 P3 B1 B2         
    //    \./
    //     |
    //    hasData includes bytes P3    (skips B1,B2)

    user_ptr = (mpeg_user_data_t *)theCBM->getRFrameUserPtr();
    if(user_ptr) {
        start_id = theCBM->getSFrameId();       // first frame of CIS
        curr_id = theCBM->getRFrameId();        // curr frame id
        curr_display_id = user_ptr->display_id; // curr display id
        if(theCBM->getRFrameType() == MPEG1_BFRAME_TYPE) {

            // if B frame, check preceding frame for B or non-B
            if(curr_id > start_id) {
                prev_id = curr_id-1;
                theCBM->seek(prev_id, XIL_CIS_ANY_FRAME_TYPE);
                if(theCBM->getRFrameType() == MPEG1_BFRAME_TYPE) {
                    // here if prev_id frame also B
                    // means curr_id not the first B in subgroup
                    // so we must "skip" around to get data
                    // first, get number of bytes in subgroup
                    // predictive frame
                    if(theCBM->seekBackToFrameType(MPEG1_NONBFRAME_TYPE)>0) {
                        // found previous nonbframe..
                        // count data in this predictive frame
                        nonb_id = theCBM->getRFrameId();
                        theCBM->getNumBytesToFrame(nonb_id,&nbytes);
                    }
                    // restore read frame to curr_id
                    theCBM->seek(curr_id, XIL_CIS_ANY_FRAME_TYPE);                  
                    // get the data from this frame forward                  
                    nbytes += theCBM->hasData();
                } else {
                    // here if prev_id frame non-B,
                    // means curr_id is the first B in subgroup
                    // so just include the prev_id non-B frame in data;
                    // get data from this frame forward 
                    nbytes += theCBM->hasData();

                    // restore read frame to curr_id
                    theCBM->seek(curr_id, XIL_CIS_ANY_FRAME_TYPE);                  
                }
            } else {
                // here if the curr_id is the start_id of the CIS,
                // no previous frames to include!
                // get the data from this frame forward                  
                nbytes += theCBM->hasData();
            }
        } else {

            // here if current frame is non-B frame
            // check to see if I am sitting at out-of-order frame
            if(curr_id == curr_display_id) {
                // here if display id and frame id match,
                // not an out-of-order frame
                // get data from this frame forward
                nbytes = theCBM->hasData();
            } else {
                // here if display id and frame id mismatch,
                // must be predictive frame for subgroup
                // only include this frame data from the subgroup,
                // then move past subgroup, add any remaining data
                theCBM->getNumBytesToFrame(curr_id,&nbytes);
                theCBM->seek((curr_display_id+1),XIL_CIS_ANY_FRAME_TYPE);

                // get data from this frame forward
                nbytes += theCBM->hasData();

                // then, move back to original read frame
                theCBM->seek(curr_id,XIL_CIS_ANY_FRAME_TYPE);
            }
        }
    } else {
        // cannot check type of frame
        // maybe partial frame or maybe at end of cis or empty cis
        // just hand back theCBM->hasData 
        nbytes = theCBM->hasData();
    }

    return nbytes;
}


//------------------------------------------------------------------------
//
//  Function:        XilDeviceCompressionMpeg1::adjustStart(int framenumber);
//
//  Description:
//        
//    This routine is used to adjust the start frame within the buffer
//      lists. Since the desired start frame may be dependent on previous
//      frames, this routine detects the type of the start frame.  If the 
//      start frame is an I-frame, there are no previous dependencies.  If the
//      start frame is a P-frame, it is dependent on the last preceding
//      I frame.  A B-frame is dependent on the next to last preceding I frame.
//      These preceding frames will be kept within the cis although they may 
//      not be accessed. They are only used to process the new start frame 
//      (and frames that may follow the new start frame). The read frame
//      is established by these rules of dependency.
//           The  XilCisBufferManager::adjustStart() routine adjusts
//      buffers such that any frames previous to the read_frame are inaccesible.
//        
//  Parameters:
//        
//        int framenumber:  the desired new start frame
//  Returns:
//        
//        XIL_SUCCESS if no error occurs else XIL_FAILURE
//        
//------------------------------------------------------------------------
int 
XilDeviceCompressionMpeg1::adjustStart(int framenumber)
{
    int curr_id, prev_id, start_id, init_read_frame_id;
    mpeg_user_data_t *user_ptr;  

    XilCisBufferManager* theCBM = getCisBufferManager();

    init_read_frame_id = theCBM->getRFrameId();

    seek(framenumber);
    user_ptr=(mpeg_user_data_t *)theCBM->getRFrameUserPtr();  
    curr_id = theCBM->getRFrameId();
    start_id = theCBM->getSFrameId();
    if(user_ptr) {
        if((user_ptr->frame_type==MPEG1_IFRAME_TYPE) || (curr_id == start_id)) {
            prev_id = curr_id;
        } else {
            theCBM->seekBackToFrameType(MPEG1_NONBFRAME_TYPE);
            prev_id = theCBM->getRFrameId();
            if((user_ptr->frame_type==MPEG1_BFRAME_TYPE) && (prev_id != start_id)) {
                theCBM->seekBackToFrameType(MPEG1_NONBFRAME_TYPE);
                prev_id = theCBM->getRFrameId();
            }
            while(1) {
                user_ptr = (mpeg_user_data_t *)theCBM->getRFrameUserPtr();
                if((user_ptr->frame_type == MPEG1_IFRAME_TYPE) ||
             (prev_id == start_id)) {
                    break;
                }
                theCBM->seekBackToFrameType(MPEG1_NONBFRAME_TYPE);
                prev_id = theCBM->getRFrameId();          
            }
        }
        if(theCBM->adjustStart(prev_id,XIL_CIS_ANY_FRAME_TYPE)==XIL_SUCCESS) {
            // reset read frame to original position if ahead of start frame
            if(init_read_frame_id > prev_id)
            theCBM->seek(init_read_frame_id,XIL_CIS_ANY_FRAME_TYPE);
            return XIL_SUCCESS;
        } else {
            return XIL_FAILURE;
        }
    } else {
        return XIL_FAILURE;
    }

}

void 
XilDeviceCompressionMpeg1::registerExternalFlush(
    PMFV_XilDeviceManager pmfv, 
    XilDeviceManager*     dev)
{
    external_flush  = pmfv;
    external_device = dev;
    return;
}

//------------------------------------------------------------------------
//
//  Function:        XilDeviceCompressionMpeg1::flush()
//
//  Description:
//        
//    This routine is called when the user issues xil_cis_flush() command.
//      This command informs the compressor that the user expects any
//      outstanding compressions to be completed, ie, output to the cis
//      if possible.  For instance, there may be internally buffered
//      frames which must be modified and output if in the midst of a
//      (PBB) group.  This is left up to the compressor; at any rate,
//      the cis write frame has been adjusted by the number of buffered
//      frames, so an appropriate number of skip frames must at least be
//      "compressed" into the cis.
//
//      TODO: the mechanism for notifying the piggybacked compressor
//            of the flush is to have it register its flush routine with
//            the XDCMpeg1::flush, so that it can be called from here.
//
//      The flush will also add an "end of seqeuence" marker to the
//      current cis if the insertSequenceEnd attribute is TRUE.  The mpeg1
//      bitstream is under special constraints for hasFrame and getBits
//      to retain the last frame or subgroup of frames in case an EOS must
//      be appended.  For more information, see the decompData.hasFrame
//      routine.
//           The  XilCisBufferManager::addToLastFrame() routine allows the
//      addition of the EOS marker to the last frame.  It registers this
//      last frame id so that addition xil_cis_flush() calls will not
//      re-append the EOS marker (this would be illegal).  It then
//      notifies the decompData class of the frame id with the additional marker
//      in case the last frame had already been parsed.
//      
//        
//  Parameters:
//        none
//        
//------------------------------------------------------------------------
void 
XilDeviceCompressionMpeg1::flush()
{
    int write_frame;

    XilCisBufferManager* theCBM = getCisBufferManager();


    // allow device compression to flush internal buffers
    if(external_flush)
    (external_device->*external_flush)();

    write_frame = theCBM->getWFrameId();

    // check for EOS marker addition
    if(decompData.insertSequenceEnd==TRUE) {
        // add the EOS marker to the last frame in the cis
        if(write_frame == 0) {
            // cannot add EOS marker to no frame; ignore
            return;
        } else if(write_frame < 0) {
            // may need EOS to establish last frame (if partial)
            if(theCBM->addToLastFrame(&mpeg1_eos_marker[0],sizeof(mpeg1_eos_marker))==XIL_SUCCESS) {
                // write frame is unknown; must establish number of frames
                // to record eos_frame_id
                numberOfFrames();
                write_frame = theCBM->getWFrameId();
                if(write_frame < 0) {
                    // error,still unresolved partial frame at end of cis...
                    XIL_CIS_ERROR( XIL_ERROR_USER,"di-329",TRUE,this,TRUE,FALSE);
                    return;
                }
                eos_frame_id = write_frame;
                // notify the decompData class of appended EOS
                decompData.addEos(eos_frame_id);
            }
        } else if(write_frame != eos_frame_id) {
            // here if write frame known;
            // add bytes to last frame, record eos_frame_id
            // errors reported by cbm routine
            if(theCBM->addToLastFrame(&mpeg1_eos_marker[0],
                                  sizeof(mpeg1_eos_marker))==XIL_SUCCESS) {
                eos_frame_id = write_frame;
                // notify the decompData class of appended EOS
                decompData.addEos(eos_frame_id);
            }
        }
    }
}



//------------------------------------------------------------------------
//
//  Function:        XilDeviceCompressionMpeg1::hasFrame()
//
//  Description:
//        
//    This routine is called when the user issues xil_cis_has_frame() command.
//      It is to determine if there is a valid frame to output for the user,
//      via either decompress or getBits.  Since Mpeg1 has the concept of
//      the EOS marker, the cis must retain the last frame or last group
//      of subframes to ensure there is a frame to append with the EOS.
//      So there is logic in the decompData class which goes beyond the
//      simple check for frame data which the cbm will perform.
//
//  NOTE: This must be CLEARLY documented for the user; frames are avail
//        1. when the subgroup is released by a following subgroup.
//      2. when the last frame of the subgroup is appended with EOS.
//      A subgroup may be as small as 1 frame (as in all I sequence cis),
//      or as large as the longest sequence of B frames (I.PBBBBBBB.PBB).
//
//  Parameters:
//        none
//
//  Return:
//      TRUE if frame available, else FALSE        .
//------------------------------------------------------------------------
Xil_boolean 
XilDeviceCompressionMpeg1::hasFrame()
{
    // call the decompData class to determine the state of this frame's subgroup
    return decompData.hasFrame();
}

//
// Function to retrieve or create dither tables.
// This is located in the DeviceCompression because
// different CIS'es may be using different dither matrices.
// So each CIS must have its own table.
//
XiliOrderedDitherLut*
XilDeviceCompressionMpeg1::getDitherTable(XilLookupColorcube* cmap,
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
    if(ditherTable == NULL                        ||
       !cmap->isSameAs(&current_cmap_version)     ||
       !dmask->isSameAs(&current_dmask_version)   ||
       scale[0]  != current_scale[0]              ||
       scale[1]  != current_scale[1]              ||
       scale[2]  != current_scale[2]              ||
       offset[0] != current_offset[0]             ||
       offset[1] != current_offset[1]             ||
       offset[2] != current_offset[2] ) {

        ditherTable = new XiliOrderedDitherLut(cmap, dmask, scale, offset, 0);
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


