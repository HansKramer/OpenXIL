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
//  File:	XilCondVar.cc
//  Project:	XIL
//  Revision:	1.13
//  Last Mod:	10:08:32, 03/10/00
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
#pragma ident	"@(#)XilCondVar.cc	1.13\t00/03/10  "

#include <errno.h>

#include "_XilDefines.h"
#include "_XilCondVar.hh"
#include "XiliUtils.hh"

XilCondVar::XilCondVar(XilMutex* mutex)
{
#if defined(_XIL_USE_PTHREADS)
	int rtn;
    rtn = pthread_cond_init(&condId, NULL);
	if(rtn < 0) {
		xili_print_debug("pthread_cond_init(%d) failed %d %d %s\n",
			condId, rtn, errno, xili_strerror(errno));
	}
#elif defined(_XIL_USE_SOLTHREADS)
    cond_init(&condId, USYNC_THREAD, NULL);
#elif defined(_XIL_USE_WINTHREADS)
    //
    //  Use auto-reset event object if we want to use broadcast feature
    //
    condId = CreateEvent(NULL,        // inheritance by other process
                         FALSE,       // Manual reset event (FALSE == auto)
                         FALSE,       // Initial event state (signaled)
                         NULL);       // event name (no name)
#endif

    localMutex = mutex;
}

XilCondVar::XilCondVar()
{
#if defined(_XIL_USE_PTHREADS)
	int rtn;
    rtn = pthread_cond_init(&condId, NULL);
	if(rtn < 0) {
		xili_print_debug("pthread_cond_init(%d) failed %d %d %s\n",
			condId, rtn, errno, xili_strerror(errno));
	}
#elif defined(_XIL_USE_SOLTHREADS)
    cond_init(&condId, USYNC_THREAD, NULL);
#elif defined(_XIL_USE_WINTHREADS)
    //
    //  Use auto-reset event object if we want to use broadcast feature
    //
    condId = CreateEvent(NULL,        // inheritance by other process
                         FALSE,       // Manual reset event (FALSE == auto)
                         TRUE,        // Initial event state (signaled)
                         NULL);       // event name (no name)
#endif

    localMutex = NULL;
}

XilCondVar::~XilCondVar()
{
#if defined(_XIL_USE_PTHREADS)
    pthread_cond_destroy(&condId);
#elif defined(_XIL_USE_SOLTHREADS)
    cond_destroy(&condId);
#elif defined(_XIL_USE_WINTHREADS)
    CloseHandle(condId);
#endif
}

void
XilCondVar::wait()
{
#if defined(_XIL_USE_PTHREADS)
	int rtn;
    rtn = pthread_cond_wait(&condId, &localMutex->mutexId);
	if(rtn < 0) {
		xili_print_debug("pthread_cond_wait(%d, %d) failed %d %d %s\n",
			condId, localMutex->mutexId, rtn, errno, xili_strerror(errno));
	}
#elif defined(_XIL_USE_SOLTHREADS)
    cond_wait(&condId, &localMutex->mutexId);
#elif defined(_XIL_USE_WINTHREADS)
    //
    //  TODO :: This NT version of wait has could casuse problems
    //          under race conditions. We are trying to lock the
    //          mutex after getting a signal. This is not atomic
    //          as it should be. Need to find some solution
    //
#ifndef _WINDOWS_95_PORTABLE
    SignalObjectAndWait(localMutex->mutexId, condId, INFINITE, FALSE);
#else
    localMutex->unlock();
    WaitForSingleObject(condId, INFINITE);
#endif // !_WINDOWS_95_PORTABLE
    localMutex->lock();
#endif // _XIL_USE_PTHREADS
}

void
XilCondVar::signal()
{
#if defined(_XIL_USE_PTHREADS)
	int rtn;
    rtn = pthread_cond_signal(&condId);
	if(rtn < 0) {
		xili_print_debug("pthread_cond_signal(%d) failed %d %d %s\n",
			condId, rtn, errno, xili_strerror(errno));
	}
#elif defined(_XIL_USE_SOLTHREADS)
    cond_signal(&condId);
#elif defined(_XIL_USE_WINTHREADS)
    PulseEvent(condId);              // set&resets event for only one thread
#endif
}

void
XilCondVar::broadcast()
{
#if defined(_XIL_USE_PTHREADS)
	int rtn;
    rtn = pthread_cond_broadcast(&condId);
	if(rtn < 0) {
		xili_print_debug("pthread_cond_broadcast(%d) failed %d %d %s\n",
			condId, rtn, errno, xili_strerror(errno));
	}
#elif defined(_XIL_USE_SOLTHREADS)
    cond_broadcast(&condId);
#elif defined(_XIL_USE_WINTHREADS)
    SetEvent(condId);                // set event to awake all threads
#endif
}

void
XilCondVar::wait(XilMutex* mutex)
{
#if defined(_XIL_USE_PTHREADS)
	int rtn;
    rtn = pthread_cond_wait(&condId, &mutex->mutexId);
	if(rtn < 0) {
		xili_print_debug("pthread_cond_wait(%d. %d) failed %d %d %s\n",
			condId, mutex->mutexId, rtn, errno, xili_strerror(errno));
	}
#elif defined(_XIL_USE_SOLTHREADS)
    cond_wait(&condId, &mutex->mutexId);
#elif defined(_XIL_USE_WINTHREADS)
    //
    //  TODO :: This NT version of wait has could casuse problems
    //          under race conditions. We are trying to lock the
    //          mutex after getting a signal. This is not atomic
    //          as it should be. Need to find some solution
    //
#ifndef _WINDOWS_95_PORTABLE
    SignalObjectAndWait(mutex->mutexId, condId, INFINITE, FALSE);
#else
    mutex->unlock();
    WaitForSingleObject(condId, INFINITE);
#endif // !_WINDOWS_95_PORTABLE
    mutex->lock();
#endif // _XIL_USE_PTHREADS
}
