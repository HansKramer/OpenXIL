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
//  File:       findNextFrameBoundary.cc
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:14:57, 03/10/00
//
//  Description:
//    layer ends. we scan start codes looking for the first        
//    non-slice code which occurs after a slice code.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)findNextFrameBoundary.cc	1.3\t00/03/10  "

#include <xil/xilGPI.hh>
#include "XilDeviceCompressionMpeg1.hh"
#include "Mpeg1DecompressorData.hh"

Xil_unsigned8* 
Mpeg1DecompressorData::findSC(Xil_unsigned8* ptr, 
                              Xil_unsigned8* end, 
                              SC_state*      state)
{
    //
    // Return ptr to last byte in 1st start code encountered
    // starting at ptr. Return null if none found before end.
    //
    Xil_unsigned8        test;

    while(1) {
        if(state->zeros == -1) {
            // Special state!  The last byte of start code is the current byte,
            // last pass stopped just after the marker "0x000001"
            state->zeros = 0;
            return ptr;
        }
        if(state->non_zero) {
            while(1) {                // look for zero byte
                if(ptr == end) {
                    return 0;
                }
                if(!*ptr++) {
                    state->non_zero = FALSE;
                    state->zeros = 8;
                    break;
                }
            }
        } else {
            while(1) {                // look for non_zero byte
                if(ptr == end) {
                    return 0;
                }
                if(test = *ptr++) {
                    state->non_zero = TRUE;
                    if(state->zeros >= 16 && test == 0x1) {
                        if(ptr == end) {
                            // Here if cannot access last byte of start code.
                            // Flag this state so next pass will detect!
                            state->zeros = -1;
                            return 0;
                        } else {
                            state->zeros = 0;
                            return ptr;        // we've got a start code
                        }
                    } else {                     // false alarm
                        state->zeros = 0;
                        break;
                    }
                } else {                // step through stuffing bytes
                    state->zeros += 8;
                }
            }
        }        
    }
}

int
Mpeg1DecompressorData::findNextFrameBoundary()
{

    Xil_unsigned8* ptr;
    Xil_unsigned8* end;
    int            nbytes;

    SC_state       state;
    state.non_zero = TRUE;
    state.zeros    = 0;
    int slice      = 0;
    while(ptr = cbm->getNextBytes(&nbytes)) {
        end = ptr + nbytes;
        while(ptr = findSC(ptr, end, &state)) {
            if(slice) {        // we're in the slice layer
                if((!*ptr) || (*ptr > 0xaf)) {
                    // we've left frame
                    if(*ptr == 0xb7) {
                        // seq end code, include in this frame 
                        return cbm->foundNextFrameBoundary(ptr+1);
                    } else {
                        return cbm->foundNextFrameBoundary(
                                     cbm->ungetBytes(ptr,4));
                    }
                }
            } else {                // looking for slice codes
                if((*ptr > 0x0) && (*ptr < 0xb0)) {
                    slice = 1;                        // found slice code
                }
            }
        }
    }

    return XIL_UNRESOLVED;                // out of buffers
}

