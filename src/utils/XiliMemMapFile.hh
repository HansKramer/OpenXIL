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
//  File:	XiliMemMapFile.hh
//  Project:	XIL
//  Revision:	1.10
//  Last Mod:	10:24:01, 03/10/00
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
#pragma ident	"@(#)XiliMemMapFile.hh	1.10\t00/03/10  "

#ifndef _XILI_MMAP_FILE_HH
#define _XILI_MMAP_FILE_HH

#ifdef _WINDOWS
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/types.h>
#endif
#include <stdio.h>

#include <XiliUtils.hh>

//
// file handle typedef's
//
#ifdef _WINDOWS
#define OFFSET_T          long
#else
#define OFFSET_T          off_t
#endif // _WINDOWS

#ifdef _WINDOWS
#define M_PROT_RD         PAGE_READONLY
#define M_PROT_RDWR       PAGE_READWRITE
#define M_PROT_EXEC       SEC_IMAGE
#define M_PROT_WRITECOPY  PAGE_WRITECOPY
#define M_PROT_NONE       0

#define M_MAP_SHARED      PAGE_READWRITE
#define M_MAP_PRIVATE     PAGE_WRITECOPY
#else
#define M_PROT_RD         PROT_READ
#define M_PROT_RDWR       PROT_READ|PROT_WRITE
#define M_PROT_EXEC       PROT_EXEC
#define M_PROT_WRITECOPY  M_PROT_RDWR
#define M_PROT_NONE       PROT_NONE

#define M_MAP_SHARED      MAP_SHARED
#define M_MAP_PRIVATE     MAP_PRIVATE
#endif // _WINDOWS

class XiliMemMapFile {
public:
    //
    // public data
    //

    //
    // f_flags   - file opening flags
    // mem_proto - memory map proto
    // mem_flags - memory map flags
    //
    XiliMemMapFile(char *fname,
                   int fflags,
                   int mproto,
                   int mflags);
    ~XiliMemMapFile();

    //
    // public methods
    //
    XilStatus memMap(size_t map_len = 0xFFFFFFFF);
    XilStatus memUnmap();
    char      *getMemMap();
    XilStatus fileSeek(OFFSET_T offset,
                       int whence = SEEK_SET);
    XilStatus fileRead(void *buffer,
                       size_t nbytes,
                       size_t *nbytes_return = NULL);
    XilStatus fileWrite(void *buffer,
                        size_t nbytes,
                        size_t *nbytes_return = NULL);

private:
    //
    // private methods
    //
    void      resetMap();

    //
    // private data members
    //

    int     m_fd;                        // file handle
    void    *m_hfilemap;                 // handle of CreateFileMapping();
    char    *m_hmemmap;                  // handle of MapViewOfFile() or mmap();
    char    m_fname[XILI_PATH_MAX];           // file name
    int     m_proto;                     // mem map protocols
    int     m_flags;                     // mem map flags
    int     m_fflags;                    // file creation flags
    size_t  m_maplen;                    // mem map length
};

#endif /* _XILI_MMAP_FILE_HH */
