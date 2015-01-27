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
//  File:	xili_hash_string.cc
//  Project:	XIL
//  Revision:	1.4
//  Last Mod:	10:16:33, 03/10/00
//
//  Description:
//	
//	
//	
//	
//	
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)xili_hash_string.cc	1.4\t00/03/10  "


#include <stdio.h>

//------------------------------------------------------------------------
//
//   Function:	hash()
//
//   Description:
//	This function is a fast hash function used by the library
//	for putting entries into the hash table.
//	
//      The fact that it is simply a sequence of lookups and additions 
//	make it quite fast for its very reasonable randomization
//      abilities.
//
//------------------------------------------------------------------------
unsigned int
xili_hash_string(const char* the_string)
{
    static unsigned int weights[256] = {
        618, 587, 491, 623, 517, 565, 914, 197, 519, 566, 592, 554, 408, 254,
        450, 756, 768, 619, 561, 539, 926, 340, 858, 788, 583, 112, 571,   3,
        895, 492, 867, 253, 160, 931, 415,  51, 580, 911, 845, 306, 144, 574,
        414, 416, 583, 337, 701, 977, 767, 270, 199, 143, 597,  69,  70, 944,
        160,  39, 614, 785, 116, 298, 114, 721, 406, 185, 324, 368, 239, 544,
        720, 678, 767, 718, 763, 704, 527, 171, 957,  52, 778, 689, 264, 366,
        585, 238, 380, 244, 890, 855, 937,  12, 644, 313, 528, 773, 770, 385,
        515, 176, 309, 536, 950, 172, 704, 227, 496, 125,  84, 390, 278, 369, 
        986, 536, 767, 648, 769, 782, 825, 152, 627, 315, 347, 919, 521, 402, 
        608, 787, 934, 872, 869, 676, 760, 583, 390, 356, 200, 829, 417, 464, 
        982, 126, 213, 961, 739, 410, 782, 760, 959,  28, 319, 759, 243, 591, 
         43, 958, 320,  59, 443, 917, 573, 119, 571, 252, 497, 237, 478, 407, 
        875, 428, 359, 383,  43, 161, 523, 698,  97, 402, 775, 245, 343, 230, 
        298, 305, 889,  36, 653, 399, 678, 734, 940, 233, 840, 970, 780, 432, 
        676, 811, 159, 280, 135, 866, 752, 208, 140, 295, 805, 219, 564, 717, 
        198, 992, 250, 431, 757, 863, 897, 980, 396, 433, 127, 459, 238, 988, 
        654, 606, 242, 456, 792,  79, 477, 153, 246, 947, 615, 991, 478, 802, 
        746, 381, 481, 528,  98, 595, 348, 143, 781, 713, 447, 706,  95, 965, 
        552, 742, 580, 639
    };

    unsigned int char_sum = 0;
    unsigned int num      = 0;
    while(*the_string) {
        char_sum += weights[*the_string++]<<((num++) % 32);
    }

    return char_sum;
}
