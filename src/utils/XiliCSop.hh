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
//  File:       XiliCSop.hh
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:23:59, 03/10/00
//
//  Description:
//      Provide Opcode support for ColorConvert
//
//------------------------------------------------------------------------
//
//      COPYRIGHT
//
//------------------------------------------------------------------------
#pragma ident   "@(#)XiliCSop.hh	1.5\t00/03/10  "

#ifndef _XILI_CSOP_HH
#define _XILI_CSOP_HH

#ifdef _XIL_LIBXIL_PRIVATE
#include "_XilDefines.h"
#else
#include <xil/xilGPI.hh>
#endif

//
// Maximum bands per image which might be src or dst of a ColorConvert op.
//
#define _XILI_CS_MAX_BANDS 4 // TODO bpb 5/2/1997 Right place 4 this definition?


#define XIL_CS_RGBLINEAR_TO_rgblinear ((XIL_CS_RGBLINEAR<<16) | XIL_CS_RGBLINEAR )
#define XIL_CS_RGBLINEAR_TO_rgb709    ((XIL_CS_RGBLINEAR<<16) | XIL_CS_RGB709    )
#define XIL_CS_RGBLINEAR_TO_photoycc  ((XIL_CS_RGBLINEAR<<16) | XIL_CS_PHOTOYCC  )
#define XIL_CS_RGBLINEAR_TO_ycc601    ((XIL_CS_RGBLINEAR<<16) | XIL_CS_YCC601    )
#define XIL_CS_RGBLINEAR_TO_ycc709    ((XIL_CS_RGBLINEAR<<16) | XIL_CS_YCC709    )
#define XIL_CS_RGBLINEAR_TO_cmyk      ((XIL_CS_RGBLINEAR<<16) | XIL_CS_CMYK      )
#define XIL_CS_RGBLINEAR_TO_cmy       ((XIL_CS_RGBLINEAR<<16) | XIL_CS_CMY       )
#define XIL_CS_RGBLINEAR_TO_ylinear   ((XIL_CS_RGBLINEAR<<16) | XIL_CS_YLINEAR   )
#define XIL_CS_RGBLINEAR_TO_y601      ((XIL_CS_RGBLINEAR<<16) | XIL_CS_Y601      )
#define XIL_CS_RGBLINEAR_TO_y709      ((XIL_CS_RGBLINEAR<<16) | XIL_CS_Y709      )

#define XIL_CS_RGB709_TO_rgblinear  ((XIL_CS_RGB709<<16) | XIL_CS_RGBLINEAR )
#define XIL_CS_RGB709_TO_rgb709     ((XIL_CS_RGB709<<16) | XIL_CS_RGB709    )
#define XIL_CS_RGB709_TO_photoycc   ((XIL_CS_RGB709<<16) | XIL_CS_PHOTOYCC  )
#define XIL_CS_RGB709_TO_ycc601     ((XIL_CS_RGB709<<16) | XIL_CS_YCC601    )
#define XIL_CS_RGB709_TO_ycc709     ((XIL_CS_RGB709<<16) | XIL_CS_YCC709    )
#define XIL_CS_RGB709_TO_cmyk       ((XIL_CS_RGB709<<16) | XIL_CS_CMYK      )
#define XIL_CS_RGB709_TO_cmy        ((XIL_CS_RGB709<<16) | XIL_CS_CMY       )
#define XIL_CS_RGB709_TO_ylinear    ((XIL_CS_RGB709<<16) | XIL_CS_YLINEAR   )
#define XIL_CS_RGB709_TO_y601       ((XIL_CS_RGB709<<16) | XIL_CS_Y601      )
#define XIL_CS_RGB709_TO_y709       ((XIL_CS_RGB709<<16) | XIL_CS_Y709      )

#define XIL_CS_PHOTOYCC_TO_rgblinear ((XIL_CS_PHOTOYCC<<16) | XIL_CS_RGBLINEAR )
#define XIL_CS_PHOTOYCC_TO_rgb709    ((XIL_CS_PHOTOYCC<<16) | XIL_CS_RGB709    )
#define XIL_CS_PHOTOYCC_TO_photoycc  ((XIL_CS_PHOTOYCC<<16) | XIL_CS_PHOTOYCC  )
#define XIL_CS_PHOTOYCC_TO_ycc601    ((XIL_CS_PHOTOYCC<<16) | XIL_CS_YCC601    )
#define XIL_CS_PHOTOYCC_TO_ycc709    ((XIL_CS_PHOTOYCC<<16) | XIL_CS_YCC709    )
#define XIL_CS_PHOTOYCC_TO_cmyk      ((XIL_CS_PHOTOYCC<<16) | XIL_CS_CMYK      )
#define XIL_CS_PHOTOYCC_TO_cmy       ((XIL_CS_PHOTOYCC<<16) | XIL_CS_CMY       )
#define XIL_CS_PHOTOYCC_TO_ylinear   ((XIL_CS_PHOTOYCC<<16) | XIL_CS_YLINEAR   )
#define XIL_CS_PHOTOYCC_TO_y601      ((XIL_CS_PHOTOYCC<<16) | XIL_CS_Y601      )
#define XIL_CS_PHOTOYCC_TO_y709      ((XIL_CS_PHOTOYCC<<16) | XIL_CS_Y709      )

