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
//  File:	xili_dyn_links.cc
//  Project:	XIL
//  Revision:	1.10
//  Last Mod:	10:16:35, 03/10/00
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
#pragma ident	"@(#)xili_dyn_links.cc	1.10\t00/03/10  "

//
//  System Includes
//

#if defined(_WINDOWS)
#elif defined(HPUX)
#include <dl.h>
#else
#include <dlfcn.h>
#endif
#include <errno.h>


//
// XIL includes

#include "XiliUtils.hh"

#define DL_UNKNOWNERR  "<UNKNOWN ERROR>"


//
// returns NULL on failure
//
XilDlHandle xili_dlopen(char *lib_name)
{
    XilDlHandle dlhandle;

#ifdef _WINDOWS
    dlhandle = LoadLibrary(lib_name);
#elif defined (HPUX)
    dlhandle = shl_load(lib_name, BIND_DEFERRED, 0);
#else
    dlhandle = dlopen(lib_name, RTLD_LAZY);
	if(dlhandle == NULL)
		xili_print_debug("Failing to open %s: %s\n", lib_name, dlerror());
#endif
    return dlhandle;
}


//
// returns NULL on failure
//
void* xili_dlsym(XilDlHandle dlhandle, const char *symbol_name)
{
    void *sym_ptr = NULL;

#ifdef _WINDOWS
    sym_ptr = GetProcAddress(dlhandle, symbol_name);
#elif defined (HPUX)
    shl_findsym(&dlhandle, symbol_name, TYPE_UNDEFINED, &sym_ptr);
#else
    sym_ptr = dlsym(dlhandle, symbol_name);
#endif

  return sym_ptr;
}


//
// returns NULL on failure
//
int xili_dlclose(XilDlHandle dlhandle)
{
    int ret_val;

#ifdef _WINDOWS
    ret_val = !FreeLibrary(dlhandle);
#elif defined (HPUX)
    ret_val = (shl_unload(dlhandle) < 0);
#else
    ret_val = dlclose(dlhandle);
#endif

    return ret_val;
}

const char* xili_dlerror()
{
#ifdef _WINDOWS
    return(xili_strerror(GetLastError()));
#elif defined (HPUX)
    if (!errno)
        errno = ENOSYM;
    return(xili_strerror(errno));
#else
    char *errstr = dlerror();
    return(errstr ? errstr : "UNKNOWN DL ERROR");
#endif
}