void 
Mpeg1DecompressorData::seek(int framenumber)
{
    int               frames_to_skip;
    int               target;
    int               curr_rframe_id;
    int               type;
    int               frame_offset;
    Xil_boolean       found;

    // check to see if we are already at the desired frame
    mpeg_user_data_t* user_ptr = (mpeg_user_data_t *)cbm->getRFrameUserPtr();
    if(user_ptr) {
        // not NULL, we scanned this frame already, check its display id
        if(user_ptr->display_id == framenumber) {
            // we are at the right place, exit
            return;
        }
    } 

    if(framenumber == cbm->getWFrameId()) {
        // here if we are looking for write frame
        target = framenumber;
    } else {
        // guess that target will be (framenumber+1)
        target = framenumber+1;
    }

    if(target > next_scan_id) {
        type = NON_MPEG1_FRAME_TYPE;
    } else {
        type = XIL_CIS_ANY_FRAME_TYPE;
    }

    frames_to_skip = cbm->seek(target, type);

    // scan forward--include this frame in count, may not yet be scanned
    found = scanForward((frames_to_skip+1), framenumber);

    if(found==TRUE) {
        // found the desired frame, exit
        return;
    } else {
        // either we were looking for an out-of-order frame or end of cis,
        // or last frame in cis is incomplete (partial)
        curr_rframe_id = cbm->getRFrameId();
        if(curr_rframe_id != cbm->getSFrameId()) {
            // look backward for out-of-order frame, ignore burn count
            cbm->seekBackToFrameType(MPEG1_NONBFRAME_TYPE);
            user_ptr = (mpeg_user_data_t *)cbm->getRFrameUserPtr();
            if(user_ptr) {
                //
                // Not NULL, we scanned this frame already. 
                // Check its display id
                //
                if(user_ptr->display_id == framenumber) {
                    // we are at the right place, exit
                    return;
                } else {
                    // possibly we are at the last predictor frame,
                    // display id unknown yet
                    if((prev_nonbframe_id != -1) && 
                       (prev_nonbframe_id == cbm->getRFrameId())) {
                        //
                        // Try to update display id 
                        // if temporal reference matches
                        //
                        frame_offset = (curr_display_id - group_base)%MAX_TEMP_REF;
                        if((group_base > curr_display_id) || 
                           (frame_offset == user_ptr->temp_ref)) {
                            user_ptr->display_id = curr_display_id++;
                            prev_nonbframe_id = -1;
                            if(user_ptr->display_id == framenumber) {
                                // we are at the right place, exit
                                return;
                            }
                        }
                    }
                    // must be looking for end of cis, return to end
                    cbm->seek(curr_rframe_id, XIL_CIS_ANY_FRAME_TYPE);
                    return;
                }
            } else {
                // ERROR--all previous frames must have been scanned
                XIL_ERROR( NULL, XIL_ERROR_SYSTEM,"di-319",TRUE);        
                return;
            }
        }
    }   
    return;
}


