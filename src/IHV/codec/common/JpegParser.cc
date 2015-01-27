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
//  File:       JpegParser.cc
//  Project:    XIL
//  Revision:   1.6
//  Last Mod:   10:16:15, 03/10/00
//
//  Description:
//
//    This file contains the JpegParser class routines.
//    This class is a parent for JPEGLLdecoder and for
//    JpegDecompressorData class.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)JpegParser.cc	1.6\t00/03/10  "


#include <string.h>
#include "JpegMacros.hh"
#include "JpegParser.hh"

#define DECODE_TBL_LEN        256
#define MAX_HUFFMAN_TABLE_ID        3
#define MAX_BANDS_PER_SCAN        4
#define MAX_SCAN_HV                10

#define SET_QTABLE_BIT(bitvector,table)  bitvector |= (1<<(table))
#define SET_DCTABLE_BIT(bitvector,table) bitvector |= ((1<<4)<<(table))
#define SET_ACTABLE_BIT(bitvector,table) bitvector |= ((1<<8)<<(table))

JpegParser::JpegParser(jpeg_mode mode)
{
    isOKFlag = FALSE ;

    decode_mode              = mode ;

    header.maxh              = 0;
    header.maxv              = 0;
    header.restartInterval   = 0 ;
    header.width             = 0;
    header.height            = 0;
    header.scan_selection    = 0;
    header.scan_pt_transform = 0;
    header.band_data = (banddata *) NULL ;

    memset((char *)quantizer, 1, sizeof(quantizer));

    for(int i=0; i < MAX_DECODER_TABLES; i++) {
        header.decoder[i] = (int *)NULL ;
    }

    definedInFrame = 0;
    reDefinedInCis = 0;
    notSeekable    = FALSE;

    cbm            = NULL ; 

    isOKFlag = TRUE ;
}

void 
JpegParser::reset()
{
    definedInFrame = 0;
    reDefinedInCis = 0;
    notSeekable    = FALSE;
}

Xil_boolean
JpegParser::isOK()
{
    if(this == NULL) {
        return FALSE;
    } else {
        if(isOKFlag == TRUE) {
            return TRUE;
        } else {
            delete this;
            return FALSE;
        }
    }
}

void 
JpegParser::free_decode_table(int *decode_table)
{
    for(int i=0; i<DECODE_TBL_LEN; i++) {
        if((decode_table[i] & 3) == 0) {
            delete (int*)decode_table[i]; 
        }
    }
    delete decode_table;
}

//
// Create a table for fast Huffman decoding.
//     .... table entries are formed as shown:
//     
//     type 0: unresolved symbol
//     
//     +--------+--------+--------+--------+
//     | address of second level table - 00|
//     +--------+--------+--------+--------+
//     
//     or type 1: resolved symbol, unresolved value
//     
//               symbol word
//               |        unused bits of index
//               |        |        # of unused index bits
//               |        |        |
//               V        V        V
//     +--------+--------+--------+--------+
//     |00000000|SSSSSSSS|XXXXXXXX|BBBBBB01|
//     +--------+--------+--------+--------+
//     
//     or type 3: resolved symbol, resolved value
//     
//     symbol value
//     |        symbol word
//     |        |        unused bits of index
//     |        |        |        # of unused index bits
//     |        |        |        |
//     V        V        V        V
//     +--------+--------+--------+--------+
//     |VVVVVVVV|SSSSSSSS|XXXXXXXX|BBBBBB11|
//     +--------+--------+--------+--------+
//

//
// Value for table entries which will never be accessed by a legal bitstream.
// Set value to type 3, 8 ununsed bits, symbol and value 0
// (this will avoid a secondary table fetch, will use the illegal (but known)
// value in all future decodes (until a Restart Inverval), and will set
// DC value to 0 or terminate the AC coefficient list
//
#define JPEG_ERROR_ENTRY  ((8<<2) | 3)

