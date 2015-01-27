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
//  File:       attemptRecovery.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:14:31, 03/10/00
//
//  Description:
//
//    Jpeg's implementation of attemptRecovery
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)attemptRecovery.cc	1.4\t00/03/10  "



#include "xil/xilGPI.hh"
#include "XilDeviceCompressionJpeg.hh"

#include "JpegMacros.hh"



#define RECOVERY_DONE( ptr, nframes, fixed)          \
{                                                    \
    cbm->errorRecoveryDone( ptr, nframes, fixed );   \
    if(fixed) {                                      \
        read_invalid = FALSE;                        \
    }                                                \
    return;                                          \
} 


// TODO:  still need to gracefully handle the case where we think we\'ve
// only looked at nframes frames, but the cbm says that that is nframes + n frames.
// In this case we need to return a failure, but be ready to pick it back
// up on the next call.  Right now there is no way to go back and get the
// prevByte back.
void 
XilDeviceCompressionJpeg::attemptRecovery(
            unsigned int nframes, 
            unsigned int nbytes,
            Xil_boolean& read_invalid, 
            Xil_boolean& /* write_invalid */)
{
    Xil_unsigned8* ptr;
    Xil_unsigned8* prev;
    Xil_unsigned8* stop_byte;
    int frames_found         = 1;
    int frame_inc            = 1;
    int bytes_read           = 0;
    int bytes_read_in_frame  = 0;
    int max_bytes_in_frame   = getMaxFrameSize();
    int starting_frame       = cbm->getRFrameId();
    int next_seekable        = cbm->nextSeek(starting_frame+1);
    Xil_boolean found_marker = FALSE;

    if(!read_invalid) {  // don't know how to handle write_invalid yet
        RECOVERY_DONE(NULL, 0, FALSE);
    }

    // no pointer to look for bytes
    // don\'t see why this should ever happen
    if((ptr = cbm->getNextByte()) == NULL) {
        if(next_seekable < 0) {          // none can be gotten by cbm
            RECOVERY_DONE(NULL, 0, FALSE);
        } else if(!nframes || (next_seekable <= starting_frame + nframes)) {
            cbm->seek(next_seekable);
            //            Not needed because the seek above goes to the right place.
            //            RECOVERY_DONE(cbm->nextFrame(), next_seekable - starting_frame, TRUE);
            return;
        } else {
            // Note that this does not REALLY work since the cbm does not
            // update the RFrameId in a case where no ptr exists
            RECOVERY_DONE(NULL, nframes, FALSE);
        }
    }

    cbm->nextKnownFrameBoundary(ptr, &stop_byte, &frame_inc);

    prev = NULL;    

    while(1) {

        if(!found_marker) {
            found_marker = (*ptr == MARKER);
        } else {
            if(*ptr == SOI) {
                RECOVERY_DONE(cbm->getNextByte(), frames_found, TRUE);
            }
            found_marker = FALSE;
        }

        // check for hitting a frame boundary (implied or specific )
        if(ptr == stop_byte) {

            if(nframes && (frame_inc > nframes)) {
                RECOVERY_DONE(ptr, nframes, FALSE);
            } else {
                RECOVERY_DONE(ptr, frame_inc, TRUE);
            }

        } else if(bytes_read_in_frame++ == max_bytes_in_frame) {

            cbm->foundFrameDuringRecovery(ptr);
            frames_found++;
            bytes_read_in_frame = 0;

            if(nframes && (frames_found > nframes)) {
                RECOVERY_DONE(ptr, frames_found, FALSE);
            }

            if(next_seekable == (starting_frame + frames_found + 1)) {
                cbm->seek(next_seekable);
                //                Not needed because the seek above goes to the right place.
                //                RECOVERY_DONE(cbm->nextFrame(), next_seekable - starting_frame, TRUE);
                return;
            }
        }

        // check for hitting a byte limit
        if(nbytes && (bytes_read++ > nbytes)) {
            RECOVERY_DONE(ptr, frames_found, FALSE);
        }

        // get the next byte
        prev = ptr;
        if((ptr = cbm->getNextByte()) == NULL) {
            RECOVERY_DONE( prev, frames_found, FALSE );
        }
    }

}


