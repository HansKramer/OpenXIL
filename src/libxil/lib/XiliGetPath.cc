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
//  File:	XiliGetPath.cc
//  Project:	XIL
//  Revision:	1.26
//  Last Mod:	10:08:07, 03/10/00
//
//  MT-level:   Safe
//	
//  Description:
//	This function returns a full pathname for accessing the
//      specified requested directory in the XIL directory structure.
//
//	The function uses the following criteria for determining what
//      to prepend to the given path:
//         o If XILHOME is set, it is treated as the only directory for
//           finding pipelines and other XIL Files.
//	
//	   o If XILHOME is NOT set, then this function looks in two  
//         places for XIL information on Solaris:
//             -  /etc/openwin/lib/xil
//             -  /usr/openwin/lib/xil
//
//         and on non-Solaris platforms, it looks in
//             -  /usr/lib/xil
//             -  /usr/local/lib/xil
//             -  /opt/xil
//
//             -  /usr/local/xil
//             -  /opt/xil
//             -  /usr/local/xil
//             -  /usr/local
//             -  /usr/lib/xil
//             -  /usr/local/lib/xil
//             -  /opt/xil
//
//
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliGetPath.cc	1.26\t00/03/10  "

#include <stdlib.h>
#ifdef _WINDOWS
#include <io.h>
#else
#include <unistd.h>
#endif
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "_XilDefines.h"
#include "_XilMutex.hh"
#include "XiliUtils.hh"

static XilMutex  static_xil_dirs_mutex;
static char**    static_xil_dirs = NULL;

#if defined(SOLARIS)
#define NUM_STD_XIL_PATHS     2

static char*     std_xil_dirs[NUM_STD_XIL_PATHS] = { "/usr/local",
                                                     NULL};
#elif defined(_WINDOWS)
#define NUM_STD_XIL_PATHS     3
static char*     std_xil_dirs[NUM_STD_XIL_PATHS] = { "C:\\Program Files\\XIL",
                                                     "D:\\Program Files\\XIL",
                                                     NULL};
#else
#define NUM_STD_XIL_PATHS     8

static char*     std_xil_dirs[NUM_STD_XIL_PATHS] = { "/usr/local",
                                                     "/opt/synarc/xil",
                                                     "/usr/local/xil",
                                                     "/usr/local",
                                                     "/usr/lib/xil",
                                                     "/usr/local/lib/xil",
                                                     "/opt/xil",
                                                     NULL};
#endif

#if !(defined(SOLARIS) && defined(_XIL_RELEASE_BUILD))
static char*     static_xilreldir[2];
#endif

char*
XiliGetPath(const char* sub_path)
{
    //
    //  If we haven't set what's in XILHOME yet, do this now so we
    //  don't have to keep calling getenv(). Note that XILHOME is
    //  ignored on for release builds on Solaris and Win32.
    //
    static_xil_dirs_mutex.lock();
    if(static_xil_dirs == NULL) {
#ifndef _XIL_RELEASE_BUILD
        //
        //  Has XILHOME been set?
        //
        char* xilreldir_str = getenv("XILHOME");
        if(xilreldir_str) {
            //
            //  We must make copy since getenv() returns a pointer to
            //  static data.
            //
            static_xilreldir[0] = strdup(xilreldir_str);
            static_xilreldir[1] = NULL;

            static_xil_dirs = (char**)static_xilreldir;
        } else {
#endif // !_XIL_RELEASE_BUILD
#ifdef _WINDOWS
            if(xilreldir_str = xili_get_install_path()) {
                static_xilreldir[0] = strdup(xilreldir_str);
                static_xilreldir[1] = NULL;

                static_xil_dirs = (char**)static_xilreldir;
           } else {
#endif // _WINDOWS
            static_xil_dirs = (char**)std_xil_dirs;
#ifdef _WINDOWS
        }
#endif
#ifndef _XIL_RELEASE_BUILD
        }
#endif
    }
    static_xil_dirs_mutex.unlock();

    //
    //  We must allocate our buffer space off the heap because if it
    //  were static, it would require too much locking.
    //
    char   tmp_buf[XILI_PATH_MAX];
    char*  ret_path = NULL;
    char** tmppath = static_xil_dirs;
    while(*tmppath != NULL) {
        //
        //  Try a directory...
        //
        sprintf(tmp_buf, "%s/%s", *tmppath, sub_path);

        if(access(tmp_buf, F_OK) != -1) {
            //
            //  Return a new string with the new path.
            //
            ret_path = strdup(tmp_buf);
            break;
        }

        tmppath++;
    }

    return ret_path;
}