int* 
JpegParser::create_decode_table(Huffman_Code* huffman, int size)
{
    int j;
    int code, bits;
    int vbits;                // number of bits in the value field
    int val, left;
    int xbits, xcount;
    int *cptr;
    
    int* hptr = new int[DECODE_TBL_LEN];
    if(hptr == NULL) {
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);        
        return 0;
    }
    memset(hptr, 0, sizeof(int)*DECODE_TBL_LEN);
    
    //
    // 'i' is implicitly the symbol (because that's the way the table was made)
    //
    for(int i=0; i<size; i++, huffman+=1) {
        code = huffman[0].code;
        bits = huffman[0].length;
        //
        // number of bits used to encode the value field
        // Symbol is defined by the JPEG spec as:
        //    |  4 bits of Runlength of 0's  |  4 bits of amplitude  |
        // (amplitude is the number of bits required to code the
        // coefficient)
        //

        //
        // Handling for 16 bit entry
        //
        if(i == 16) {   
            vbits = 16 ;
        }
        else {
            vbits = i & 0x0f;
        }
        
        if(bits == 0) {
            continue;
        } else
            //
            // If the Huffman code length is <=8, we can decode it using
            // a single table lookup using a byte-sized index.
            //
            if(bits <= 8) {
                xbits = 8 - bits;
                xcount = 1 << xbits;
                code = code << xbits;
                if(vbits > xbits) {
                    //
                    // This will be a type 1 table entry because the number of
                    // bits in the value field + number of bits of Huffman
                    // code exceeds 8 and therefore cannot be full decoded
                    // in a single lookup using byte indexes
                    //
                    // Create a table entry for each possible combination of
                    // unused bits
                    //
                    for(j = 0; j < xcount; j++) {
                        hptr[code] = (i << 16) | (j << 8) | (xbits << 2) | 1;
                        code += 1;
                    }
                } else {
                    xbits -= vbits;
                    //
                    // Create a table entry for each possible combination of
                    // unused bits and values.
                    //
                    for(j = 0; j < xcount; j++) {
                        val = j >> xbits;
                        left = j & ((1 << xbits) - 1);
                        if((val & (1 << (vbits - 1))) == 0)
                            val = val - (1 << vbits) + 1;
                        hptr[code] = (val << 24) | (i << 16) | 
                                     (left << 8) | (xbits << 2) | 3;
                        code += 1;
                    }
                }
            } else {
                xbits = bits - 8;
                //
                // index 'j' is the high 8 bits of code (code length <= 16)
                //
                j = code >> xbits;
                if(hptr[j] == 0) {
                    cptr = new int[DECODE_TBL_LEN];
                    if(cptr == NULL) {
                       XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE); 
                       return 0;
                    }
                    memset(cptr, 0, sizeof(int)*DECODE_TBL_LEN);
                    hptr[j] = (int) cptr;
                } else {
                    cptr = (int *) hptr[j];
                }
                xbits = 16 - bits;
                xcount = 1 << xbits;
                //
                // 'code' is low order bits of code that were not used in
                // initial table lookup
                //
                code = (code << xbits) & 0xff;
                if(vbits > xbits) {
                    for(j = 0; j < xcount; j++) {
                        cptr[code] = (i << 16) | (j << 8) | (xbits << 2) | 1;
                        code += 1;
                    }
                } else {
                    xbits -= vbits;
                    for(j = 0; j < xcount; j++) {
                        val = j >> xbits;
                        left = j & ((1 << xbits) - 1);
                        if((val & (1 << (vbits - 1))) == 0)
                            val = val - (1 << vbits) + 1;
                        cptr[code] = (val << 24) | (i << 16) | 
                                     (left << 8) | (xbits << 2) | 3;
                        code += 1;
                    }
                }
            }
    }
    //
    // Insert error handling entries into unused table entries
    //
    for(i=0; i<DECODE_TBL_LEN; i++) {
        if((hptr[i] & 3) == 0) {
            if(hptr[i] == 0) {
                hptr[i] = JPEG_ERROR_ENTRY;
            } else {
                int* h2ptr = (int *) hptr[i];
                for(j=0; j<DECODE_TBL_LEN; j++) {
                    if(h2ptr[j] == 0) {
                        h2ptr[j] = JPEG_ERROR_ENTRY;
                    }
                }
            }
        }
    }
    return (hptr);
}