Xil_boolean 
Mpeg1DecompressorData::scanForward(int scan_count, 
                                   int desired_frame)
{
    int count;
    int curr_id, write_id;
    Xil_unsigned8 *ptr;
    Xil_unsigned8 *end;
    mpeg_user_data_t *user_ptr;
    int type;
    int found_id;
    Mpeg1AdjustType adjust_id;   
    int seek_backup = 1;

    count = scan_count;
    curr_id = cbm->getRFrameId();
    found_id = -1;
    seek_backup = 1;         // usually end up 1 past last scanned frame
    user_ptr = (mpeg_user_data_t *)cbm->getRFrameUserPtr();
    while((user_ptr) && (count>0)) {
        // here if this frame has already been scanned, check its display id
        if(user_ptr->display_id == desired_frame) {
            found_id = curr_id;
        }
        // not desired frame, skip over this frame, to check next
        count--;
        curr_id++;
        cbm->seek(curr_id, XIL_CIS_ANY_FRAME_TYPE);
        user_ptr = (mpeg_user_data_t *)cbm->getRFrameUserPtr();
    }

    while(count>0) {
        //
        // Check for end of cis
        // if we are skipping ahead, the write_frame_id wont be established
        // until last frame scanned, so must check each time
        //
        write_id = cbm->getWFrameId();
        if(curr_id == write_id) {
            // we are at the end of cis...is this where we wanted to be?
            if(desired_frame == write_id) {
                found_id = write_id;
                break;
            } else  {
                seek_backup = 0;     // write frame is last scanned frame
                break;
            }
        } else {
            // we don't know end of cis yet, or we are looking at valid frame
            // attempt to update its frame info node!
            if(ptr = cbm->nextFrame(&end,TRUE)) {
                // cbm says valid frame at this position, establish frame info
                ptr = establishFrameInfo(ptr,end,&user_ptr,&adjust_id);
                if((user_ptr) && (ptr)) {
                    // scanned a valid frame, store its data
                    if(user_ptr->frame_type == MPEG1_BFRAME_TYPE) {
                        type = MPEG1_BFRAME_TYPE;
                    } else {
                        type = MPEG1_NONBFRAME_TYPE;
                        // non-bframe must be start of subgroup
                        subgroup_id = next_scan_id;
                    }
                    cbm->decompressedFrame(ptr,type,user_ptr);
                    next_scan_id++;
                    // check to see if its the desired frame
                    if(user_ptr->display_id == desired_frame) {
                        // we found the frame
                        found_id = curr_id;
                    }

                    // The adjust_id flag indicates when we have determined the
                    // display id of a predictive frame, an I or P frame, also
                    // called a non-B frame.
                    // non-B frames have a display id determined by the number
                    // of "B" frames between them, so the previous non-B frame
                    // is updated when the current non-B frame is established.
                    // Or when the sequence end marker is encountered; if the
                    // sequence end is contained in the current non-B frame, we
                    // can also establish its display id.
                    // So the adjust_id flag has 3 possible states:  NO_ADJUST,
                    // PREV_NONBFRAME, SEQUENCE_END.

                    if(adjust_id == SEQUENCE_END) {
                        // scanned frame contains sequence end,
                        // release past subgroup for getBits/hasFrame
                        subgroup_id = next_scan_id;
                    }
                    if((adjust_id == PREV_NONBFRAME) || 
                       (adjust_id == SEQUENCE_END)) {
                        if(prev_nonbframe_id == -1) {
                            // no previous non-B frame in this seq, start here
                            prev_nonbframe_id = curr_id;
                        } else if(prev_nonbframe_id >= cbm->getSFrameId()) {
                            // update the previous non-B frame display id
                            cbm->seek(prev_nonbframe_id, XIL_CIS_ANY_FRAME_TYPE);
                            user_ptr = (mpeg_user_data_t *)cbm->getRFrameUserPtr();
                            if(user_ptr == NULL) {
                                XIL_ERROR(NULL,XIL_ERROR_SYSTEM,"di-319",TRUE);
                                break;
                            }
                            if(user_ptr->display_id != DISPLAY_ID_TBD) {
                                XIL_ERROR(NULL,XIL_ERROR_SYSTEM,"di-341",TRUE);
                                break;
                            }
                            user_ptr->display_id = curr_display_id++;
                            if(user_ptr->display_id == desired_frame) {
                                // this is the frame we are looking for!
                                found_id = prev_nonbframe_id;
                            }
                            if(adjust_id == SEQUENCE_END) {
                                // end of sequence found in this frame
                                prev_nonbframe_id = -1;
                                // if this is non-b frame, we know its display id
                                cbm->seek(curr_id,XIL_CIS_ANY_FRAME_TYPE);
                                user_ptr = (mpeg_user_data_t *)cbm->getRFrameUserPtr();
                                if(user_ptr == NULL) {
                                    XIL_ERROR(NULL,XIL_ERROR_SYSTEM,"di-319",TRUE);
                                    break;
                                }
                                if((user_ptr->frame_type == MPEG1_PFRAME_TYPE) ||
                       (user_ptr->frame_type == MPEG1_IFRAME_TYPE)) {
                                    // ended sequence with non-B frame
                                    // assign it the next display id
                                    if(user_ptr->display_id != DISPLAY_ID_TBD) {
                                        XIL_ERROR(NULL,XIL_ERROR_SYSTEM,"di-95",TRUE);
                                        break;
                                    }
                                    user_ptr->display_id = curr_display_id++;
                                    if(user_ptr->display_id == desired_frame) {
                                        // this is the frame we are looking for!
                                        found_id = curr_id;
                                    }
                                }   
                            } else {
                                // not end of sequence, update prev_nonbframe_id
                                prev_nonbframe_id = curr_id;
                            }
                            // restore read frame as if no special processing occurred
                            cbm->seek((curr_id+1),XIL_CIS_ANY_FRAME_TYPE);
                        }
                    }
                    curr_id++;
                    count--;
                } else {
                    //
                    // Not valid frame, CIS must end in incomplete frame or
                    // possibly could not alloc user data
                    // cannot scan ahead any farther
                    //
                    seek_backup = 0;     // curr frame is last scanned frame
                    break;
                }
            } else {
                //
                // Not valid frame, CIS must end in  incomplete frame
                // cannot scan ahead any farther
                //
                seek_backup = 0;     // curr frame is last scanned frame
                break;
            }  
        } 
    } // end while
    if(found_id == -1) {
        //
        // If we get here, scanned through all avail frames, did not find 
        // desired frame, seek back to last scanned frame
        //
        cbm->seek((curr_id-seek_backup),XIL_CIS_ANY_FRAME_TYPE);
        return(FALSE);
    } else {
        //
        // If we get here, we've scanned through all avail frames, 
        // found desired frame
        //
        cbm->seek(found_id,XIL_CIS_ANY_FRAME_TYPE);
        return(TRUE);
    }     

} // end routine





