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
//  File:	XiliThread.cc
//  Project:	XIL
//  Revision:	1.25
//  Last Mod:	10:08:35, 03/10/00
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
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliThread.cc	1.25\t00/03/10  "

#include "_XilDefines.h"
#include "_XilSystemState.hh"
#include "XiliThread.hh"

#if defined(_XIL_USE_PTHREADS) && defined(IRIX)
/*
 * IRIX pthreads has unresolved globals - making them here in the
 * hope of makeing some progress
 */
unsigned __us_rsthread_pmq[256];
#endif

XiliThread::XiliThread(XiliThreadFunc start_function,
                       void*          args)
{
    isOKFlag = FALSE;

#if defined(_XIL_USE_PTHREADS)
    if(pthread_create(&threadId, NULL, start_function, args) != 0) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-411", TRUE);
        return;
    }
#elif defined(_XIL_USE_SOLTHREADS)
    if(thr_create(NULL, 0, start_function, args, 0, &threadId) != 0) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-411", TRUE);
        return;
    }
#elif defined(_XIL_USE_WINTHREADS)

#ifdef CREATE_THREAD
typedef DWORD (WINAPI *WinThreadFunc)(void *);

    DWORD threadPid;

    threadId = CreateThread(NULL,                          // security attrib
                            0,                             // Stack Size
                            (WinThreadFunc)start_function, // start func.
                            args,                          // args
                            0,                             // creation flags
                            &threadPid);                   // thread identifier
#else
typedef unsigned int (WINAPI *WinThreadFunc)(void *);

    unsigned int  threadPid;

    threadId = (XilThreadId) \
               _beginthreadex(NULL,                        // security attrib
                            0,                             // Stack Size
                            (WinThreadFunc)start_function, // start func.
                            args,                          // args
                            0,                             // creation flags
                            &threadPid);                   // thread identifier
#endif  // CREATE_THREAD
    if(!threadId) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-411", TRUE);
        return;
    }

#else
    threadId = 0;
    //
    //  Just call the function.
    //
    (*start_function)(args);
#endif // _XIL_USE_PTHREADS


    isOKFlag = TRUE;
}

XiliThread::~XiliThread()
{
}
    
Xil_boolean
XiliThread::isOK()
{
    _XIL_ISOK_TEST();
}
                        
XilThreadId 
XiliThread::getId()
{
    return threadId;         // thread handle and NOT THREAD IDENTIFIER !!!
}

XilThreadId 
XiliThread::self()
{
#if defined(_XIL_USE_PTHREADS)
    return pthread_self();
#elif defined(_XIL_USE_SOLTHREADS)
    return thr_self();
#elif defined(_XIL_USE_WINTHREADS)
    //
    // thrd identifier instead of thrd handle, check with John Furlani
    //
    return ((XilThreadId) GetCurrentThreadId());
#else
    return 0;
#endif
}

XilStatus
XiliThread::join()
{
#if defined(_XIL_USE_PTHREADS)
    void*    status;
    pthread_join(threadId, &status);
#elif defined(_XIL_USE_SOLTHREADS)
    thread_t departed;
    void*    status;
    
    thr_join(threadId, &departed, &status);
#elif defined(_XIL_USE_WINTHREADS)
    if (WaitForSingleObject(threadId, INFINITE) == WAIT_OBJECT_0) {
        DWORD  dwStatus;

        GetExitCodeThread(threadId, &dwStatus);
    }
#endif
    return XIL_SUCCESS;
}

XilStatus
XiliThread::kill(int sig)
{
#if defined(_XIL_USE_PTHREADS)
    pthread_kill(threadId, sig);
#elif defined(_XIL_USE_SOLTHREADS)
    thr_kill(threadId, sig);
#elif defined(_XIL_USE_WINTHREADS)
    TerminateThread(threadId, sig);
#else
    //
    //  So it appears used...
    //
    sig = 0;
#endif
    return XIL_SUCCESS;
}