//------------------------------------------------------------------------
//
//  Function:        readtoscan
//
//  Description:
//        
//          .... read from the current position within frame to
//          the beginning of the next scan data, updateing header
//          data as you go ....
//        
//  Parameters:
//        None
//        
//  Returns:
//        0 if an error is encountered
//      1 if OK
//        
//  Side Effects:
//        Fills in header and updates bitstream pointer
//
//  Deficiencies/TODO:
//        Should we try to avoid creating a fast Huffman decode table
//        when the new Huffman table is the same as the old Huffman table?
//        (Performance boost for JPEG Interchange format bitstreams which
//        include all tables with each frame).
//
//------------------------------------------------------------------------

int 
JpegParser::readtoscan()
{
    int            code;
    int            data;
    int            i,j;
    Xil_unsigned8  count[16];
    Xil_unsigned8  list[DECODE_TBL_LEN];
    int            symbols, type, number;
    Xil_unsigned8* symbol;
    Huffman_Code*  table;
    int            maxh = 0;
    int            maxv = 0;
    Xil_unsigned8* pend = endOfBuffer;
    
    //
    //.... scan ahead to next marker ....
    //
    while(1) {
        NEXTBYTE(rdptr, pend, data);
        
        if(data != MARKER) continue;

        while(data == MARKER) {
            NEXTBYTE(rdptr, pend, data);
        }

        // Start Of Image marker
        if(data == SOI) {
            header.restartInterval = 0;
        }
        // Start Of Frame marker: Baseline or extended Huffman
        else if(data == SOF(0) || (data == SOF(1))) {
          int bands;
          // if cis is JpegLL and bitstream is baseline return
          if(decode_mode == JPEGLL) {
            XIL_ERROR(NULL, XIL_ERROR_USER, "di-284", TRUE) ;
            return 0 ;
          }
            
            //
            //.... skip byte count....
            //
            GETSHORT(rdptr, pend, data);
            //
            // Make sure sample precision is 8
            //
            NEXTBYTE(rdptr, pend, data);
            header.precision = data;
            
            if(data != 8) {
                JPEG_ERROR( XIL_ERROR_CIS_DATA, "di-74", TRUE, TRUE, rdptr);
                return 0;
            }
            
            GETSHORT(rdptr, pend, header.height);
            GETSHORT(rdptr, pend, header.width);
            
            if(header.height <= 0) {
                XIL_ERROR( NULL, XIL_ERROR_CIS_DATA, "di-75", TRUE); 
                return 0;
            }
            if(header.width <= 0) {
                XIL_ERROR( NULL, XIL_ERROR_CIS_DATA, "di-76", TRUE); 
                return 0;
            }
            NEXTBYTE(rdptr, pend, bands);
            if(header.band_data != NULL && header.bands != bands) {
                delete header.band_data;
                header.band_data = NULL;
            }
            if(header.band_data == NULL) {
                header.band_data = new banddata[bands];
                if(header.band_data == NULL) { 
                    XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
                    return 0;
                }
            }
            
            header.bands = bands;
            for(i = 0; i < bands; i++) {
                int h,v;
                NEXTBYTE(rdptr, pend, data);
                header.band_data[i].id = data;
                NEXTBYTE(rdptr, pend, data);
                header.band_data[i].h = h = data >> 4;
                header.band_data[i].v = v = data & 15;
                header.band_data[i].p = h * v;
                if(h > maxh)
                    maxh = h;
                if(v > maxv)
                    maxv = v;
                NEXTBYTE(rdptr, pend, data);
                header.band_data[i].qindex = data & 3;
                header.band_data[i].history = 0;
            }
        } else
          // SOF for Lossless Mode
          if(data == SOF(3)) {
            int bands;
            // If cis is jpeg, and bitstream is lossless return
            if(decode_mode == JPEG) { 
            XIL_ERROR(NULL, XIL_ERROR_USER, "di-284", TRUE) ;
            return 0 ;
          }

            
            // Toss the length
            GETSHORT(rdptr, pend, data);

            //
            // Sample precision - make sure its 8 or 16
            //
            NEXTBYTE(rdptr, pend, data);
            header.precision = data;
            if((header.precision != 8) && (header.precision != 16)) {
                XIL_ERROR( NULL, XIL_ERROR_USER, "di-74", TRUE);
                return 0 ;
            }
            
            // Height
            GETSHORT(rdptr, pend, header.height);
            if(header.height <= 0 ) {
                XIL_ERROR( NULL, XIL_ERROR_USER, "di-75", TRUE);
                return 0 ;
            }

            // Width
            GETSHORT(rdptr, pend, header.width);
            if(header.width <= 0 ) {
                XIL_ERROR( NULL, XIL_ERROR_USER, "di-76", TRUE);
                return 0 ;
            }

            // Number of components
            NEXTBYTE(rdptr, pend, bands);

            //
            // Alloc array of banddata structs 
            //
            delete header.band_data;
            header.band_data = new banddata[bands];
            if(header.band_data == NULL) { 
                XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
                return 0;
            }

            //
            // Get SOF data for each component
            //
            header.bands = bands;
            for(i=0; i<bands; i++) {
                int h,v;

                //
                // Component ID
                //
                NEXTBYTE(rdptr, pend, data);
                header.band_data[i].id = data;

                //
                // H and V samping factors - 4 bits each
                //
                NEXTBYTE(rdptr, pend, data);
                header.band_data[i].h = h = data >> 4;
                header.band_data[i].v = v = data & 15;
                header.band_data[i].p = h * v;
                if(h > maxh) {
                    maxh = h;
                }
                if(v > maxv) {
                    maxv = v;
                }

                //
                // Quant Table Index (N/A to lossless)
                //
                NEXTBYTE(rdptr, pend, data);
                header.band_data[i].qindex = data;

                header.band_data[i].history = 0;
            }

        } else
            // Define Huffman Table marker
             if(data == DHT) {
              
                
                int max_table_class;
                if(decode_mode == JPEGLL) {
                    max_table_class = 0;
                } else {
                    max_table_class = 1;
                }

                GETSHORT(rdptr, pend, symbols);
                symbols -= 2;
                table = new Huffman_Code[DECODE_TBL_LEN];
                if(table==NULL) {
                   XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE); 
                   return 0;
                }
                while(symbols > 0) {
                     //
                     // create table of huffman codes, indexed by symbol value
                     //
                     memset(table, 0, sizeof(Huffman_Code)*DECODE_TBL_LEN);
                     NEXTBYTE(rdptr, pend, type);
                     if( ((type & 0xf) > MAX_HUFFMAN_TABLE_ID) ||
                          ((type>>4) > max_table_class)) {
                        JPEG_ERROR( XIL_ERROR_CIS_DATA,"di-90",TRUE,TRUE,rdptr);
                       return 0;
                     }
                     if((type>>4)) {
                         SET_ACTABLE_BIT(definedInFrame,(type&3));
                     } else {
                         SET_DCTABLE_BIT(definedInFrame,(type&3));
                     }
                     
                     //
                     // Read number of codes for each of 16 code lengths
                     //
                     number = 0;
                     for(i = 0; i < 16; i++) {
                         NEXTBYTE(rdptr, pend, data);
                         count[i] = data;
                         number += data;
                     }
                     if(number >= DECODE_TBL_LEN) {
                        XIL_ERROR( NULL, XIL_ERROR_USER, "di-98", TRUE);
                        return 0 ;
                     }

                     
                     // Read symbols corresponding to each code
                     for(i = 0; i < number; i++) {
                         NEXTBYTE(rdptr, pend, data);
                         list[i] = data;
                     }
                     code = 0;
                     symbol = list;
                     //
                     // Loop over each code length
                     // Codes are implicit in the JPEG standard.
                     // Within a code length, codes are numbered sequentially.
                     // The first code for a given length is
                     // 2 * (highest_code_for_previous_code_length + 1)
                     //
                     for(i = 0; i < 16; i++) {
                         for(j = 0; j < (int)count[i]; j++) {
                             table[symbol[j]].length = i + 1;
                             table[symbol[j]].code = code;
                             code += 1;
                         }
                         symbol += count[i];
                         code <<= 1;
                     }
                     // Create fast decode table
                     if(header.decoder[type])
                        free_decode_table(header.decoder[type]);
                     header.decoder[type] = create_decode_table(table, DECODE_TBL_LEN);
                     if(!header.decoder[type]) {
                        delete table;
                        return 0;
                     }
                     symbols -= number + 16 + 1;
                  }
                  delete table;
        } else
            //Define Quantization Table marker
            if(data == DQT) {
                int cnt, tableindex;
                
                
                GETSHORT(rdptr, pend, cnt);
                cnt = (cnt - 2) / 65;
                while(cnt > 0) {
                    NEXTBYTE(rdptr, pend, tableindex);
                    if(tableindex & 0xf0) {
                       JPEG_ERROR( XIL_ERROR_CIS_DATA,"di-79",TRUE,TRUE, rdptr);
                       return 0;
                    }
                    if(tableindex > MAX_JPEG_QUANT_INDEX) {
                       // Jpeg bitstream error: tableindex too large
                       JPEG_ERROR( XIL_ERROR_CIS_DATA,"di-91",TRUE,TRUE, rdptr);
                       return 0;
                    }
                    SET_QTABLE_BIT(definedInFrame,tableindex);
                    for(i = 0; i < 64; i++) {
                        NEXTBYTE(rdptr, pend, data);
                        quantizer[tableindex][i] = data;
                    }
                    cnt -= 1;
                }
            } else
                //Start Of Scan marker Baseline Mode
                if(data == SOS && decode_mode == JPEG) {
                    int bands, id;
                    int sumhv = 0;
                    int* ptable;
                    
                    
                    GETSHORT(rdptr, pend, data);
                    NEXTBYTE(rdptr, pend, bands);
                    if(bands < 1 || bands > MAX_BANDS_PER_SCAN) {
                        XIL_ERROR( NULL, XIL_ERROR_USER, "di-98", TRUE);
                        return 0 ;
                    }
                    header.scanbands = bands;
                    for(i=0; i<bands; i++) {
                        NEXTBYTE(rdptr, pend, id);
                        for(j=0; (j < header.bands) && 
                            (id != header.band_data[j].id); j++)
                            ;
                        if(j == header.bands) {
                            XIL_ERROR( NULL, XIL_ERROR_USER, "di-93", TRUE);
                            return 0 ;
                        }
                        NEXTBYTE(rdptr, pend, data);
                        header.scancomponents[i] = j;
                        //
                        // Test for bad old movies
                        // Inherited from 1.2 (lperry).
                        // Moved out of decompress into here.
                        //
                        if(i != 0 && j == 0 ) {
                            XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-95", TRUE)
                            return 0;
                        }

                        if((data >> 4) > MAX_HUFFMAN_TABLE_ID) {
                         JPEG_ERROR(XIL_ERROR_CIS_DATA,"di-84",TRUE,TRUE,rdptr);
                         return 0;
                        }
                        
                        if((data & 15) > MAX_HUFFMAN_TABLE_ID) {
                         JPEG_ERROR(XIL_ERROR_CIS_DATA,"di-83",TRUE,TRUE,rdptr);
                         return 0;
                        }
                        ptable = header.decoder[data >> 4];
                        if(!ptable) {
                         JPEG_ERROR(XIL_ERROR_CIS_DATA,"di-88",TRUE,TRUE,rdptr);
                         return 0;
                        }
                        header.band_data[j].dctable = ptable;
                        ptable = header.decoder[16 + (data & 15)];
                        if(!ptable) {
                         JPEG_ERROR(XIL_ERROR_CIS_DATA,"di-87",TRUE,TRUE,rdptr);
                         return 0;
                        }
                        header.band_data[j].actable = ptable;
                        sumhv += header.band_data[j].p;
                    }
                    if(sumhv < 1 || sumhv > MAX_SCAN_HV) {
                        XIL_ERROR( NULL, XIL_ERROR_USER, "di-98", TRUE);
                        return 0 ;
                    }
                    NEXTBYTE(rdptr, pend, data);
                    NEXTBYTE(rdptr, pend, data);
                    NEXTBYTE(rdptr, pend, data);
                    // Break out of while(1) loop because we have found a scan
                    break;
                } else
                // Start of Scan Marker Lossless Mode
                if(data == SOS && decode_mode == JPEGLL) {
                    int bands, id;
                    
                    
                    // Toss away length
                    GETSHORT(rdptr, pend, data);

                    // Number of components
                    NEXTBYTE(rdptr, pend, bands);
                    if(bands < 1 || bands > MAX_BANDS_PER_SCAN) {
                      XIL_ERROR( NULL, XIL_ERROR_USER, "di-98", TRUE);
                      return 0 ;
                    }
                    header.scanbands = bands;

                    int old_id = -1;
                    for(i=0; i<bands; i++) {
                        // Get component ID
                        NEXTBYTE(rdptr, pend, id);

                        //
                        // Components don't necessarily come in order
                        // 
                        if(id<0 || id>=header.bands) {
                          XIL_ERROR( NULL, XIL_ERROR_USER, "di-93", TRUE);
                          return 0 ;
                        }

                        //
                        // Test for bad old movies in which
                        // there are duplicate component IDs
                        // TODO: What kinds of errors were being
                        //       encountered here. This test seems
                        //       inadequate. (lperry)
                        //
                        if(id == old_id) {
                            XIL_ERROR(NULL, XIL_ERROR_SYSTEM, "di-95", TRUE);
                            return 0;
                        }

                        for(j=0; (j<header.bands) && 
                                  (id != header.band_data[j].id); j++)
                          ;
                        if(j == header.bands) {
                            XIL_ERROR(NULL, XIL_ERROR_USER, "di-93", TRUE);
                            return 0;
                        }
                        NEXTBYTE(rdptr, pend, data);
                        header.scancomponents[i] = j;
                        if((data >> 4) > MAX_HUFFMAN_TABLE_ID) {
                          JPEG_ERROR(XIL_ERROR_CIS_DATA,"di-84",TRUE,TRUE,rdptr);
                          return 0;
                        } 
                        header.band_data[j].table = header.decoder[data >> 4];
                    }
                    NEXTBYTE(rdptr, pend, data);        // read Ss = predictor selection
                    if((data<1) || (data>7)) {
                        XIL_ERROR( NULL, XIL_ERROR_USER,"di-73",TRUE);  
                        return 0;
                    }
                    header.scan_selection = data;
                    NEXTBYTE(rdptr, pend, data);        // read Se (ignore)
                    NEXTBYTE(rdptr, pend, data);        // read Ah|Al
                    header.scan_pt_transform = data & 0xf; // Al = pt transform
                    // Break out of while(1) loop because we have found a scan
                    break;
            } else
              // Define Restart Interval
                if(data == DRI) {
                    GETSHORT(rdptr, pend, data);
                    GETSHORT(rdptr, pend, data);
                    header.restartInterval = data;
            } else {
                //
                // Process other markers.  Some can be skipped, some
                // indicate bitstreams which we do not accept.
                //
                // XXX Right now, just skip most of them
                //
                int cnt;

                if(data == SOF(2) ||( (data > SOF(3)) && (data <= SOF(15)))) {
                    JPEG_ERROR( XIL_ERROR_CIS_DATA, "di-72", TRUE,TRUE, rdptr); 
                    return 0;
                  }
                  
                // These markers are stand-alone
                if(data == MARKER || data == 0 || data == EOI) {
                    ;
                } else {
                    // These markers are the start of segments
                    GETSHORT(rdptr, pend, cnt);
                    cnt -= 2;
                    for(int k=0; k < cnt; k++)
                        NEXTBYTE(rdptr, pend, data);
                }
            }

    }
    // If there was no SOF in this frame, avoid setting maxh/v
    if(maxh > 0)
        header.maxh = maxh;
    if(maxv > 0)
        header.maxv = maxv;

    //
    // Some sanity checks
    //
    if(header.maxh == 0 || header.maxv == 0 ||
        header.height <= 0) {
        XIL_ERROR( NULL, XIL_ERROR_USER, "di-98", TRUE);
        return 0 ;
    }
        
    return 1;