Xil_unsigned8* 
Mpeg1DecompressorData::establishFrameInfo(Xil_unsigned8*     start,
                                           Xil_unsigned8*     end,
                                           mpeg_user_data_t** data_ptr, 
                                           Mpeg1AdjustType*   adjust_id)
{
    SC_state state;
    int slice;
    Xil_unsigned8 *ptr;
    Xil_unsigned8 type;
    int tempword;
    Xil_unsigned16 temp_ref;

    state.non_zero = TRUE;
    state.zeros = 0;
    slice = 0;
    ptr = start;
    *data_ptr = NULL;
    *adjust_id = NO_ADJUST;

    while(ptr = findSC(ptr, end, &state)) {
        if(slice) {        // we're in the slice layer, look for non-slice codes
            if((!*ptr) || (*ptr > 0xaf))        // found non-slice code, end frame
            if(*ptr == SEQ_END) {
                *adjust_id = SEQUENCE_END;
                if(ptr = findSC(ptr, end, &state)) {
                    // include all bytes up to next start code in this frame
                    return (ptr-3);
                } else {
                    // seq end last start code, end of cis = end of frame
                    return(end);
                }
            } else  {
                return (ptr-3);
            }
        } else {                // before slice layer, check for SEQ, GOP, PSC

            if(*ptr == SEQ_SC) {
                int tmp_width, tmp_height;

                // update sequence state info
                initParser();         // reset nbits, savedBits
                rdptr = ptr+1;
                endOfBuffer = end;
                if(!doSeq(&tmp_width, &tmp_height, 
                              &aspect_ratio, &picture_rate)) {
                    // bitstream ERROR, incomplete frame
                    XIL_ERROR( NULL, XIL_ERROR_USER,"di-329",TRUE);        
                    return (NULL);
                }
                initParser();         // reset nbits, savedBits
                if(bitstreamWidth == 0) {
                    SetWidthHeightData(tmp_width, tmp_height);
                } else if((tmp_width != bitstreamWidth) || 
                     tmp_height != bitstreamHeight) {
                    XIL_ERROR( NULL, XIL_ERROR_USER, "di-2", TRUE );
                    return (NULL);
                }
            } else if(*ptr == GOP_SC) {
                // update group state info
                if((ptr + 4) > end) {
                    // bitstream ERROR, incomplete frame
                    XIL_ERROR( NULL, XIL_ERROR_USER,"di-329",TRUE);        
                    return (NULL);
                }
                time_code = (((int)ptr[1]&0xff)<<17) | (((int)ptr[2]&0xff)<<9) |
                (((int)ptr[3]&0xff)<<1) | (((int)ptr[4]&0x80)>>7);
                closed_gop = ((int)ptr[4]&0x40)>>6; 
                broken_link = ((int)ptr[4]&0x20)>>5;
                group_base = next_scan_id;
            } else if(*ptr == PIC_SC) {
                // grab picture temp ref, type info for user data
                if((ptr + 2) > end) {
                    // bitstream ERROR, incomplete frame
                    XIL_ERROR( NULL, XIL_ERROR_USER,"di-329",TRUE);        
                    return (NULL);
                }
                mpeg_user_data_t *mpeg_ptr = new mpeg_user_data_t;
                if(mpeg_ptr==NULL) {        
                    XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);        
                    return NULL;
                }
                ptr++;                 // skip over PIC_SC
                tempword = *ptr++;    // grab 8 bits
                tempword = (tempword << 8) | *ptr;    // grab 8 more bits
                temp_ref = (tempword >> 6) & 0x3ff;   // temp_ref = bits 15:6 bits
                mpeg_ptr->temp_ref = temp_ref;
                type = (tempword >> 3) & 0x7;        // type = bits 5:3
                mpeg_ptr->frame_type = type;
                //
                // we ignore temporal reference since so many bitstreams
                // have non-sequential numbering, (only used when no following
                // P/I frame or sequence END)
                // determine display id by sequence of B and non-B frames
                //
                if(type == MPEG1_BFRAME_TYPE) {
                    // for B frame, establish display id
                    mpeg_ptr->display_id = curr_display_id++;
                } else {
                    // for non-B frame, establish prev non-B frame display id
                    mpeg_ptr->display_id = DISPLAY_ID_TBD;
                    *adjust_id = PREV_NONBFRAME;
                }
                mpeg_ptr->time_code = time_code;
                mpeg_ptr->closed_gop = closed_gop;
                mpeg_ptr->broken_link = broken_link;
                mpeg_ptr->aspect_ratio = aspect_ratio;
                mpeg_ptr->picture_rate = picture_rate;
                *data_ptr = mpeg_ptr;
            } else if((*ptr > 0x0) && (*ptr < 0xb0)) {
                slice = 1;                        // found slice code
            }
        }

    }
    // here if we reached end of buffer without leaving slice layer
    if(slice) {
        // found slice code, must be in "isolated" buffer--1 frame only
        return (end);
    } else {
        // never found picture slice code !!! bitstream ERROR, incomplete frame
        XIL_ERROR( NULL, XIL_ERROR_USER,"di-329",TRUE);        
        return (NULL);
    }
}

