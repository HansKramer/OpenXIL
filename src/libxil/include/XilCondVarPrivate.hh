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
//  File:	XilCondVarPrivate.hh
//  Project:	XIL
//  Revision:	1.7
//  Last Mod:	10:21:47, 03/10/00
//
//  Description:
//	
//	
//	
//	
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilCondVarPrivate.hh	1.7\t00/03/10  "

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
    pthread_cond_t    condId;
#elif defined(_XIL_USE_SOLTHREADS)
    cond_t            condId;
#elif defined(_XIL_USE_WINTHREADS)
    HANDLE            condId;
#endif
    
    XilMutex*         localMutex;

    //
    //  The size of this is 64 bytes.  If you add data members to this 
    //  class you must add to the count to be subtracted from 64.
    // 
#if defined(_XIL_USE_PTHREADS)
    unsigned char      extraData[64 - sizeof(pthread_cond_t) - sizeof(XilMutex*)];
#elif defined(_XIL_USE_SOLTHREADS)
    unsigned char      extraData[64 - sizeof(cond_t) - sizeof(XilMutex*)];
#elif defined(_XIL_USE_WINTHREADS)
    unsigned char      extraData[64 - sizeof(HANDLE) - sizeof(XilMutex*)];
#else
    unsigned long      extraData[64 / sizeof(unsigned long)];
#endif

#endif // _XIL_PRIVATE_DATA
