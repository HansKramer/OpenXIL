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
//  File:	XiliMemMapFile.cc
//  Project:	XIL
//  Revision:	1.14
//  Last Mod:	10:16:34, 03/10/00
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
#pragma ident	"@(#)XiliMemMapFile.cc	1.14\t00/03/10  "

//
//  System Includes
//
#ifdef _WINDOWS
#include <windows.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>

#include "XiliUtils.hh"
#include "XiliMemMapFile.hh"


//
// f_flags   - file opening flags
// mem_proto - memory map proto
// mem_flags - memory map flags
//
XiliMemMapFile::XiliMemMapFile(char *fname,
                               int fflags,
                               int mproto,
                               int mflags)
{
    m_maplen = 0xFFFFFFFF;

    m_fd     = 0xFFFFFFFF;
    m_fflags = fflags;

    m_proto  = mproto;
    m_flags  = mflags;

    m_hfilemap = NULL;
    m_hmemmap  = NULL;

    memset(m_fname, '\0', XILI_PATH_MAX);
    if(fname)
        strcpy(m_fname, fname);
#ifdef _WINDOWS
    else {
        //
        // for creating a file mapping object backed by OS page file
        //
        m_fd = 0xFFFFFFFF;
    }
#endif
}

XiliMemMapFile::~XiliMemMapFile()
{
    this->resetMap();
}

XilStatus XiliMemMapFile::memMap(size_t map_len)
{


    if(map_len != 0xFFFFFFFF)
        m_maplen = map_len;

    //
    // open file operation 
    // NOTE : NT has an option of using OS page file for mapping. 
    //        if the m_fname is null and m_fd is 0xFFFFFFFF
    //        create file mapping backed by system OS page file
    //

#ifdef _WINDOWS

    // security attribs been set to NULL which will cause the handle
    // not to be inherited

    //
    // open only if file name is specified
    //
    if(m_fname[0])
    {
        int iCreat  = 0;
        int iAccess = 0;
    
        if((m_fflags & O_RDONLY) == O_RDONLY)
            iAccess |= GENERIC_READ;
    
        if(m_fflags & O_WRONLY)
            iAccess |= GENERIC_WRITE;
        
        if(m_fflags & O_RDWR)
            iAccess |= (GENERIC_READ | GENERIC_WRITE);
    
        if((m_fflags & O_CREAT) && !(m_fflags & O_EXCL))
            iCreat |= OPEN_ALWAYS;
        else
            iCreat |= OPEN_EXISTING;
    
        m_fd = (int) CreateFile(m_fname,              // fname
                                iAccess,              // desired access
                                0,                    // share mode
                                NULL,                 // security attributes
                                iCreat,               // creation flag
                                0,                    // flags & attribs
                                NULL);                // template file
    
        if(m_fd == (int) INVALID_HANDLE_VALUE) {
            this->resetMap();
            return XIL_FAILURE;
        }
    }

    //
    // if file size is not specified and we want to use OS page file
    // flag error
    //
    if((m_fd == 0xFFFFFFFF) && (m_maplen == 0xFFFFFFFF)) {
        this->resetMap();
        return XIL_FAILURE;
    }

    //
    // Find the size for map length
    //

    if((m_maplen != 0xFFFFFFFF) || ((m_fd != 0xFFFFFFFF) &&
            ((m_maplen = GetFileSize((HANDLE)m_fd, NULL)) == 0xFFFFFFFF))) {
        this->resetMap();
        return XIL_FAILURE;
    }

    //
    // file mapping flags and protections
    //
    if(m_flags & M_MAP_PRIVATE)
        m_proto  = M_PROT_WRITECOPY;

        
    m_hfilemap = CreateFileMapping((HANDLE)m_fd,  // file handle
                                   NULL,          // fmap attribs.
                                   m_proto,       // protection
                                   0,             // max size high
                                   m_maplen,      // max size low
                                   NULL);         // mapping object name
    if(m_hfilemap == NULL) {
        this->resetMap();
        return XIL_FAILURE;
    }

    //
    // map view flags and protections
    //
    int  mvf_flags = 0;                    // map view file flag

    if(m_flags & M_MAP_PRIVATE)
        mvf_flags = FILE_MAP_COPY;

    if(m_proto & M_PROT_RD)
        mvf_flags |= FILE_MAP_READ;

    if(m_proto & M_PROT_RDWR)
        mvf_flags |= FILE_MAP_ALL_ACCESS;

    // entire file is mapped by specifying last arg as 0
    m_hmemmap = (char *) MapViewOfFile(m_hfilemap,       // fmapping object
                                       mvf_flags,        // desired access
                                       0,                // foffset high
                                       0,                // foffset low
                                       m_maplen);        // bytes to map
    if(m_hmemmap == NULL) {
        this->resetMap();
        return XIL_FAILURE;
    }
#else
    m_fd = open(m_fname, m_fflags);

    if(m_fd < 0) {
        this->resetMap();
        return XIL_FAILURE;
    }

    // Find the size for map length

    if(m_maplen == 0xFFFFFFFF) {
        struct stat statbuf;
        if(fstat(m_fd, &statbuf) == -1) {
            this->resetMap();
            return XIL_FAILURE;
        }
        m_maplen = statbuf.st_size;
    }

    m_hmemmap = (char*)mmap(NULL, m_maplen, m_proto, m_flags, m_fd, 0);
    if(m_hmemmap == (caddr_t)-1) {
        this->resetMap();
        return XIL_FAILURE;
    }
#endif // _WINDOWS
    return XIL_SUCCESS;
}