#define XIL_CS_YCC601_TO_rgblinear   ((XIL_CS_YCC601<<16) | XIL_CS_RGBLINEAR )
#define XIL_CS_YCC601_TO_rgb709      ((XIL_CS_YCC601<<16) | XIL_CS_RGB709    )
#define XIL_CS_YCC601_TO_photoycc    ((XIL_CS_YCC601<<16) | XIL_CS_PHOTOYCC  )
#define XIL_CS_YCC601_TO_ycc601      ((XIL_CS_YCC601<<16) | XIL_CS_YCC601    )
#define XIL_CS_YCC601_TO_ycc709      ((XIL_CS_YCC601<<16) | XIL_CS_YCC709    )
#define XIL_CS_YCC601_TO_cmyk        ((XIL_CS_YCC601<<16) | XIL_CS_CMYK      )
#define XIL_CS_YCC601_TO_cmy         ((XIL_CS_YCC601<<16) | XIL_CS_CMY       )
#define XIL_CS_YCC601_TO_ylinear     ((XIL_CS_YCC601<<16) | XIL_CS_YLINEAR   )
#define XIL_CS_YCC601_TO_y601        ((XIL_CS_YCC601<<16) | XIL_CS_Y601      )
#define XIL_CS_YCC601_TO_y709        ((XIL_CS_YCC601<<16) | XIL_CS_Y709      )

#define XIL_CS_YCC709_TO_rgblinear ((XIL_CS_YCC709<<16) | XIL_CS_RGBLINEAR )
#define XIL_CS_YCC709_TO_rgb709    ((XIL_CS_YCC709<<16) | XIL_CS_RGB709    )
#define XIL_CS_YCC709_TO_photoycc  ((XIL_CS_YCC709<<16) | XIL_CS_PHOTOYCC  )
#define XIL_CS_YCC709_TO_ycc601    ((XIL_CS_YCC709<<16) | XIL_CS_YCC601    )
#define XIL_CS_YCC709_TO_ycc709    ((XIL_CS_YCC709<<16) | XIL_CS_YCC709    )
#define XIL_CS_YCC709_TO_cmyk      ((XIL_CS_YCC709<<16) | XIL_CS_CMYK      )
#define XIL_CS_YCC709_TO_cmy       ((XIL_CS_YCC709<<16) | XIL_CS_CMY       )
#define XIL_CS_YCC709_TO_ylinear   ((XIL_CS_YCC709<<16) | XIL_CS_YLINEAR   )
#define XIL_CS_YCC709_TO_y601      ((XIL_CS_YCC709<<16) | XIL_CS_Y601      )
#define XIL_CS_YCC709_TO_y709      ((XIL_CS_YCC709<<16) | XIL_CS_Y709      )

#define XIL_CS_CMY_TO_rgblinear   ((XIL_CS_CMY<<16) | XIL_CS_RGBLINEAR )
#define XIL_CS_CMY_TO_rgb709      ((XIL_CS_CMY<<16) | XIL_CS_RGB709    )
#define XIL_CS_CMY_TO_photoycc    ((XIL_CS_CMY<<16) | XIL_CS_PHOTOYCC  )
#define XIL_CS_CMY_TO_ycc601      ((XIL_CS_CMY<<16) | XIL_CS_YCC601    )
#define XIL_CS_CMY_TO_ycc709      ((XIL_CS_CMY<<16) | XIL_CS_YCC709    )
#define XIL_CS_CMY_TO_cmy         ((XIL_CS_CMY<<16) | XIL_CS_CMY       )
#define XIL_CS_CMY_TO_cmyk        ((XIL_CS_CMY<<16) | XIL_CS_CMYK      )
#define XIL_CS_CMY_TO_ylinear     ((XIL_CS_CMY<<16) | XIL_CS_YLINEAR   )
#define XIL_CS_CMY_TO_y601        ((XIL_CS_CMY<<16) | XIL_CS_Y601      )
#define XIL_CS_CMY_TO_y709        ((XIL_CS_CMY<<16) | XIL_CS_Y709      )