ErrorReturn:
    //
    // We get here when the bytstream reading macros detect that we have
    // run off the end of the bytstream.
    //
    XIL_ERROR( NULL, XIL_ERROR_USER, "di-98", TRUE);
    return 0 ;

}  

JpegParser::~JpegParser() {

}

void 
JpegParser::burnFrames(int nframes)
{
  int bands;
  
  while(nframes > 0) {
    
    // get next frame pointer
    rdptr = cbm->nextFrame(&endOfBuffer);
    
    if(rdptr == NULL) {
      XIL_ERROR( NULL, XIL_ERROR_SYSTEM, "di-98", TRUE); 
      return;
    }
    
    bands = 0;
    
    // We want to do a readtoscan for each scan because there might
    // be header definitions preceeding the scan and we musn't skip
    // them..  Also, this is an easy way to skip over marker segments
    // which might inadvertantly contain a MARKER EOI.
    
    //
    // Subtract 2 from endOfBuffer because we need to test if we
    // can read 2 bytes from rdptr.  endOfBuffer points to 1 byte
    // past where we can read, plast_2bytes points to the last 2
    // legal bytes in the buffer
    //
    Xil_unsigned8* plast_2bytes = endOfBuffer - 2;
    definedInFrame = 0;
    do {
      //
      // Parse through header
      //
      if(!readtoscan())
          return;              // error in readtoscan
      //
      // Skip past Huffman encoded bytes to either next scan
      // or EOI.  Leave rdptr pointing to MARKER MARKER_CODE
      // We do this by stopping at the first MARKER segment that is
      // not MARKER 0 or MARKER RST*
      //
      do {
        if(rdptr > plast_2bytes) {
            XIL_ERROR( NULL, XIL_ERROR_USER, "di-98", TRUE);
            return;
        }
        //
        // If this is not a marker segment, or this is a restart marker,
        // keep skipping
        //
        if(rdptr[0] != MARKER ||
              (rdptr[1] == 0 ||                        // stuffed Huffman code 
              rdptr[1] == MARKER ||                // optional MARKER byte 
              (rdptr[1] >= RST(0) && rdptr[1] <= RST(7))))
            rdptr++;
        else
            break;
      } while(1);
    } while((bands += header.scanbands) < header.bands);
      
    //
    // Move rdptr past MARKER EOI
    //
    rdptr += 2;
    CheckSeekable();
    cbm->decompressedFrame(rdptr,JPEG_FRAME_TYPE);
    
    nframes--;
  }
}


