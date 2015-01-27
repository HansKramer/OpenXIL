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
//  File:       XilDeviceCompressionFaxG4.hh
//  Project:    XIL
//  Revision:   1.8
//  Last Mod:   10:22:45, 03/10/00
//
//  Description:
//
//	Header file for CCITT group 4 fax codec.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDeviceCompressionFaxG4.hh	1.8\t00/03/10  "

#ifndef XILDEVICECOMPRESSIONFAXG4_H
#define XILDEVICECOMPRESSIONFAXG4_H

#include <xil/xilGPI.hh>
#include "../faxcommon/XilDeviceCompressionFax.hh"
#include "CompressInfo.hh"
#include "DecompressInfo.hh"

class XilDeviceCompressionFaxG4 : public XilDeviceCompressionFax {
public:

    //
    // Constructor/deconstructor
    //
    XilDeviceCompressionFaxG4 (XilDeviceManagerCompression* xdct, 
                               XilCis*                      cis);

    ~XilDeviceCompressionFaxG4 ();

    //
    //  REQUIRED PURE VIRTUAL IMPLEMENTATIONS
    //      from XilDeviceCompression
    //
    XilStatus    compress(XilOp*       op,
                          unsigned int op_count,
                          XilRoi*      roi,
                          XilBoxList*  bl);

    XilStatus    decompress(XilOp*       op,
                            unsigned int op_count,
                            XilRoi*      roi,
                            XilBoxList*  bl);

    XilStatus    decompressTranspose(XilOp*       op,
                                     unsigned int op_count,
                                     XilRoi*      roi,
                                     XilBoxList*  bl);

    //
    //  VIRTUAL FUNCTION OVERRIDES
    //      Functions which override the default behavior
    //
    int          findNextFrameBoundary();

private:
    int          add_eofb(Xil_unsigned8* buf,
                          int            where);

    unsigned int compressBand_2d(unsigned int   band,
                                 CompressInfo*  ci,
                                 Xil_unsigned8* cis_addr);

    XilStatus    decompressFrame(DecompressInfo* di);

    int          decompressBand_2d(unsigned int    band,
                                   DecompressInfo* di,
                                   Xil_unsigned8*  cis_addr,
                                   Xil_unsigned8*  cis_end);

    int          decTransposeFrame(DecompressInfo* di,
                                   XilFlipType     fliptype);

    // Ptr to a line of zeroes used as an initial reference for 2-D coding
    Xil_unsigned8* initial_ref;

    unsigned int* uc_table[2];

    //
    // Static tables - one set for all instances
    //
    static unsigned int uc_table_b[8192];
    static unsigned int uc_table_w[8192];
    static unsigned int table_2d[256];

    static unsigned char leading_zeroes[256];
    static unsigned char trailing_zeroes[256];
};

#endif


