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

//This line lets emacs recognize this as -*- C++ -*- Code
//------------------------------------------------------------------------
//
//  File:       XilDeviceCompressionFaxG3.hh
//  Project:    XIL
//  Revision:   1.8
//  Last Mod:   10:22:43, 03/10/00
//
//  Description:
//
//    A CCITT group 3 fax device compressor.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilDeviceCompressionFaxG3.hh	1.8\t00/03/10  "


#ifndef XILDEVICECOMPRESSIONFAXG3_H
#define XILDEVICECOMPRESSIONFAXG3_H

#include <xil/xilGPI.hh>
#include "../faxcommon/XilDeviceCompressionFax.hh"
#include "CompressInfo.hh"
#include "DecompressInfo.hh"


class XilDeviceCompressionFaxG3 : public XilDeviceCompressionFax 
{
public:
    //
    // Constructor/deconstructor
    //
    XilDeviceCompressionFaxG3(XilDeviceManagerCompression* xdct, 
                              XilCis*                      cis);

    ~XilDeviceCompressionFaxG3();

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

    //
    //  VIRTUAL FUNCTION OVERRIDES
    //      Functions which override the default behavior
    //
    int          findNextFrameBoundary(void);

private:
    int          add_eol(Xil_unsigned8* buf, 
                         int            where);

    int          find_eol(unsigned int* base, 
                          unsigned int* end, 
                          int           index);

    int          grab_12(Xil_unsigned8* base, 
                         unsigned int   bit_offset);

    unsigned int compressBand(unsigned int band,
                              CompressInfo*  ci,
                              Xil_unsigned8* cis_addr);

    XilStatus    decompressFrame(DecompressInfo* di);

    int          decompressBand(unsigned int    band,
                                DecompressInfo* di,
                                Xil_unsigned8*  cis_addr,
                                Xil_unsigned8*  cis_end);

    //
    // Static tables - one copy for all instances
    //
    static unsigned int cmpr_table[4096];
    static unsigned int ss_table_b[8192];
    static unsigned int ss_table_w[8192];

    unsigned int* ss_table[2];

    static unsigned char leading_zeroes[256];
    static unsigned char trailing_zeroes[256];
};

#endif // XILDEVICECOMPRESSIONFAXG3_H