int 
Mpeg1DecompressorData::checkHistoryBuffers()
{
    int curr_id, next_id, prev_id, target, start_id;
    int next_display_id;
    mpeg_user_data_t *user_ptr;
    Mpeg1ReferenceFrame* temp;

    // 
    // Check to seek if current frame is already in the "next" buffer
    // just transfer to display
    //  
    curr_id = cbm->getRFrameId();
    start_id = cbm->getSFrameId();

    //
    // Check to see if current history buffers contain correct predictors
    //
    user_ptr = (mpeg_user_data_t *)cbm->getRFrameUserPtr();
    if(user_ptr) {
        next_display_id = user_ptr->display_id +1;
        if(user_ptr->frame_type != MPEG1_BFRAME_TYPE) {

            // push P/I frame into last buffer if possible
            // check for next frame pre-fetched
            if((next) && (next->getFrameId() == curr_id)) {
                // flush next to last buffer
                prev_id = curr_id+1;
                if(establishLastBuffer(curr_id,prev_id,next_display_id)) {
                    return(0);
                } else {
                    // failed to prefetch next frame, at curr_id
                    // decompress curr_id again
                    temp = next;
                    next = last;
                    last = temp;
                    return(1);
                }
            }

            if(user_ptr->frame_type == MPEG1_PFRAME_TYPE) {
                // for P frame dependent on previous frames
                if(curr_id!=start_id) {
                    cbm->seekBackToFrameType(MPEG1_NONBFRAME_TYPE);
                }
                prev_id = cbm->getRFrameId();
            } else {
                // I frame no previous dependents
                prev_id = curr_id;
            }

            while(1) {

                // for P frame, check for prev frame in pipeline
                // for I frame, noop
                if((next) && (next->getFrameId() == prev_id)) {
                    prev_id += 1;
                    if(establishLastBuffer(curr_id,prev_id,next_display_id)) {
                        return(0);
                    } else {
                        // failed to prefetch next frame
                        cbm->seek(prev_id,XIL_CIS_ANY_FRAME_TYPE);
                        burnFrames(curr_id-prev_id);
                        return(1);
                    }
                }

                // no pipeline established, must pre-fetch next predictive frame
                user_ptr = (mpeg_user_data_t *)cbm->getRFrameUserPtr();
                if((user_ptr->frame_type == MPEG1_IFRAME_TYPE) ||
                   (prev_id == start_id)) {
                    if(establishLastBuffer(curr_id,prev_id,next_display_id)) {
                        return(0);
                    } else {
                        // failed to prefetch next frame, at curr_id
                        // burn through any dependents, decompress curr_id
                        cbm->seek(prev_id,XIL_CIS_ANY_FRAME_TYPE);
                        burnFrames(curr_id-prev_id);
                        return(1);
                    }
                }

                // keep looking back for start 
                cbm->seekBackToFrameType(MPEG1_NONBFRAME_TYPE);
                prev_id = cbm->getRFrameId();
            }

        } else {
            // check both next and last buffers for B frame
            if(curr_id!=start_id) {
                cbm->seekBackToFrameType(MPEG1_NONBFRAME_TYPE);
            }
            next_id = cbm->getRFrameId();
            //
            // Check if we are at first available frame, 
            // ( sequence I2 B0 B1... ) 
            //
            if(next_id == start_id) {    // cannot get "prev" frame 
                if((next) && (next->getFrameId() == next_id)) {
                    cbm->seek(curr_id,XIL_CIS_ANY_FRAME_TYPE);
                } else {
                    burnFrames(curr_id-next_id);
                }
                return(1);
            }
            cbm->seekBackToFrameType(MPEG1_NONBFRAME_TYPE);
            prev_id = cbm->getRFrameId();
            if((next) && (last)) {
                if((next->getFrameId() == next_id) &&
                   (last->getFrameId() == prev_id)) {
                    cbm->seek(curr_id,XIL_CIS_ANY_FRAME_TYPE);
                    return(1);
                }
            }
            while(1) {
                // check for burn from current position 
                if(next) {
                    if(next->getFrameId() == prev_id) {
                        target = prev_id+1;
                        cbm->seek(target,XIL_CIS_ANY_FRAME_TYPE);
                        burnFrames(curr_id-target);
                        return(1);
                    }
                }
                // check for I frame to initialize burn 
                // or first available frame 
                user_ptr = (mpeg_user_data_t *)cbm->getRFrameUserPtr();
                if((user_ptr->frame_type == MPEG1_IFRAME_TYPE) ||
                   (prev_id==start_id)) {
                    burnFrames(curr_id-prev_id);
                    return(1);
                }
                // keep looking back for start 
                cbm->seekBackToFrameType(MPEG1_NONBFRAME_TYPE);
                prev_id = cbm->getRFrameId();
            }
        } // end if/else type "i" 
    } else {
        // error, asked to check history buffer for incomplete frame
        XIL_ERROR( NULL, XIL_ERROR_USER, "di-329", TRUE);
        return(-1);
    }
}

