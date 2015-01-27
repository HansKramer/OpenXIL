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
//  File:       Jpeg_Quantizer.cc
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:14:28, 03/10/00
//
//  Description:
//
//    Constructor for Jpeg_Quantizer object. The number of tables and the
//    precision of the tables are validated. A call is then made to the
//    Init routine of the base abstract class which will construct
//    all tables.
//
//  Parameters:
//        
//    unsigned int nt:   number of tables
//
//    unsigned int p:    precision of tables
//
//    SingleBuffer*:       pointer to SingleBuffer Object
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Jpeg_Quantizer.cc	1.6\t00/03/10  "

#include <xil/xilGPI.hh>
#include "Jpeg_Quantizer.hh"
#include "JpegMacros.hh"
#include "ZigZag.hh"


Jpeg_Quantizer::Jpeg_Quantizer(unsigned int  nt, 
                                unsigned int  p, 
                                Xil_unsigned8 mark, 
                                SingleBuffer* buf)
{
    // Set number of Quantization tables

    if(nt > MAX_NUM_QTABLES) {

        // Jpeg bitstream error: number of quantizer tables too large 
        XIL_ERROR( NULL, XIL_ERROR_USER, "di-80", TRUE);

        num_tables = 0;
    } else {
        num_tables = nt;
    }

    // Set precision of tables


    if((p != BIT_PRECISION_8) && (p != BIT_PRECISION_16)) {

        // Jpeg bitstream error: invalid precision identifier 
        XIL_ERROR( NULL, XIL_ERROR_USER, "di-79", TRUE); 

        table_precision = UNKNOWN_PRECISION;
    } else {
        table_precision = p;
    }

    Init(num_tables, table_precision, mark, buf);

}

//------------------------------------------------------------------------
//
//  Function:        Jpeg_Quantizer::~Jpeg_Quantizer
//  Created:        92/03/24
//
//  Description:
//        
//    Destructor for Jpeg_Quantizer object. Calls Delete of base
//    abstract class which will delete all tables.
//
//  Deficiencies/ToDo:
//
//    Use new methof for delete
//
//------------------------------------------------------------------------

Jpeg_Quantizer::~Jpeg_Quantizer()
{
    Delete();
}


//------------------------------------------------------------------------
//
//  Function:        Jpeg_Quantizer::Quantize
//  Created:        92/03/24
//
//  Description:
//        
//   Quantizes a Block of data using table given by table id
//        
//  Parameters:
//        
//   int* b:     pointer to a Block of data
//   int table:  quantization table id
//        
//  Side Effects:
//        
//   Supplied Block is changed
//
//  Notes:
//        
//   Does this routine lean toward a jpeg implementation??
//
//------------------------------------------------------------------------

void  Jpeg_Quantizer::Quantize( int* b, int table)
{
    if(tableInUse(table)) {

        //transform and perform JPEG compliant 
        //  quantization of an 8x8 input block        

        int *qptr = &qtables[table]->table[0][0];
        int *bptr = b;

        //
        // Quantize block  (Note: coefficents are 4x JPEG
        // specification, the rescaling is done here)
        // Do a pre-test of the coefficients to see if they
        // would quantize out to zero (numerator < denominator).
        // This saves the cost of the integer divide
        //
        for(int count=64; count!=0; count--) {
            int fuv = *bptr;
            int quv = *qptr;
            if(fuv < 0) {
                if(-fuv <= quv) {
                    *bptr = 0;
                } else {
                    *bptr = ((fuv / quv) - 2) / 4;
                }
            } else {
                if(fuv <= quv) {
                    *bptr = 0;
                } else {
                    *bptr = ((fuv / quv) + 2) / 4;
                }
            }
            bptr++;
            qptr++;
        }
    } else {
        // Jpeg bitstream error: Table not used by any bands 
        XIL_ERROR( NULL, XIL_ERROR_USER, "di-78", TRUE); 
    }
}

