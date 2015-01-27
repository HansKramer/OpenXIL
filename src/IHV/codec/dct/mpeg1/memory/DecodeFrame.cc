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
//  File:       DecodeFrame.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:14:44, 03/10/00
//
//  Description:
//
//    MpegDecompressorData class.
//    Contains data unique to a frame decompression
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)DecodeFrame.cc	1.2\t00/03/10  "

#include "Mpeg1DecompressorData.hh"
#include "IdctRefFrame.hh"

//
// This is the main routine that decompresses an Mpeg1 frame
//
void 
Mpeg1DecompressorData::color_video_frame()
{
    initParser();

    //
    // TODO: Add tests for XIL_FAILURE
    //

    //
    // Allocate three reference frames.
    // These will be used to hold the:
    //     Oldest reconstructed I or P frame
    //     Newest reconstructed I or P frame
    //     A B frame.
    //

    if(Copy0.getMacroBlockArray() == NULL) {
        //
        // Allocate an array of Macroblocks to hold the whole frame
        //
        Copy0.allocMacroBlocks(current[F_TBLOCK]);

        //
        // Allocate a reference frame to hold a decoded I or P frame
        // to be used in reconstructing other P or B frames
        //
        //
        Copy0.createReference(current[F_HORZ], current[F_VERT]);
        Copy0.setType(0);

        Copy1.allocMacroBlocks(current[F_TBLOCK]);
        Copy1.createReference(current[F_HORZ], current[F_VERT]);
        Copy1.setType(0);

        CopyB.allocMacroBlocks(current[F_TBLOCK]);
        CopyB.createReference(current[F_HORZ], current[F_VERT]);
        CopyB.setType(0);

        //
        // TODO: Make individual tests
        //
#if 0
        if(!Copy0.mb || !Copy0.rf || !Copy1.mb || !Copy1.rf || !CopyB.mb
            || !CopyB.rf) {
            XIL_CIS_UNCOND_ERROR(XIL_ERROR_RESOURCE,"di-1", TRUE, 
                                 cbm->getXilDeviceCompression(), TRUE, FALSE);
            deleteFrames();
            return;
        }
#endif

        last = &Copy1;
        next = &Copy0;
        bbbb = &CopyB;
    }

    int         foundStart = 0;
    int         word       = 0;
    Xil_boolean done       = FALSE;

    //
    // Start parsing the bitstream
    //
    while(!done) {

        if(!gettostart()) {
            // Must have hit end of buffer!
            if(foundStart) {
                break;
            } else {
                return;
            }
        }

        GETBITS(32,word);

        switch(word) {
          case SEQ_ST_CODE:
            if(!doSeq(0,0,0,0)) {
                return;
            }
            break;
          case GOP_ST_CODE:
            if(!doGop()) {
                return;
            }
            break;
          case USR_ST_CODE:
            if(!doUsr()) {
                return;
            }
            break;
          case EXT_ST_CODE:
            if(!doExt()) {
                return;
            }
            break;
          case PIC_ST_CODE:
            //
            // TODO: XIL 1.2 note. 
            // Temporary hack to determine end of picture.
            // Need something better
            //
            if(foundStart) {
                done   = TRUE;;
                rdptr -= 4;
            } else {
                foundStart = 1;
                if(!doPic()) {
                    return;
                }
            }
            break;
          case END_ST_CODE:
            done = TRUE;;
            break;
          case 0xffffffff:
            done = TRUE;;
            break;
          default:
            // Presumably a slice code
            if(word >= 0x00000101 && word <= 0x000001af) {
                if(frametypeC == MPEG1_IFRAME_TYPE) {
                    doSscI(word&0xff);
                } else if(frametypeC == MPEG1_PFRAME_TYPE) {
                    doSscP(word&0xff);
                } else if(frametypeC == MPEG1_BFRAME_TYPE) {
                    doSscB(word&0xff);
                }
            }
            break;

        } // End switch

    } // End while(! done)


#if 0 // TODO: Restore after dithering gets implemented
     //  Also, cases are identical in I and P frames
    if(getDitheringFlag()) {
        switch(frametypeC) {
          case MPEG1_IFRAME_TYPE:
          case MPEG1_PFRAME_TYPE:
            if(next->type == MPEG1_IFRAME_TYPE) {
                doDith(&imager, next->mb,current[F_TBLOCK],current[F_HORZ]);
            } else if(next->type == MPEG1_PFRAME_TYPE) {
                doDith(&imager, next->mb,current[F_TBLOCK],current[F_HORZ]);
            }
            break;
          case MPEG1_BFRAME_TYPE:
            doDith(&imager, bbbb->mb,current[F_TBLOCK],current[F_HORZ]);
            break;
        }
    }
#endif


  ErrorReturn:
    return;
}

