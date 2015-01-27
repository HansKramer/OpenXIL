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
//  File:	XiliThread.hh
//  Project:	XIL
//  Revision:	1.11
//  Last Mod:	10:21:11, 03/10/00
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
#pragma ident	"@(#)XiliThread.hh	1.11\t00/03/10  "


#if defined(_XIL_USE_PTHREADS)
#include <signal.h>
#include <pthread.h>
#elif defined(_XIL_USE_SOLTHREADS)
#include <signal.h>
#include <thread.h>
#elif defined(_XIL_USE_WINTHREADS)
#include <process.h>
#endif

#include "_XilDefines.h"

#ifndef _XILI_THREAD_HH
#define _XILI_THREAD_HH

typedef void* (*XiliThreadFunc) (void*);

#if defined(_XIL_USE_PTHREADS)
typedef pthread_t		XilThreadId;
#elif defined(_XIL_USE_SOLTHREADS)
typedef thread_t		XilThreadId;
#elif defined(_XIL_USE_WINTHREADS)
typedef HANDLE			XilThreadId;
#else
typedef unsigned int		XilThreadId;
#endif

class XiliThread {
public:
    XilThreadId          getId();

    static XilThreadId   self();
    
    XilStatus            join();

    //
    //  Send the given signal to the thread.
    //
    XilStatus           kill(int sig);
    
    Xil_boolean         isOK();
    
                        XiliThread(XiliThreadFunc start_function,
                                   void*          args);

                        ~XiliThread();
    
#if 0
    XilStatus           suspend();
    XilStatus           continue();
#endif

private:
    XilThreadId         threadId;
    Xil_boolean         isOKFlag;
};

#endif // _XILI_THREAD_HH