//------------------------------------------------------------------------
//
//  Function:        Jpeg_Quantizer::Output
//  Created:        92/03/24
//
//  Description:
//        
//    Calls for outputing quantization table(s). If supplied table
//    id equals AL_QTABLES, then all quantization tables are
//    outputes. Otherwise, if the id is valid, only the specified
//    quantization table is outputed.
//        
//  Parameters:
//        
//    int table:  quantization table id
//        
//------------------------------------------------------------------------

void  Jpeg_Quantizer::Output(int table)
{
    int qtlen = 2;                           // 2 bytes for marker
    int qtsize = QTABLE_HEIGHT*QTABLE_WIDTH; // size of qtable
    int pr_id;                               // precision & table id
    ZigZagArray zigzag;
    Xil_unsigned8* zptr = zigzag.getArray(); // ptr to zigzag pattern
    int   i,t,qval;

    if(buffer == NULL) {

        // buffer does not exist, Internal error
        XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-95", TRUE);

    } else if(marker == 0) {

        // Jpeg bitstream error: invalid qtable marker 0
        XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-77", TRUE);

    } else {
        if(table != ALL_QTABLES) {
            if(tableInUse(table)) {

                // Define quantization tables
                buffer->addByte(MARKER);
                buffer->addByte(marker);        

                // length += qtable_size 1 byte for table precision and id
                qtlen += qtsize + 1;

                //  quantization table length
                buffer->addShort(qtlen);

                // determine precision & table id byte value
                pr_id = ((qtables[table]->Precision()&0xf)<<4) | (table&0xf);
                buffer->addByte(pr_id);


                // output table using zigzag indexing pattern
                for(i = 0; i < qtsize; i++) {
                    qval = (*qtables[table])[*zptr++];
                    buffer->addByte(qval);
                }
                table_outputted[table] = 1;
            }
            else {
                if(ValidTable(table)) {

                    // Jpeg bitstream error: invalid table identifier
                    XIL_ERROR( NULL, XIL_ERROR_USER, "di-82", TRUE);

                } else {

                    // Jpeg bitstream error: Table not used by any bands
                    XIL_ERROR( NULL, XIL_ERROR_USER, "di-78", TRUE);

                }
            }
        } else {
            // Define quantization tables
            buffer->addByte(MARKER);
            buffer->addByte(marker);

            // deteermine the number of tables being used
            int num_tables_in_use = numTablesInUse();

            // length += qtable_size*num_tables_in_use + 
            // num_table_in_use bytes for table precisions
            // and ids

            qtlen += qtsize*num_tables_in_use + num_tables_in_use;

            //  quantization table length
            buffer->addShort(qtlen);          

            for(t = 0; t < num_tables; t++, zptr = zigzag.getArray()) {
                if(tableInUse(t)) {
                    // determine precision & table id byte value         
                    pr_id = ((table_precision&0xf)<<4) | (t&0xf);
                    buffer->addByte(pr_id);

                    // output table using zigzag indexing pattern
                    for(i = 0; i < qtsize; i++) {
                        qval = (*qtables[t])[*zptr++];
                        buffer->addByte(qval);         
                    }

                    table_outputted[t] = 1;
                }
            }
        }
    }
}

//------------------------------------------------------------------------
//
//  Function:        Jpeg_Quantizer::OutputChanges(int table)
//  Created:        92/03/24
//
//  Description:
//        
//    Calls for outputing quantization table(s). If supplied table
//    id equals AL_QTABLES, then all quantization tables are
//    outputes. Otherwise, if the id is valid, only the specified
//    quantization table is outputed. In any case, only tables that
//    have not yet been outputted are actually sent to the byte
//    stream.
//        
//  Parameters:
//        
//    int table:  quantization table id
//        
//------------------------------------------------------------------------

void  Jpeg_Quantizer::OutputChanges(int table)
{
    int   t;

    if(table != ALL_QTABLES) {
        if(tableInUse(table) && !table_outputted[table]) {
            Output(table);
        }
    } else {
        for(t = 0; t < num_tables; t++) {
            if(tableInUse(t) && !table_outputted[t]) {
                Output(t);
            }
        }
    }

}


