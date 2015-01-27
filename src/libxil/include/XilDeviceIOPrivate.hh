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
//  File:	XilDeviceIOPrivate.hh
//  Project:	XIL
//  Revision:	1.6
//  Last Mod:	10:21:36, 03/10/00
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
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilDeviceIOPrivate.hh	1.6\t00/03/10  "

#ifdef _XIL_PRIVATE_DATA

public:
    //
    //  All data must live in public header file since this class is derived
    //  from in the GPI 

    //
    //  Set the op which is currently in the process of a capture on this
    //  device.   It is used by giveData() to provide the op with the data to
    //  which is ready to be processed.
    //
    void           setCaptureOpInfo(XilOp*        capture_op,
                                    XilOp*        bottom_op,
                                    XilTileNumber bottom_tile)
    {
        captureOp        = capture_op;
        bottomOp         = bottom_op;
        bottomTileNumber = bottom_tile;
    }

    //
    //  Set the controlling image which represents this device.
    //
    void           setControllingImage(XilImage* controlling_image)
    {
        controllingImage = controlling_image;
    }

    XilImage*      getControllingImage()
    {
        return controllingImage;
    }

    //
    //  Mark this as a framebuffer device. 
    //
    void           markAsFramebufferDevice()
    {
        framebufferDeviceFlag = TRUE;
    }

    //
    //  Test whether this as a framebuffer device. 
    //
    Xil_boolean    isFramebufferDevice()
    {
        return framebufferDeviceFlag;
    }

    //
    //  Mark this as a device which supports double buffering.
    //
    void           markAsDoubleBufferingDevice()
    {
        doubleBufferingDeviceFlag = TRUE;
    }
    
    Xil_boolean    isDoubleBufferingDevice()
    {
        return doubleBufferingDeviceFlag;
    }

    //
    //  Mark this as a device which supports stereo
    //
    void           markAsStereoDevice()
    {
        stereoDeviceFlag = TRUE;
    }
    
    Xil_boolean    isStereoDevice()
    {
        return stereoDeviceFlag;
    }


#endif // _XIL_PRIVATE_DATA
