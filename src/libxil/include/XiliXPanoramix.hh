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

#ifndef _XILI_XPANORAMIX_H_
#define _XILI_XPANORAMIX_H_

#include <X11/Xlib.h>

typedef struct {
    Window  window;         /* PanoramiX window - may not exist */
    int     screen;
    int     State;          /* PanoramiXOff, PanoramiXOn */
    int     width;          /* width of this screen */
    int     height;         /* height of this screen */
    int     ScreenCount;    /* real physical number of screens */
    XID     eventMask;      /* selected events for this client */
} XPanoramiXInfo;

#ifdef __cplusplus
extern "C" {
#endif

Bool 
XPanoramiXQueryExtension(Display* display,
                         int*     event_base,
                         int*     error_base);

Status 
XPanoramiXQueryVersion(Display* display,
                       int*     major_version,
                       int*     minor_version);

Status 
XPanoramiXGetState(Display*        display,
                   Drawable        drawable,
                   XPanoramiXInfo* panoramiX_info);

Status 
XPanoramiXGetScreenCount(Display*        display,
                         Drawable        drawable,
                         XPanoramiXInfo* panoramiX_info);

Status 
XPanoramiXGetScreenSize(Display*        display,
                        Drawable        drawable,
                        int             screen_num,
                        XPanoramiXInfo* panoramiX_info);

XPanoramiXInfo*
XPanoramiXAllocInfo(void);

#ifdef __cplusplus
}
#endif

#endif /* _XILI_XPANORAMIX_H_ */