XilStatus XiliMemMapFile::memUnmap()
{
    int ret;

#ifdef _WINDOWS
    ret = UnmapViewOfFile(m_hmemmap);
    ret = CloseHandle(m_hfilemap);
    m_hfilemap = NULL;
#else
    ret = !munmap((char*)m_hmemmap, m_maplen);
#endif
    m_hmemmap = NULL;
    m_maplen  = 0xFFFFFFFF;

    return(ret ? XIL_SUCCESS : XIL_FAILURE);
}

// private method
void XiliMemMapFile::resetMap()
{
    if(m_hmemmap)
        this->memUnmap();
    m_hmemmap  = NULL;
    
#ifdef _WINDOWS
    if(m_hfilemap)
        CloseHandle(m_hfilemap);
    m_hfilemap = NULL;
#endif

    if(m_fd != 0xFFFFFFFF)
#ifdef _WINDOWS
        CloseHandle((HANDLE)m_fd);
#else
        close(m_fd);
#endif
    m_fd     = 0xFFFFFFFF;

    m_maplen = 0xFFFFFFFF;

    m_proto  = m_flags  = m_fflags = 0;

    memset(m_fname, '\0', XILI_PATH_MAX);
}

char* XiliMemMapFile::getMemMap()  
{
    return ((char *)m_hmemmap);
}

XilStatus XiliMemMapFile::fileSeek(OFFSET_T offset,
                                   int whence)
{
#ifdef _WINDOWS
    unsigned short move_method;

    switch(whence) {
        case SEEK_SET:
            move_method = FILE_BEGIN;
            break;
        case SEEK_CUR:
            move_method = FILE_CURRENT;
            break;
        case SEEK_END:
            move_method = FILE_END;
            break;
        default:
            //
            // TODO bpb 11/20/1997 Error message needed ???
            //
            return XIL_FAILURE;
    }

    if(SetFilePointer((HANDLE)m_fd, offset, NULL, move_method) == 0xFFFFFFFF) {
        //
        // TODO bpb 11/20/1997 Error message needed ???
        //
        return XIL_FAILURE;
    }
#else
    if(lseek(m_fd, offset, whence) == (off_t)-1) {
        //
        // TODO bpb 11/20/1997 Error message needed ???
        //
        return XIL_FAILURE;
    }
#endif // _WINDOWS

    return XIL_SUCCESS;
}

XilStatus XiliMemMapFile::fileRead(void *buffer,
                                   size_t nbytes,
                                   size_t *nbytes_return)
{
#ifdef _WINDOWS
    unsigned long nbytes_read;

    if(ReadFile((HANDLE)m_fd, buffer, nbytes, &nbytes_read, NULL) != 0) {
        //
        // TODO bpb 11/20/1997 Error message needed ???
        //
        return XIL_FAILURE;
    }
#else
    size_t nbytes_read;

    if((nbytes_read = read(m_fd, buffer, nbytes)) < 0) {
        //
        // TODO bpb 11/20/1997 Error message needed ???
        //
        return XIL_FAILURE;
    }
#endif // _WINDOWS

    //
    // If requested, set the number of bytes actually read.
    //
    if(nbytes_return != NULL) {
        *nbytes_return = nbytes_read;
    }

    return XIL_SUCCESS;
}

XilStatus XiliMemMapFile::fileWrite(void *buffer,
                                    size_t nbytes,
                                    size_t *nbytes_return)
{
#ifdef _WINDOWS
    unsigned long nbytes_written;

    if(WriteFile((HANDLE)m_fd, buffer, nbytes, &nbytes_written, NULL) != 0) {
        //
        // TODO bpb 11/20/1997 Error message needed ???
        //
        return XIL_FAILURE;
    }
#else
    size_t nbytes_written;

    if((nbytes_written = write(m_fd, buffer, nbytes)) < 0) {
        //
        // TODO bpb 11/20/1997 Error message needed ???
        //
        return XIL_FAILURE;
    }
#endif // _WINDOWS

    //
    // If requested, set the number of bytes actually read.
    //
    if(nbytes_return != NULL) {
        *nbytes_return = nbytes_written;
    }

    return XIL_SUCCESS;
}
