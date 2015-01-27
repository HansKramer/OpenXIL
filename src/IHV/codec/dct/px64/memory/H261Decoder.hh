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
//  File:       H261Decoder.hh
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:23:12, 03/10/00
//
//  Description:
//
//    H261Decoder class definition
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)H261Decoder.hh	1.3\t00/03/10  "

#ifndef H261DECODER_H
#define H261DECODER_H

#include "IdctDecoder.hh"

#define MTYPE_INTER			0x10
#define MTYPE_MC			0x08
#define MTYPE_FIL			0x04
#define MTYPE_CBP			0x02
#define MTYPE_MQUANT			0x01

#define ESC_CODE	0xffff
#define PSC		0x0010

#define MBA_STUFFING	34
#define MBA_START_CODE	35

class H261Decoder : public IdctDecoder {

private:

    int* mbaTable;
    int* mvdTable;
    int* mtyTable;
    int* cbpTable;
    int* dctTable;
    int* fstTable;

public:
    int isok;

    int* useMBATable()	{return mbaTable; }
    int* useMVDTable()	{return mvdTable; }
    int* useMTYTable()	{return mtyTable; }
    int* useCBPTable()	{return cbpTable; }
    int* useDCTTable()	{return dctTable; }
    int* useFSTTable()	{return fstTable; }

    int allocOk()       {return isok; }

    // Constructor
    H261Decoder();
    ~H261Decoder();
};

#endif /* H261DECODER_H */