int
JpegParser::getOutputType(unsigned int* width, 
                          unsigned int* height, 
                          unsigned int* nbands, 
                          XilDataType*  datatype) 
{
    if(header.width != 0) {
        *width  = header.width;
        *height = header.height;
        *nbands = header.bands;
        if(header.precision <= 8) {
            *datatype = XIL_BYTE;
        } else {
            *datatype = XIL_SHORT;
        }

        return 1;
    }

    //
    // Get next frame pointer
    //
    Xil_unsigned8* pend;
    if((rdptr = cbm->nextFrame(&pend)) == NULL) {
        return 0;
    }


    //
    // Scan until the next SOF (start of frame) is found
    //
    int data;
    while(1) {
        NEXTBYTE(rdptr, pend, data);
        if(data != MARKER) {
            continue;
        }
        while(data == MARKER) {
            NEXTBYTE(rdptr, pend, data);
        }

        switch(data) {
          case SOF(0):
          case SOF(1):
          case SOF(3):
            // Toss length field
            GETSHORT(rdptr, pend, data);

            // Precision
            NEXTBYTE(rdptr, pend, data);
            if(data <= 8) {
                *datatype = XIL_BYTE;
            } else {
                *datatype = XIL_SHORT;
            }
            GETSHORT(rdptr, pend, *height);
            GETSHORT(rdptr, pend, *width);
            NEXTBYTE(rdptr, pend, *nbands);
            return 1;

          //
          // These markers have no additional contents,
          // so just discard them
          //
          case MARKER: 
          case 0:      // Stuffed zero byte
          case EOI:
          case SOI:
            break;

          default:
            //
            // These markers are the start of segments
            // Get length and discard segment contents
            //
            int count;
            GETSHORT(rdptr, pend, count);
            count -= 2;
            for(int i=0; i<count; i++) {
                NEXTBYTE(rdptr, pend, data);
            }
        }
    }

  ErrorReturn:
    //
    // We get here when the bytstream reading macros detect that we have
    // run off the end of the bytstream.
    //  
    XIL_ERROR( NULL, XIL_ERROR_USER, "di-98", TRUE);
    return 0 ;
}
