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
//  File:   InFrame.cc
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:15:42, 03/10/00
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
//  MT-level:  <??????>
//
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)InFrame.cc	1.3\t00/03/10  "

#include "xil/xilGPI.hh"
#include "InFrame.hh"

InFrame::InFrame(void) 
{
    //
    //  I put Yr*r into a table for simple lookups and shifts later.
    //  I incorporate the constant additions into the third array of the table
    //  for more speed.  The multiplication by 16.0 is to add an additional 4
    //  significant bits for each entry into the table.  The values are added
    //  together with these significant bits and then down shifted by 4 bits.
    //  This ensures proper accuracy is maintained.
    //
    for(int i=0; i<256; i++) {
      Yr[i] = int( YR_VAL * (float) i * 16.0);
      Yg[i] = int( YG_VAL * (float) i * 16.0);
      Yb[i] = int((YB_VAL * (float) i +  16.0 + 0.5) * 16.0);

      Ur[i] = int( UR_VAL * (float) i * 16.0);
      Ug[i] = int( UG_VAL * (float) i * 16.0);
      Ub[i] = int((UB_VAL * (float) i + 128.0 + 0.5) * 16.0);

      Vr[i] = int( VR_VAL * (float) i * 16.0);
      Vg[i] = int( VG_VAL * (float) i * 16.0);
      Vb[i] = int((VB_VAL * (float) i + 128.0 + 0.5) * 16.0);
    }    

    data = NULL;
    yuvimage = FALSE;
}

//------------------------------------------------------------------------
//
//  Function:    InFrame::useNewImage()
//
//  Description:
//    Resets the image from which the data is being grabbed from.
//    
//------------------------------------------------------------------------

int
InFrame::useNewImage(XilSystemState* system_state,
                     unsigned int    imageWidth,
                     unsigned int    imageHeight,
                     unsigned int    num_bands,
                     unsigned int    p_stride,
                     unsigned int    s_stride,
                     Xil_unsigned8*  buffer,
                     XilColorspace*  colorspace) 
{
    systemState = system_state;
    width  = imageWidth;
    height = imageHeight;
    bands  = num_bands;
    
    if (colorspace != NULL) {
      XilColorspace* rgb709 = (XilColorspace*)
            systemState->getObjectByName("rgb709", XIL_COLORSPACE);
      XilColorspace* ycc601 = (XilColorspace*)
            systemState->getObjectByName("ycc601", XIL_COLORSPACE);
      XilColorspace* ycc709 = (XilColorspace*)
            systemState->getObjectByName("ycc709", XIL_COLORSPACE);
      XilColorspace* photoycc = (XilColorspace*)
            systemState->getObjectByName("photoycc", XIL_COLORSPACE);

      if (colorspace->getOpcode() == rgb709->getOpcode()) {
        yuvimage = FALSE;
      } else if (colorspace->getOpcode() == ycc601->getOpcode() ||
                 colorspace->getOpcode() == ycc709->getOpcode() ||
                 colorspace->getOpcode() == photoycc->getOpcode()) {
        yuvimage = TRUE;
      } else {
        // error -- incompatible colorspace
      }
    }
    
    scan_stride  = s_stride;
    pixel_stride = p_stride;
    line_length  = width*pixel_stride;

    data = buffer;
    
    cur_y = 0;
    cur_x = (int)-1;

    return XIL_SUCCESS;
}
    
//------------------------------------------------------------------------
//
//  Function:    InFrame::getYUVBlock
//
//  Description:
//    Pulls out the next 4x4 block of data from the image and
//      returns it in YUV format.
//    
//------------------------------------------------------------------------
int
InFrame::getYUVBlock(ColorValue* block)
{
    if (data) {
      if (cur_x == 0) {
        cur_y += scan_stride<<2;
      } else if ((int)cur_x == (int)-1) {
        cur_y = cur_x = 0;
      }

      int  y, x;
      int  j, k;
      int  r,g,b;
      ColorValue* pCV = block;
      if (yuvimage == TRUE) {
        for (y=cur_y, j=0; j<4; y += scan_stride,j++) {
          for (x=cur_x, k=0; k<4; x += pixel_stride, k++) {
            Xil_unsigned8* offset = (data + y + x);
            pCV->setColor(*(offset), *(offset + 1), *(offset + 2));
            pCV++;
          }
        }
      } else {
        for (y=cur_y, j=0; j<4; y += scan_stride,j++) {
          for (x=cur_x, k=0; k<4; x += pixel_stride, k++) {
            Xil_unsigned8* offset = (data + y + x);

            b = *(offset);
            g = *(offset + 1);
            r = *(offset + 2);

            pCV->setColor(((Yr[r] + Yg[g] + Yb[b])>>4),
                          ((Ur[r] + Ug[g] + Ub[b])>>4),
                          ((Vr[r] + Vg[g] + Vb[b])>>4));
            pCV++;
          }
        }
      }
        
    if ((cur_x += (4 * pixel_stride)) >= line_length) {
      cur_x = 0;
    }

    return XIL_SUCCESS;

    } else {
      XIL_ERROR(systemState, XIL_ERROR_SYSTEM, "di-95", TRUE);
      return XIL_FAILURE;
    }
}

//------------------------------------------------------------------------
//
//  Function:    InFrame::getRGBBlock
//
//  Description:
//    Gets the next 4x4 block of data from the image keeping it in RGB
//  colorspace. 
//    
//------------------------------------------------------------------------

int
InFrame::getRGBBlock(ColorValue* block)
{
    if (data) {
      if(cur_x == 0) {
        cur_y += scan_stride<<2;
      } else if((int)cur_x == (int)-1) {
        cur_y = cur_x = 0;
      }
    
      int  y, x;
      int  j, k;
      ColorValue* pCV = block;
      for (y=cur_y, j=0; j<4; y += scan_stride,j++) {
        for (x=cur_x, k=0; k<4; x += pixel_stride, k++) {
          Xil_unsigned8* offset = (data + y + x);
          pCV->setColor(*(offset + 2), *(offset + 1), *(offset));

          if (yuvimage == TRUE) {
            pCV->YUVtoRGB();
          }
          pCV++;
        }
      }
    
      if ((cur_x += (4 * pixel_stride)) >= line_length) {
        cur_x = 0;
      }
      return XIL_SUCCESS;
    } else {
      XIL_ERROR(systemState, XIL_ERROR_SYSTEM, "di-95", TRUE);
      return XIL_FAILURE;
    }
}
