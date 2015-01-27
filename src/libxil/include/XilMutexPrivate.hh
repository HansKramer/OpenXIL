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
//  File:	XilMutexPrivate.hh
//  Project:	XIL
//  Revision:	1.10
//  Last Mod:	10:21:58, 03/10/00
//
//  Description:
//	
//	
//	
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilMutexPrivate.hh	1.10\t00/03/10  "

//
//  Private Includes
//
#ifdef  _XIL_PRIVATE_INCLUDES

#if defined(_XIL_USE_PTHREADS)
#include <pthread.h>
#elif defined(_XIL_USE_SOLTHREADS)
#include <thread.h>
#elif defined(_XIL_USE_WINTHREADS)
#include <windows.h>
#endif

#endif

//
//  Private data
//
#ifdef _XIL_PRIVATE_DATA

#if defined(_XIL_USE_PTHREADS)
    pthread_mutex_t   mutexId;
#elif defined(_XIL_USE_SOLTHREADS)
    mutex_t           mutexId;
#elif defined(_XIL_USE_WINTHREADS)
    HANDLE            mutexId;
#endif

    //
    //  The size of this is 64 bytes.  If you add data members to this 
    //  class you must add to the count to be subtracted from 64.
    // 
#if defined(_XIL_USE_PTHREADS)
    unsigned char      extraData[64 - sizeof(pthread_mutex_t)];
#elif defined(_XIL_USE_SOLTHREADS)
    unsigned char      extraData[64 - sizeof(mutex_t)];
#elif defined(_XIL_USE_WINTHREADS)
    unsigned char      extraData[64 - sizeof(HANDLE)];
#else
    unsigned long      extraData[64/sizeof(unsigned long)];
#endif

    friend class XilCondVar;

#endif // _XIL_PRIVATE_DATA

//
//  Private function implementations -- borrowed from XilMutex.cc so we can
//  inline them into libxil.so.1.
//
//  Windows doesn't seem to make the following as inlines
//  Make modifications in XilMutex.cc if any made here for windows
#ifndef _WINDOWS
#if defined(_XIL_PRIVATE_FUNCTIONS) && ! defined(_XIL_PRIVATE_MUTEX_CC)

inline
XilMutex::XilMutex()
{
#if defined(_XIL_USE_PTHREADS)
    pthread_mutex_init(&mutexId, NULL);
#elif defined(_XIL_USE_SOLTHREADS)
    mutex_init(&mutexId, USYNC_THREAD, NULL);
#elif defined(_XIL_USE_WINTHREADS)
    mutexId = CreateMutex(NULL, FALSE, NULL);
#endif    
}

inline
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
inline
void
XilMutex::lock()
{
#if defined(_XIL_USE_PTHREADS)
    pthread_mutex_lock(&mutexId);
#elif defined(_XIL_USE_SOLTHREADS)
    mutex_lock(&mutexId);
#elif defined(_XIL_USE_WINTHREADS)
    WaitForSingleObject(mutexId, INFINITE);
#endif
}

inline
void
XilMutex::unlock()
{
#if defined(_XIL_USE_PTHREADS)
    pthread_mutex_unlock(&mutexId);
#elif defined(_XIL_USE_SOLTHREADS)
    mutex_unlock(&mutexId);
#elif defined(_XIL_USE_WINTHREADS)
    ReleaseMutex(mutexId);
#endif
}

inline
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

#endif /* _WINDOWS */

#endif // _XIL_PRIVATE_FUNCTIONS
