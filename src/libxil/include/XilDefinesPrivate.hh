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
//  File:	XilDefinesPrivate.hh
//  Project:	XIL
//  Revision:	1.46
//  Last Mod:	10:20:49, 03/10/00
//
//  Description:
//	
//	Internal libxil definitions.  These are never seen by an
//	application.
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#ifdef _WINDOWS
#pragma warning ( disable : 4068 )
#else
#pragma ident	"@(#)XilDefinesPrivate.hh	1.46\t00/03/10  "
#endif

//
//  Error Checking Macro Definitions
//
#define _XIL_TEST_FOR_NULL_THIS(ret_val, error_code) \
    if(this == NULL) { \
        XIL_ERROR(NULL, XIL_ERROR_USER, error_code, TRUE); \
        return ret_val; \
    }

#define _XIL_TEST_FOR_NULL_THIS_VOID(error_code) \
    if(this == NULL) { \
        XIL_ERROR(NULL, XIL_ERROR_USER, error_code, TRUE); \
        return; \
    }

#define _XIL_TEST_FOR_NULL_PTR(ret_val, error_code, obj) \
    if(obj == NULL) { \
        XIL_ERROR(NULL, XIL_ERROR_USER, error_code, TRUE); \
        return ret_val; \
    }

#define _XIL_TEST_FOR_NULL_PTR_VOID(error_code, obj) \
    if(obj == NULL) { \
        XIL_ERROR(NULL, XIL_ERROR_USER, error_code, TRUE); \
        return; \
    }

#define _XIL_ISOK_TEST() \
    if(this == NULL) { \
        return FALSE; \
    } else { \
        if(isOKFlag == TRUE) { \
            return TRUE; \
        } else { \
            delete this; \
            return FALSE; \
        } \
    }

#define _XIL_ISOK_TEST_VOID() \
    if(this == NULL) { \
        return; \
    } else { \
        if(isOKFlag == TRUE) { \
            return; \
        } else { \
            delete this; \
            return; \
        } \
    }

//
//  Enable the use of TNF tracing/probing based on _XIL_HAS_TNF_PROBES 
//  (for the platform) and _XIL_ENABLE_PROBES (for this build).
//
#ifdef  _XIL_HAS_TNF_PROBES

#ifndef _XIL_ENABLE_PROBES
//
//  Per TNF_PROBE(3X), if NPROBE is defined, the probes will be disabled. 
//
#define NPROBE
#endif

#include <tnf/probe.h>
#else
//
//  A platform that doesn't have TNF probes -- so we have to disable them
//  ourselves...this is all of the defines used in XIL -- there are other
//  probe calls in <tnf/probe.h> that we don't disable here.
//
#define TNF_PROBE_0(namearg, keysarg, detail) \
                ((void)0)
#define TNF_PROBE_1(namearg, keysarg, detail, type_1, namearg_1, valarg_1) \
                ((void)0)
#define TNF_PROBE_2(namearg, keysarg, detail, type_1, namearg_1, valarg_1, type_2, namearg_2, valarg_2) \
                ((void)0)
#define TNF_PROBE_3(namearg, keysarg, detail, type_1, namearg_1, valarg_1, type_2, namearg_2, valarg_2, type_3, namearg_3, valarg_3) \
                ((void)0)
#define TNF_PROBE_4(namearg, keysarg, detail, type_1, namearg_1, valarg_1, type_2, namearg_2, valarg_2, type_3, namearg_3, valarg_3, type_4, namearg_4, valarg_4) \
                ((void)0)
#define TNF_PROBE_5(namearg, keysarg, detail, type_1, namearg_1, valarg_1, type_2, namearg_2, valarg_2, type_3, namearg_3, valarg_3, type_4, namearg_4, valarg_4, type_5, namearg_5, valarg_5) \
                ((void)0)
#endif // HAS_TNF_PROBES

#include "_XilClasses.hh"
#include "_XilGPIDefines.hh"

//

//
//  Some internal defines...
//
#define  XIL_PIPELINE_VERSION        3
#define  XIL_DEFAULT_COMPANY_NAME    "SunSoft"
#define  XIL_DEFAULT_CLASSIFICATION  "XIL"

//
//  Define the primary I/O device we'll try.
//
#ifdef _WINDOWS
#define _XILI_PRIMARY_IO_DISPLAY "SUNWMSDX"        // DIRECTX pipeline
#else
#define _XILI_PRIMARY_IO_DISPLAY "SUNWxshm"
#endif

//
//  Define the default I/O display device if other devices fail.
//
#ifdef _WINDOWS
#define _XILI_DEFAULT_IO_DISPLAY "SUNWMSWin"        // GDI pipeline
#else
#define _XILI_DEFAULT_IO_DISPLAY "SUNWxlib"
#endif

//
//  Macros to fill in the new and delete operators for a class which
//    uses the freeList mechanism for accelerating data allocation and
//    deallocation.
//
//  Use _XIL_OVERLOAD_NEW_AND_DELETE to indicate whether (some) classes should
//  overload new and delete or not. 
//
#if !defined(GCC) && !defined(_WINDOWS) && !defined(HPUX)
#define _XIL_OVERLOAD_NEW_AND_DELETE
#endif

#ifdef  _XIL_OVERLOAD_NEW_AND_DELETE

#include <stdio.h>

#define _XIL_NEW_DELETE_OVERLOAD_PUBLIC(className)    \
    void*        operator new (size_t);               \
    void         operator delete (void* ptr, size_t);

#define _XIL_NEW_DELETE_OVERLOAD_PRIVATE(className)  \
    static className*       freeList;                \
    static XilMutex         freeListMutex;           \
    static unsigned int     freeListCount;           \
    className*              nextFree;



