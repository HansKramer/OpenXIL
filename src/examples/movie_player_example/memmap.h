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
*    File:	memmap.h
*    Project:	XIL Movie Player Example
*    Revision:	1.2
*    Last Mod:	10:38:33, 03/10/00
*  
*
*   Description:
*
*        This file is used to support the mmap utilities in memmap.c
*
*------------------------------------------------------------------------
*
*    COPYRIGHT
*
*----------------------------------------------------------------------*/

typedef struct {
    Xil_unsigned8 *dptr;    /* current pointer into the bitstream    */
    Xil_unsigned8 *start;    /* pointer to start of the bitstream     */
    int fd;    /* file descriptor of bitstream file     */
    int len;    /* length (bytes) of bitstream memory    */
    int currentFrame;    /* current frame number                  */
    Xil_unsigned8 *mstart;    /* pointer to start of first image       */
    int   mlen;    /* length of bitstrean from first image  */
} MFILE;

void openfile( MFILE*, char*);
void attach_file( MFILE*, char *);
void init_memfile( MFILE* );