int 
Mpeg1DecompressorData::burnFrames(int nframes)
{
    int i;
    mpeg_user_data_t *user_ptr;

    for(i=0;i<nframes;i++) {
        if( !setByteStreamPtr()) {
            // We have a condition where 
            //   decompress: no frame from buffer mgr
            XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-110", TRUE);
            return XIL_FAILURE;
        }

        user_ptr = (mpeg_user_data_t *)cbm->getRFrameUserPtr();
        if(user_ptr == NULL) {
            // should never be asked to burn unscanned frame 
            // must be bad bitstream or internal error 
            return XIL_FAILURE;
        }

        if(user_ptr->frame_type == MPEG1_BFRAME_TYPE) {
            // skip over bframes 
            finishDecode();
        } else {        
            // I or P frame, update history buffers 
            setDitheringFlag(0);
            color_video_frame();
            finishDecode();
        }    
    }
    return XIL_SUCCESS;
}

void 
Mpeg1DecompressorData::outputLastBuffer()
{
    // here to output frame stored in last buffer 
    // TODO: FIx when doDith is done
    //doDith(&imager, last->mb,current[F_TBLOCK],current[F_HORZ]);
    return;
}

int 
Mpeg1DecompressorData::establishLastBuffer(int curr, 
                                               int start_burn, 
                                               int next_frame)
{
    int end;

    seek(next_frame);
    if(cbm->hasFrame()) {
        end = cbm->getRFrameId();
        if(cbm->getRFrameType()==MPEG1_BFRAME_TYPE) {
            end -=1;
        }
        cbm->seek(start_burn,XIL_CIS_ANY_FRAME_TYPE);
        if(burnFrames((end-start_burn)+1) == XIL_SUCCESS) {
            return(1);
        }
    }
    // here if no "next" frame or failed the burnFrames
    // go back to curr frame
    cbm->seek(curr, XIL_CIS_ANY_FRAME_TYPE);
    return(0);

}