int 
Mpeg1DecompressorData::doSeq(int* ret_width, 
                             int* ret_height,
                             int* ret_aspect_ratio, 
                             int* ret_picture_rate)
{

    int horz;
    GETBITS(12,horz);

    int vert;
    GETBITS(12,vert);

    int par;
    GETBITS(4,par);

    int pr;
    GETBITS(4,pr);

    int bitrate;
    GETBITS(18,bitrate);

    int resbit;
    GETBITS(1,resbit);

    int bufsiz;
    GETBITS(10,bufsiz);

    int constrain;
    GETBITS(1,constrain);

    int liq;
    GETBITS(1,liq);

    int i;
    int quant_val;

    if(liq == 1) {
        for(i=0;i<64;i++) {
            GETBITS(8,quant_val);
            // quant_val of 0 is illegal
            if(quant_val == 0) {
                quant_val = 2;
            }
            quantintra[i] = quant_val;
        }
    } else {
        initIntraTable();
    }

    int lnq;
    GETBITS(1,lnq);

    if(lnq == 1) {
        for(i=0;i<64;i++) {
            GETBITS(8,quant_val);
            // quant_val of 0 is illegal
            if(quant_val == 0) {
                quant_val = 2;
            }
            quantnonin[i] = quant_val;
        }
    } else {
        initNonintraTable();
    }

    if(ret_width != NULL) {
        *ret_width = horz;
    }
    if(ret_height != NULL) {
        *ret_height = vert;
    }
    if(ret_aspect_ratio != NULL) {
        *ret_aspect_ratio = par;
    }
    if(ret_picture_rate != NULL) {
        *ret_picture_rate = pr;
    }

    return 1;

  ErrorReturn:
    return 0;
}

int Mpeg1DecompressorData::doUsr()
{
    if(!gettostart()) {
        return 0;
    }
    return 1;

}

int Mpeg1DecompressorData::doExt()
{
    if(!gettostart()) {
        return 0;
    }
    return 1;

}

int Mpeg1DecompressorData::doGop()
{
    int                tmp;

    // GETBITS(25,timec); GETBITS(1,close); GETBITS(1,broke); 

    GETBITS(27,tmp);
    return 1;

  ErrorReturn:
    return 0;
}


int 
Mpeg1DecompressorData::doPic()
{
    Mpeg1ReferenceFrame* tptr;
    int                  tempr,ptype,bufful;
    int                  j;
    int*                 cptr = current;


    GETBITS(10,tempr);
    GETBITS(3,ptype);
    GETBITS(16,bufful);

    if(ptype == MPEG1_DFRAME_TYPE) {
        XIL_ERROR(NULL, XIL_ERROR_CIS_DATA,"di-325",TRUE);
        return 0;
    }

    frametypeC = ptype;
    cptr[F_COUNT]++;
    cptr[L_MBA] = -2;
    cptr[C_MBA] = -1;

    if(ptype == MPEG1_IFRAME_TYPE) {
        // Swap last and next Ref frames
        tptr = next;
        next = last;
        last = tptr;
        next->setFilledFlag(FALSE);
        next->setType(MPEG1_IFRAME_TYPE);
        next->setFrameId(cbm->getRFrameId());
    }

    if(ptype == MPEG1_PFRAME_TYPE) {
        // Swap last and next Ref frames
        tptr = next;
        next = last;
        last = tptr;
        next->setFilledFlag(FALSE);
        next->setType(MPEG1_PFRAME_TYPE);
        next->setFrameId(cbm->getRFrameId());

        if(!last->getFilledFlag()) {
            last->populateReference();
            last->setFilledFlag(TRUE);
        }
    }

    if(ptype == MPEG1_BFRAME_TYPE) {
        if(!last->getFilledFlag()) {
            last->populateReference();
            last->setFilledFlag(TRUE);
        }
        if(!next->getFilledFlag()) {
            next->populateReference();
            next->setFilledFlag(TRUE);
        }
    }

    if(ptype == MPEG1_PFRAME_TYPE || ptype == MPEG1_BFRAME_TYPE) {
        GETBITS(1,cptr[F_F_FPV]);
        GETBITS(3,cptr[F_F_F_CODE]);
        if(ptype == MPEG1_BFRAME_TYPE) {
            GETBITS(1,cptr[F_B_FPV]);
            GETBITS(3,cptr[F_B_F_CODE]);
        }
    }

    GETBITS(1, j);
    while(j == 1) {
        GETBITS(8,j);
        GETBITS(1,j);
    }

    return 1;

  ErrorReturn:
    return 0;
}
