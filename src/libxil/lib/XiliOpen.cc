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
//  File:	XiliOpen.cc
//  Project:	XIL
//  Revision:	1.21
//  Last Mod:	10:07:57, 03/10/00
//
//  Description:
//	
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliOpen.cc	1.21\t00/03/10  "

//
//  System Includes
//
#ifndef _WINDOWS
#include <unistd.h>
#if !defined(HPUX) && !defined(IRIX)
#include <libintl.h>
#endif
#include <locale.h>
#include <string.h>
#endif

//
//  C++ Includes
//
#include "_XilDefines.h"
#include "_XilMutex.hh"
#include "_XilGlobalState.hh"

#ifndef GCC
#ifdef SOLARIS
//
//  For DevPro C++, I've copied the declaration of set_new_handler() from
//  new.h which lives in the compiler distribution.
//
typedef void (*_Pvf)();
_Pvf set_new_handler(_Pvf);
#endif
#endif

static XilMutex    xil_initialized_mutex;

XilSystemState*
XiliOpen()
{
    XilGlobalState* xgs;

    //
    //  All of this initialization only needs to occur when we're not
    //  initialized (i.e. the very first xil_open() by an application or the
    //  first after all the outstanding system states were closed).
    //
    xil_initialized_mutex.lock();

#ifndef GCC 
#ifdef SOLARIS
    //
    //  For DevPro C++, we need to call set_new_handler() with NULL in order
    //  to stop new from throwing an exception when an error occurs.
    //
    set_new_handler(NULL);
#endif
#endif

    if(XilGlobalState::theXGS == NULL) {
        //
        //  Initialize Error reporting...
        //
        char* locale_path = XiliGetPath("locale");
        if(locale_path == NULL) {
            //
            //  This is a pretty funny situation. If this error gets
            //  translated so that the user can read it then there really
            //  isn't a problem worth reporting since the error information
            //  is in the default directory
            //
            XIL_ERROR(NULL, XIL_ERROR_SYSTEM, "di-247", FALSE);

            //
            // At this point there isn't much point in continuing since if
            // XiliGetPath() failed there isn't even a couple hundred bytes
            // left but since the particular failure isn't fatal it
            // wouldn't be right to give up yet.
            //
        } else {
#ifdef SOLARIS
            bindtextdomain("xil", locale_path);
#endif
        }
        free(locale_path);

        //
        //  NOTE:  Unlike XIL 1.2, we do not reset the locale any more.
        //         XIL 1.3 generates the full messages and does not rely on
        //         the locale handling mechanism to translate from di-??? to
        //         english. 
        //

        //
        //  Actually get and setup the global state
        //
        xgs = XilGlobalState::getXilGlobalState();
        if(xgs == NULL || xgs->isOK() == FALSE) {
            xil_initialized_mutex.unlock();
            return NULL;
        }
        
        //
        //  Only needs to be done once.  If we're here, we need to do a full
        //  initialization from the very start so indicate that to
        //  loadDependents().
        //
        if(xgs->loadDependents(TRUE) == XIL_FAILURE) {
            XIL_ERROR(NULL,XIL_ERROR_CONFIGURATION,"di-248",FALSE);
            xil_initialized_mutex.unlock();
            return NULL;
        }
    } else {
        //
        //  Just set our local reference to the global state.
        //
        xgs = XilGlobalState::theXGS;
    }        
    xil_initialized_mutex.unlock();

    //
    //  Return the system state.
    //
    return xgs->createSystemState();
}