#define XIL_CS_CMYK_TO_rgblinear  ((XIL_CS_CMYK<<16) | XIL_CS_RGBLINEAR )
#define XIL_CS_CMYK_TO_rgb709     ((XIL_CS_CMYK<<16) | XIL_CS_RGB709    )
#define XIL_CS_CMYK_TO_photoycc   ((XIL_CS_CMYK<<16) | XIL_CS_PHOTOYCC  )
#define XIL_CS_CMYK_TO_ycc601     ((XIL_CS_CMYK<<16) | XIL_CS_YCC601    )
#define XIL_CS_CMYK_TO_ycc709     ((XIL_CS_CMYK<<16) | XIL_CS_YCC709    )
#define XIL_CS_CMYK_TO_cmy        ((XIL_CS_CMYK<<16) | XIL_CS_CMY       )
#define XIL_CS_CMYK_TO_cmyk       ((XIL_CS_CMYK<<16) | XIL_CS_CMYK      )
#define XIL_CS_CMYK_TO_ylinear    ((XIL_CS_CMYK<<16) | XIL_CS_YLINEAR   )
#define XIL_CS_CMYK_TO_y601       ((XIL_CS_CMYK<<16) | XIL_CS_Y601      )
#define XIL_CS_CMYK_TO_y709       ((XIL_CS_CMYK<<16) | XIL_CS_Y709      )

#define XIL_CS_YLINEAR_TO_rgblinear ((XIL_CS_YLINEAR<<16) | XIL_CS_RGBLINEAR )
#define XIL_CS_YLINEAR_TO_rgb709    ((XIL_CS_YLINEAR<<16) | XIL_CS_RGB709    )
#define XIL_CS_YLINEAR_TO_photoycc  ((XIL_CS_YLINEAR<<16) | XIL_CS_PHOTOYCC  )
#define XIL_CS_YLINEAR_TO_ycc601    ((XIL_CS_YLINEAR<<16) | XIL_CS_YCC601    )
#define XIL_CS_YLINEAR_TO_ycc709    ((XIL_CS_YLINEAR<<16) | XIL_CS_YCC709    )
#define XIL_CS_YLINEAR_TO_cmy       ((XIL_CS_YLINEAR<<16) | XIL_CS_CMY       )
#define XIL_CS_YLINEAR_TO_cmyk      ((XIL_CS_YLINEAR<<16) | XIL_CS_CMYK      )
#define XIL_CS_YLINEAR_TO_ylinear   ((XIL_CS_YLINEAR<<16) | XIL_CS_YLINEAR   )
#define XIL_CS_YLINEAR_TO_y601      ((XIL_CS_YLINEAR<<16) | XIL_CS_Y601      )
#define XIL_CS_YLINEAR_TO_y709      ((XIL_CS_YLINEAR<<16) | XIL_CS_Y709      )

#define XIL_CS_Y601_TO_rgblinear  ((XIL_CS_Y601<<16) | XIL_CS_RGBLINEAR )
#define XIL_CS_Y601_TO_rgb709     ((XIL_CS_Y601<<16) | XIL_CS_RGB709    )
#define XIL_CS_Y601_TO_photoycc   ((XIL_CS_Y601<<16) | XIL_CS_PHOTOYCC  )
#define XIL_CS_Y601_TO_ycc601     ((XIL_CS_Y601<<16) | XIL_CS_YCC601    )
#define XIL_CS_Y601_TO_ycc709     ((XIL_CS_Y601<<16) | XIL_CS_YCC709    )
#define XIL_CS_Y601_TO_cmy        ((XIL_CS_Y601<<16) | XIL_CS_CMY       )
#define XIL_CS_Y601_TO_cmyk       ((XIL_CS_Y601<<16) | XIL_CS_CMYK      )
#define XIL_CS_Y601_TO_ylinear    ((XIL_CS_Y601<<16) | XIL_CS_YLINEAR   )
#define XIL_CS_Y601_TO_y601       ((XIL_CS_Y601<<16) | XIL_CS_Y601      )
#define XIL_CS_Y601_TO_y709       ((XIL_CS_Y601<<16) | XIL_CS_Y709      )

#define XIL_CS_Y709_TO_rgblinear ((XIL_CS_Y709<<16) | XIL_CS_RGBLINEAR )
#define XIL_CS_Y709_TO_rgb709    ((XIL_CS_Y709<<16) | XIL_CS_RGB709    )
#define XIL_CS_Y709_TO_photoycc  ((XIL_CS_Y709<<16) | XIL_CS_PHOTOYCC  )
#define XIL_CS_Y709_TO_ycc601    ((XIL_CS_Y709<<16) | XIL_CS_YCC601    )
#define XIL_CS_Y709_TO_ycc709    ((XIL_CS_Y709<<16) | XIL_CS_YCC709    )
#define XIL_CS_Y709_TO_cmy       ((XIL_CS_Y709<<16) | XIL_CS_CMY       )
#define XIL_CS_Y709_TO_cmyk      ((XIL_CS_Y709<<16) | XIL_CS_CMYK      )
#define XIL_CS_Y709_TO_ylinear   ((XIL_CS_Y709<<16) | XIL_CS_YLINEAR   )
#define XIL_CS_Y709_TO_y601      ((XIL_CS_Y709<<16) | XIL_CS_Y601      )
#define XIL_CS_Y709_TO_y709      ((XIL_CS_Y709<<16) | XIL_CS_Y709      )

#endif // _XILI_CSOP_HH