void
Mpeg1DecompressorData::addEos(int frame_id)
{
    // EOS encountered while parsing the bitstream
    // adjusts the subgroup id ahead of the current
    // parse frame...we need to do the same thing
    // if the frame appended with the eos has
    // already been scanned. frame_id is last_frame+1, ie
    // write_frame; next_scan_id is last_parse_frame+1.
    if(frame_id==next_scan_id) {
        subgroup_id = frame_id;
    }
}

Xil_boolean
Mpeg1DecompressorData::hasFrame()
{
    int curr_id;
    int save_scan_id;

    //
    // Because of the EOS marker, once we know the cbm 
    // has a frame avail, still must check if this
    // subgroup is available.
    //
    // The scanner tracks the frame id of the start
    // of the most recently parsed subgroup.
    // A subgroup is started by any non-Bframe,
    // so in a cis of all I frames, or I-P frames,
    // every frame is the start of a subgroup.
    //
    // In cis with B frames, then each non-B frame
    // is tagged as the start of a subgroup.
    // Any subgroup is released (made avail as frame)
    // by the following subgroup or by the addition
    // of the EOS marker to the last frame in the group.
    //
    if(cbm->hasFrame()==TRUE) {
        //
        // At the current read position, the cbm has a frame
        // check this frame's subgroup relationship.
        //
        curr_id = cbm->getRFrameId();
        if(curr_id < subgroup_id) {
            //
            // This frame is "before" the current subgroup start
            // so it has been released
            //
            return TRUE;
        }
        //
        // The subgroup_id has only advanced as far as the scanner
        // has parsed...if there are unscanned frames in the
        // cis, they may release the current subgoup.
        // move forward until this frame's subgroup is released
        // or until we hit the end of the CIS.
        //
        save_scan_id = -1;
        while( (next_scan_id != cbm->getWFrameId()) &&
             (next_scan_id != save_scan_id) ) {
            save_scan_id = next_scan_id;
            cbm->seek(next_scan_id,XIL_CIS_ANY_FRAME_TYPE);
            scanForward(1,-1);
            if(curr_id < subgroup_id) {
                //
                // Subgroup was released by forward scan
                //
                cbm->seek(curr_id,XIL_CIS_ANY_FRAME_TYPE);
                return TRUE;
            }
        }
        //
        // Here if scanned through all frames, and this frame's
        // subgroup still has not been released
        //
        cbm->seek(curr_id,XIL_CIS_ANY_FRAME_TYPE);
        return FALSE;  
    } else {
        return FALSE;
    }
}

