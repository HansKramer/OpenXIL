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
//  File:       Jpeg_Quantizer.hh
//  Project:    XIL
//  Revision:   1.5
//  Last Mod:   10:22:54, 03/10/00
//
//  Description:
//
//    Definition of Jpeg_Quantizer Class

//    The Jpeg_Quantizer object, derived from the Quantizer abstract
//    class, provides quantization and output routines specific to
//    JPEG:
//
//    o The Jpeg_Quantizer can be instructed to Quantize a Block of data
//        given using one of its tables. The 
//
//    o The Jpeg_Quantizer can be instructed to Output the Tables it manages
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Jpeg_Quantizer.hh	1.5\t00/03/10  "


#ifndef JPEG_QUANTIZER_H
#define JPEG_QUANTIZER_H

#include "SingleBuffer.hh"
#include "Quantizer.hh"

class Jpeg_Quantizer : public Quantizer {
public:
  
  Jpeg_Quantizer(unsigned int  nt, 
                 unsigned int  p   = BIT_PRECISION_8,
                 Xil_unsigned8 m   = 0, 
                 SingleBuffer* buf = NULL);
  ~Jpeg_Quantizer();
  
  void    Quantize(int* b, int table);
  void    Output(int table = ALL_QTABLES);
  void    OutputChanges(int table = ALL_QTABLES);  

private:
  //
  // Use anonymous enum to define symbolic constant
  //
  enum {
      MAX_NUM_QTABLES = 4
  };

};

#endif

