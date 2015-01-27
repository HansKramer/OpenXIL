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
//  File:       Quantizer.hh
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:23:48, 03/10/00
//
//  Description:
//
//    Declaration of Quantizer Abstract Class
//
//    A Quantizer is an abstract class designed for derivation which
//    should define both the Quantize and an Output routines.
//
//    A Quantizer object manages some number of QTables (quantization
//    tables). It also maintains a precision variable that defines the
//    precision for all of its qtables.
//
//    o A set of operations exist to Add Tables to the Quantizer
//
//    o The Quantizer can be instructed to Scale its tables by a
//        given value.
//
//               ----- Derived Quantizers should -----
//
//    o The Quantizer can be instructed to Quantize a Block of data
//        given using one of its tables. 
//
//    o The Quantizer can be instructed to Output the Tables it manages
//
//               ------  Derived Quantizers can ------
//
//    o Redefine the Scale routine
//
//               -------------------------------------
//
//   The quantizer has a buffer object (XilCisBuffer) through
//   which output of quantization tables are made to.
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Quantizer.hh	1.3\t00/03/10  "



#ifndef QUANTIZER_H
#define QUANTIZER_H

#include <xil/xilGPI.hh>
#include "SingleBuffer.hh"
#include "QTable.hh"

#define ALL_QTABLES      -1


class Quantizer {
public:
    void    Init(unsigned int nt, unsigned int p, Xil_unsigned8 m, SingleBuffer* buf);  
    void    Delete();

    void    Add_Table(QTable* t, int as_table);
    void    Add_Table(QTable& t, int as_table);  
    void    Add_Table(int x[QTABLE_WIDTH][QTABLE_HEIGHT], int as_table);

    void    set_Buffer(SingleBuffer* buf) { buffer = buf; }
    void    set_Marker(Xil_unsigned8 m)          { marker = m; }

    QTable* Table(int table) const;
    QTable* operator[](int table) const { return Table(table); }


    Xil_unsigned8  Num_Tables() const { return num_tables; }
    Xil_unsigned8  Precision()  const { return table_precision; }

    void  resetTableUsage();
    void  usingTable(int table);
    int   numTablesInUse();
    int     tableLoaded(int table_id);

    virtual void    Scale(int value, int table = ALL_QTABLES);
    virtual void    Quantize(int* b, int table) = 0;
    virtual void    Output(int table = ALL_QTABLES) = 0;

protected:
    QTable**      qtables;
    int*          tables_in_use;
    int*          tables_loaded;
    int*          table_outputted;

    Xil_unsigned8 num_tables;
    Xil_unsigned8 table_precision;
    Xil_unsigned8 marker;

    SingleBuffer* buffer;


    int      tableInUse(int table);
    int      tableOutputted(int table);

    int      ValidTable(int table) const
        { return (table>=0 && table<num_tables) ? 1 : 0; }

};

#endif

