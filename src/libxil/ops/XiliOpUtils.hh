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
//  File:	XiliOpUtils.hh
//  Project:	XIL
//  Revision:	1.11
//  Last Mod:	10:20:33, 03/10/00
//
//  Description:
//	
//	
//	
//	
//	
//	
//	
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliOpUtils.hh	1.11\t00/03/10  "

#ifndef _XILI_OP_UTILS_HH
#define _XILI_OP_UTILS_HH

//
//  The XIL GPI.
//
#include <xil/xilGPI.hh>

//
//  The op cache class
//
#include "XiliOpCache.hh"

//
//  This enum is used in preference to the string name.  It saves us a lot of
//  strcmps throughout the op code.
//
enum XiliInterpolationType {
    XiliUnsupportedInterpolation = -1,

    //
    //  We set these so we can use them as offsets into arrays.
    //
    XiliNearest       = 0,
    XiliBilinear      = 1,
    XiliBicubic       = 2,
    XiliGeneral       = 3
};

const unsigned int XILI_NUM_SUPPORTED_INTERPOLATIONS = 4;
const unsigned int XILI_MAX_GEOMETRIC_NAME_LENGTH    = 256;


//
//  Used to verify the common attributes of images are correct and
//  lookup the op number and place it into a given cache.
//
//  The cache only makes sense to keep in a static array.  So, the
//  user is required to pass in a mutex which is used to guarentee the
//  cache is locked correctly for multiple threads.
//
//  MT-level:  Safe
//

XilOpNumber
xili_check_op_cache(char         op_prefix[],
                    XiliOpCache* op_cache,
                    XilImage*    dst,
                    Xil_boolean  generate_error = TRUE);

XilOpNumber
xili_check_op_cache_cast(char         op_prefix[],
                         XiliOpCache* op_cache,
                         XilImage*    dst,
                         XilImage*    src);

XilOpNumber
xili_check_op_cache_tablewarp(char         op_prefix[],
                              XiliOpCache* op_cache,
                              XilImage*    dst,
                              XilImage*    warp);

XilOpNumber
xili_verify_op_args(char         op_prefix[],
                    XiliOpCache* op_cache,
                    XilImage*    dst,
                    XilImage*    src1 = NULL,
                    XilImage*    src2 = NULL,
                    XilImage*    src3 = NULL,
                    Xil_boolean  generate_error = TRUE);
Xil_boolean
xili_get_geometric_function_name(XilSystemState*       state,
                                 const char*           function_name,
                                 XiliInterpolationType interp,
                                 char*                 func_name);

Xil_boolean
xili_verify_op_logicals(XilImage* image);

XilOpNumber
xili_verify_op_tablewarp(char         op_prefix[],
                         XiliOpCache* op_cache,
                         XilImage*    dst,
                         XilImage*    src1,
                         XilImage*    warp,
                         unsigned int warp_bands);

void*
xili_round_op_values(XilDataType  dtype,
                     float*       values,
                     unsigned int nbands);

void*
xili_clamp_op_logical(XilDataType   dtype,
                      unsigned int* values,
                      unsigned int  nbands);

#endif // _XILI_OP_UTILS_HH

