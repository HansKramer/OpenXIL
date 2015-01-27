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
//  File:	XiliReadWrite.hh
//  Project:	XIL
//  Revision:	1.8
//  Last Mod:	10:21:42, 03/10/00
//
//  Description:
//	Abstracts a read/write thread locking primitive.  Used for
//      multiple readers, multiple writers.
//	
//  MT-level:  Safe
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliReadWrite.hh	1.8\t00/03/10  "

#ifndef _XILI_READ_WRITE_HH
#define _XILI_READ_WRITE_HH

//
//  System Includes
//
#if defined(_XIL_USE_PTHREADS)
#include <pthread.h>
#elif defined(_XIL_USE_SOLTHREADS)
#include <thread.h>
#elif defined(_XIL_USE_WINTHREADS)
#include <windows.h>
#endif

//
//  C++ Includes
//
#include "_XilDefines.h"

class XiliReadWrite {
public:
    //
    //  Constructor/Destructor
    //
                      XiliReadWrite()
    {
#if defined(_XIL_USE_PTHREADS)
        pthread_mutex_init(&mutexId, NULL);
#elif defined(_XIL_USE_SOLTHREADS)
        rwlock_init(&rwLockId, USYNC_THREAD, NULL);
#elif defined(_XIL_USE_WINTHREADS)
        mutexId = CreateMutex(NULL, FALSE, NULL);
#endif
    }

                      ~XiliReadWrite()
    {
#if defined(_XIL_USE_PTHREADS)
        pthread_mutex_destroy(&mutexId);
#elif defined(_XIL_USE_SOLTHREADS)
        //
        //  TODO: jlf 12/6/96  There is a bug in the 2.6 thread library that
        //                     causes this to SEGV.
        //
        //rwlock_destroy(&rwLockId);
#elif defined(_XIL_USE_WINTHREADS)
        CloseHandle(mutexId);
#endif
    }
                          

    //
    //  The primitives.
    //
    void              readLock()
    {
#if defined(_XIL_USE_PTHREADS)
        pthread_mutex_lock(&mutexId);
#elif defined(_XIL_USE_SOLTHREADS)
        rw_rdlock(&rwLockId);
#elif defined(_XIL_USE_WINTHREADS)
        WaitForSingleObject(mutexId, INFINITE);
#endif
    }        

    void              writeLock()
    {
#if defined(_XIL_USE_PTHREADS)
        pthread_mutex_lock(&mutexId);
#elif defined(_XIL_USE_SOLTHREADS)
        rw_wrlock(&rwLockId);
#elif defined(_XIL_USE_WINTHREADS)
        WaitForSingleObject(mutexId, INFINITE);
#endif
    }
        
    void              unlock()
    {
#if defined(_XIL_USE_PTHREADS)
        pthread_mutex_unlock(&mutexId);
#elif defined(_XIL_USE_SOLTHREADS)
        rw_unlock(&rwLockId);
#elif defined(_XIL_USE_WINTHREADS)
        ReleaseMutex(mutexId);
#endif
    }
    
    XilStatus         tryReadLock()
    {
#if defined(_XIL_USE_PTHREADS)
        return pthread_mutex_trylock(&mutexId) ? XIL_FAILURE : XIL_SUCCESS;
#elif defined(_XIL_USE_SOLTHREADS)
        return rw_tryrdlock(&rwLockId) ? XIL_FAILURE : XIL_SUCCESS;
#elif defined(_XIL_USE_WINTHREADS)
        if(WaitForSingleObject(mutexId, 0) == WAIT_OBJECT_0) {
            return XIL_SUCCESS;
        } else {
            return XIL_FAILURE;
        }
#else
        return XIL_SUCCESS;
#endif
    }

    XilStatus         tryWriteLock()
    {
#if defined(_XIL_USE_PTHREADS)
        return pthread_mutex_trylock(&mutexId) ? XIL_FAILURE : XIL_SUCCESS;
#elif defined(_XIL_USE_SOLTHREADS)
        return rw_trywrlock(&rwLockId) ? XIL_FAILURE : XIL_SUCCESS;
#elif defined(_XIL_USE_WINTHREADS)
        if(WaitForSingleObject(mutexId, 0) == WAIT_OBJECT_0) {
            return XIL_SUCCESS;
        } else {
            return XIL_FAILURE;
        }
#else
        return XIL_SUCCESS;
#endif
    }
    
private:
#if defined(_XIL_USE_PTHREADS)
    pthread_mutex_t   mutexId;
#elif defined(_XIL_USE_SOLTHREADS)
    rwlock_t          rwLockId;
#elif defined(_XIL_USE_WINTHREADS)
    HANDLE            mutexId;
#endif
};

#endif // _XIL_READ_WRITE_HH
