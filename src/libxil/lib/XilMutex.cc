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
//  File:	XilMutex.cc
//  Project:	XIL
//  Revision:	1.16
//  Last Mod:	10:07:58, 03/10/00
//
//  Description:
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilMutex.cc	1.16\t00/03/10  "

//
//  XilMutexPrivate.hh has copies of these routines.  For libxil.so.1, we want
//  to inline the calls to remove the extra function overhead so we copy this
//  file into XilMutexPrivate.hh and put "inline" before every function.  When
//  you update this file, you must copy the changes into XilMutexPrivate.hh
//
//  This define indicates to the private include NOT to define the inlines so
//  the symbols are created in this file and made available to the GPI.
//
#define _XIL_PRIVATE_MUTEX_CC

#include <errno.h>

#include "_XilDefines.h"
#include "_XilMutex.hh"

#include "XiliUtils.hh"

XilMutex::XilMutex()
{
#if defined(_XIL_USE_PTHREADS)
	int rtn;
    rtn = pthread_mutex_init(&mutexId, NULL);
	if(rtn < 0) {
		xili_print_debug("pthread_mutex_init(%d) failed %d %d %s\n",
			mutexId, rtn, errno, xili_strerror(errno));
	}
#elif defined(_XIL_USE_SOLTHREADS)
    mutex_init(&mutexId, USYNC_THREAD, NULL);
#elif defined(_XIL_USE_WINTHREADS)
    mutexId = CreateMutex(NULL, FALSE, NULL); 
#endif    
}

XilMutex::~XilMutex()
{
#if defined(_XIL_USE_PTHREADS)
    pthread_mutex_destroy(&mutexId);
#elif defined(_XIL_USE_SOLTHREADS)
    mutex_destroy(&mutexId);
#elif defined(_XIL_USE_WINTHREADS)
    CloseHandle(mutexId);
#endif
}

//
//  void is returned by these routines because it's not clear how one
//  can recover if the lock or unlock fails and it almost never fails.
//
void
XilMutex::lock()
{
#if defined(_XIL_USE_PTHREADS)
	int rtn;
    rtn = pthread_mutex_lock(&mutexId);
	if(rtn < 0) {
		xili_print_debug("pthread_mutex_lock(%d) failed %d %d %s\n",
			mutexId, rtn, errno, xili_strerror(errno));
	}
#elif defined(_XIL_USE_SOLTHREADS)
    mutex_lock(&mutexId);
#elif defined(_XIL_USE_WINTHREADS)
    DWORD dwRet;
    dwRet = WaitForSingleObject(mutexId, INFINITE);
#endif
}

void
XilMutex::unlock()
{
#if defined(_XIL_USE_PTHREADS)
	int rtn;
    rtn = pthread_mutex_unlock(&mutexId);
	if(rtn < 0) {
		xili_print_debug("pthread_mutex_unlock(%d) failed %d %d %s\n",
			mutexId, rtn, errno, xili_strerror(errno));
	}
#elif defined(_XIL_USE_SOLTHREADS)
    mutex_unlock(&mutexId);
#elif defined(_XIL_USE_WINTHREADS)
    DWORD dwRet;
    dwRet = ReleaseMutex(mutexId);
#endif
}

XilStatus
XilMutex::tryLock()
{
#if defined(_XIL_USE_PTHREADS)
    if(!pthread_mutex_trylock(&mutexId)) {
        return XIL_SUCCESS;
    } else {
        return XIL_FAILURE;
    }
#elif defined(_XIL_USE_SOLTHREADS)
    if(!mutex_trylock(&mutexId)) {
        return XIL_SUCCESS;
    } else {
        return XIL_FAILURE;
    }
#elif defined(_XIL_USE_WINTHREADS)
    if (WaitForSingleObject(mutexId, 0) == WAIT_OBJECT_0) {
        return XIL_SUCCESS;
    } else {
        return XIL_FAILURE;
    }
#else
    return XIL_SUCCESS;
#endif
}
