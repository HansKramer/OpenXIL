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

/*-----------------------------------------------------------------------
* 
*    File:	memmap.c
*    Project:	XIL Movie Player Example
*    Revision:	1.4
*    Last Mod:	10:24:15, 03/10/00
*  
*    Description:
*        This is a utility file to handle opening and mmaping
*        data from an external file.
*        
*------------------------------------------------------------------------
*
*    COPYRIGHT
*
*----------------------------------------------------------------------*/
#ifndef lint
static	char     sccsid[] = "@(#)memmap.c	1.4\t00/03/10  ";
#endif

#include <stdio.h>
#include <unistd.h>
#include <xil/xil.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "memmap.h"


void
openfile(MFILE* memfile, char* fname)
{
    int fd;
    Xil_unsigned8 *fp;
    struct stat buf;
    
    if((fd = open(fname, O_RDONLY)) < 0) {
        fprintf(stderr, "ERROR: failed to open %s file.\n", fname );
        exit(-1);
    }
    fstat(fd, &buf);
    fp = (Xil_unsigned8 *) mmap(0, (size_t)buf.st_size, PROT_READ,
                                MAP_SHARED, fd, (off_t)0);
    if((int)fp == -1) {
        fprintf(stderr, "ERROR: mmap failed\n");
        exit(-1);
    }
    memfile->dptr = fp;
    memfile->start = fp;
    memfile->fd = fd;
    memfile->len = (int) buf.st_size;
}

void
init_memfile( MFILE* memfile )
{
    memfile->dptr = NULL;
    memfile->start = NULL;
    memfile->fd = 0;
    memfile->len = 0;
    memfile->currentFrame = 0;
    memfile->mstart = NULL;
    memfile->mlen = 0;
}

void
detach_file( MFILE* memfile )
{
    close(memfile->fd);
    if (munmap((caddr_t) memfile->start, memfile->len) == -1) {
        fprintf(stderr, "ERROR: munmap failed in detach_file\n");
    }
    memfile->start = 0;
    memfile->len = 0;
    memfile->fd = 0;
}

void
attach_file(MFILE *memfile, char *fname)
{
    if(memfile->start)
        detach_file( memfile);
    
    openfile(memfile,fname);
    
    memfile->mstart = memfile->dptr;
    memfile->mlen = memfile->len;
}








