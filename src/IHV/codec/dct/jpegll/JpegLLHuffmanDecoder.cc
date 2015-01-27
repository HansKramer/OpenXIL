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
//  File:       JpegLLHuffmanDecoder.cc
//  Project:    XIL
//  Revision:   1.4
//  Last Mod:   10:14:58, 03/10/00
//
//  Description:
//
//    JpegLL Bitstream decoding
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)JpegLLHuffmanDecoder.cc	1.4\t00/03/10  "

#include "xil/xilGPI.hh"
#include "JpegLLHuffmanDecoder.hh"
 

//------------------------------------------------------------------------
//
//  Function:	decode_ll
//
//  Description:
//	Huffman decode the next token in the JPEG bitstream
//	
//  Parameters:
//	ppstream	pointer to pointer to bitstream memory
//	table		Huffman decode table
//	output		pointer to output decoded data 
//	count		count of decoded tokens
//	
//  Returns:
//	nothing
//------------------------------------------------------------------------
void JpegLLHuffmanDecoder::decode_ll(Xil_unsigned8** ppstream, 
                                     int*            dctable,
				     Xil_signed16*   output,
				     unsigned int    loop_count,
				     Xil_unsigned8*  pend)
				
{
   int *bptr = bitsleftover;
   register int leftover = bptr[0];
   register int lastbyte = bptr[1];
   register int one = 1;
   register int index, data, type;
   int *table = dctable;

   GETBYTE_INIT;
     
   do {
     
     /*
       .... table entries are formed as shown:
       
       type 0: unresolved symbol
       
       +--------+--------+--------+--------+
       | address of second level table - 00|
       +--------+--------+--------+--------+
       
       or type 1: resolved symbol, unresolved value
       
       symbol word
       |  unused bits of index
       |     |  # of unused index bits
       |     |     |
       V     V     V
       +--------+--------+--------+--------+
       |00000000|SSSSSSSS|XXXXXXXX|BBBBBB01|
       +--------+--------+--------+--------+
       
       or type 3: resolved symbol, resolved value
       
       symbol value
       |   symbol word
       |     |  unused bits of index
       |     |     |  # of unused index bits
       |     |     |     |
       V     V     V     V
       +--------+--------+--------+--------+
       |VVVVVVVV|SSSSSSSS|XXXXXXXX|BBBBBB11|
       +--------+--------+--------+--------+
       
       .... get an 8-bit index into first level
       huffman decoding table and check for second
       level lookup table (Note: a valid integer
       address must have two LSBs == 00) ....
       */
     
     if (leftover < 8) {
	  GETBYTE(index);
	  lastbyte = (lastbyte << 8) | index;
     } else {
	  leftover -= 8;
     }
     index = lastbyte >> leftover;
     
     data = table[index];
     type = data & 3;
     
     if (type == 3)
    {
	   /*
	       .... OR in the unused bits of the index into the
	       lastbyte, increase the count of leftover bits by
	       the number of unused index bits, and shift data
	       to symbol boundry ....
		 */
      // increase the count of leftover bits
      leftover += (data >> 2) & 0x3f;
      // Remove the index bits which were used in this decode
      lastbyte = lastbyte & ((one << leftover) - one);
      data >>= 16;
    }
     else
    {

	   
	   if (type == 0)
	    {
		 /*
		     .... get an 8-bit index into second-level
		     huffman decoding table ....
		     */
		 GETINDEX(index);
		 data = ((int *) data)[index];
		 type = data & 3;
	    }
	   
	   /*
	       .... OR in the unused bits of the index into the
	       lastbyte, increase the count of leftover bits by
	       the number of unused index bits, and shift data
	       to symbol boundry ....
		 */
	   // increase the count of leftover bits
	   leftover += (data >> 2) & 0x3f;
	   // Remove the index bits which were used in this decode
	   lastbyte = lastbyte & ((one << leftover) - one);
	   data = data >> 16;
	   
	   /*
	       .... in type 1 table entries the symbol value
	       must be extracted from the remaining bit-stream
	       (Note: "bits" will *always* be > 0, otherwise
	       the table entry would have been of type 3) ....
	       */
	   if (type == 1)
	    {
		 register int value, bits;
		 /*
		     .... read a "bits"-sized constant from the data
		     stream, where the following values coincide with
		     the preceeding values of bits; the leftmost value
		     corresponds with a 0 valued bit-stream and the
		     rightmost maps to a bit-stream value of 2^bits - 1
		     (Note: for positive values the bit-stream value ==
		     return value) ....
		     
		     bits	      values
		     1		-1,1
		     2	    -3,-2,2,3
		     3	 -7,-6,-5,-4,4,5,6,7
		     4      -15,-14,...,-8,8,...,14,15
		     5     -31,-30,...,-16,16,...,30,31
		     6     -63,-62,...,-32,32,...,62,63
		     7      -127,-126,...,-64,64,...,126,127
		     8     -255,-254,...,-128,128,...,254,255
		     9     -511,-510,...,-256,256,...,510,511
		     10   -1023,-1022,...,-512,512,...,1022,1023
		     11  -2047,-2046,...,-1024,1024,...,2046,2047

		     .... read enough bytes until there are at least "bits"
		     bits in the stream buffer word, lastbyte ....
		     */
	         if(data==16) {
                   data = MAX_DIFF_ENCODED<<8; // Special case do not look for bits in bs
		 }
		 else {
		   bits = data & 0x0f;
		   while (bits > leftover) {
		    int tmp;
		    GETBYTE(tmp);
		    lastbyte = (lastbyte << 8) | tmp;
		    leftover += 8;
		   }
		 
		   /*
		     .... extract value from bit-stream, then adjust for
		     negative valued constants, update lastbyte to reflect
		     extracted data, and append value to symbol ....
		     */
		   leftover -= bits;
		   value = lastbyte >> leftover;
		   if ((value & (one << (bits - one))) == 0) {
		      value = value - (one << bits) + one;
		   }
		   lastbyte = lastbyte & ((one << leftover) - one);
		   data |= (value << 8);
		}
	    }
    }
	 
   *output++ = (Xil_signed16) (data >> 8);

  } while (--loop_count >0);

  bptr[0] = leftover;
  bptr[1] = lastbyte;
  GETBYTE_END;
  return;


//   Since we do not want to hit the performance of the decoder
//  if the pointer is at the end of the buffer is found then the
//  decoder will continue working but will not core dump. It will
//  continue doing this until the next buffer is handed.

 ErrorReturn:
   GETBYTE_END ;
    return;
}