#define _XIL_NEW_DELETE_OVERLOAD_CC_FILE(className, maxFreeListSize, blockSize) \
                                                                         \
className*       className::freeList= NULL;                              \
XilMutex         className::freeListMutex;                               \
unsigned int     className::freeListCount = 0;                           \
                                                                         \
void*                                                                    \
className::operator new(size_t)                                          \
{                                                                        \
    className* entry;                                                    \
                                                                         \
    freeListMutex.lock();                                                \
                                                                         \
    if(freeListCount > maxFreeListSize) {                                \
        entry = ::new className;                                         \
        entry->nextFree = (className*)-1;                                \
    } else {                                                             \
        entry = freeList;                                                \
                                                                         \
        if(entry == NULL) {                                              \
            className* tmp = ::new className[blockSize];                 \
                                                                         \
            for(entry=freeList=&tmp[blockSize-1]; tmp<entry; entry--) {  \
                entry->nextFree = entry-1;                               \
            }                                                            \
            entry->nextFree = NULL;                                      \
                                                                         \
            entry = freeList;                                            \
                                                                         \
            freeListCount += blockSize;                                  \
        }                                                                \
                                                                         \
        freeList = entry->nextFree;                                      \
    }                                                                    \
                                                                         \
    freeListMutex.unlock();                                              \
                                                                         \
    return entry;                                                        \
}                                                                        \
                                                                         \
void                                                                     \
className::operator delete(void* ptr, size_t)                            \
{                                                                        \
    freeListMutex.lock();                                                \
                                                                         \
    if(((className*)ptr)->nextFree == (className*)-1) {                  \
        ::delete ptr;                                                    \
    } else {                                                             \
        ((className*)ptr)->nextFree   = freeList;                        \
        freeList                      = (className*)ptr;                 \
    }                                                                    \
                                                                         \
    freeListMutex.unlock();                                              \
}

#define _XIL_NEW_DELETE_OVERLOAD_CC_FILE_1(className, maxFreeListSize) \
                                                                         \
className*       className::freeList= NULL;                              \
XilMutex         className::freeListMutex;                               \
unsigned int     className::freeListCount = 0;                           \
                                                                         \
void*                                                                    \
className::operator new(size_t)                                          \
{                                                                        \
    className* entry;                                                    \
                                                                         \
    freeListMutex.lock();                                                \
                                                                         \
    if(freeListCount > maxFreeListSize) {                                \
        entry = ::new className;                                         \
        entry->nextFree = (className*)-1;                                \
    } else {                                                             \
        entry = freeList;                                                \
                                                                         \
        if(entry == NULL) {                                              \
            entry = ::new className;                                     \
                                                                         \
            entry->nextFree = NULL;                                      \
                                                                         \
            freeListCount++;                                             \
        }                                                                \
                                                                         \
        freeList = entry->nextFree;                                      \
    }                                                                    \
                                                                         \
    freeListMutex.unlock();                                              \
                                                                         \
    return entry;                                                        \
}                                                                        \
                                                                         \
void                                                                     \
className::operator delete(void* ptr, size_t)                            \
{                                                                        \
    freeListMutex.lock();                                                \
                                                                         \
    if(((className*)ptr)->nextFree == (className*)-1) {                  \
        ::delete ptr;                                                    \
    } else {                                                             \
        ((className*)ptr)->nextFree   = freeList;                        \
        freeList                      = (className*)ptr;                 \
    }                                                                    \
                                                                         \
    freeListMutex.unlock();                                              \
}
#else
#define _XIL_NEW_DELETE_OVERLOAD_PUBLIC(className)

#define _XIL_NEW_DELETE_OVERLOAD_PRIVATE(className)

#define _XIL_NEW_DELETE_OVERLOAD_CC_FILE(className, maxFreeListSize, blockSize)
#define _XIL_NEW_DELETE_OVERLOAD_CC_FILE_1(className, maxFreeListSize)
#endif //_XIL_OVERLOAD_NEW_AND_DELETE

//
//  Global Function Definitions
//
XilSystemState* XiliOpen();
char*           XiliGetPath(const char* given_sub_string);

//
//  Function declaration for an image's data supply routine which is specified
//  by xil_set_data_supply_routine() binding.
//
//  NOTE:  A second declaration of XilDataSupplyFuncPtr lives in
//         xil.h because XilStorage is an object at the API and a pointer
//         inside of XIL. 
//
//  The image is included as an argument because the programmer may
//      use the same routine for multiple images
//  The storage object is to fill with the information
//  The x,y are a convenient way to get the upper left coord of the data
//  The xsize and ysize will most likely be txsize,tysize, but
//      as the image may have been re-imxported, the programmer will have
//      no way to access the values at the time of the callback
//  TODO: maynard/jlf 12/6/96 - There is a problem of recursive
//        locking if the programmer makes a call on the image while
//        in the callback routine.
//
typedef int  (*XilDataSupplyFuncPtr)(XilImage*      image,
                                     XilStorageAPI* storage,
                                     unsigned int   x,
                                     unsigned int   y,
                                     unsigned int   xsize,
                                     unsigned int   ysize,
                                     void*          user_args);

//
//  Using 0.9999 instead of 1.0 to simulate the open endedness of the
//  image area.  +1.0 is the area edge of the image, but we want our
//  calculations that round and clamp to be based on pixels having extent
//  right upto the edge of the image, but not including the right edge of the
//  image.
//
const double XILI_TOP_LF_EXTENT = 0.0;
const double XILI_BOT_RT_EXTENT = 1.0 - 1.0e-9;
