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
//  File:       xili_codec_utils.hh
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:23:51, 03/10/00
//
//  Description:
//
//    Utility functions for codecs
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)xili_codec_utils.hh	1.2\t00/03/10  "

#ifndef _XILI_CODEC_UTILS_HH_
#define _XILI_CODEC_UTILS_HH_

#include "xil/xilGPI.hh"

    void        xili_copy_rects(void*        src_buf,
                                unsigned int src_nbands,
                                unsigned int src_ps,
                                unsigned int src_ss,
                                unsigned int src_bs,
                                unsigned int src_offset,
                                XilStorage*  storage,
                                XilRoi*      roi,
                                XilBox*      box);

#endif // _XILI_CODEC_UTILS_HH_
