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
//  File:       cosines.hh
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:23:42, 03/10/00
//
//  Description:
//
//    Definitions of cosine constants for the DCT alogorithm.
//    All constants are scaled by 2^16 (65536)
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)cosines.hh	1.2\t00/03/10  "

#ifndef COSINES_H
#define COSINES_H

#define COS_SCLE	65536
#define COS_HALF	2048
#define COS_SHFT	12

#define COS_00	   32768
#define COS_01	   45451
#define COS_02	   42813
#define COS_03	   38531
#define COS_04	   32768
#define COS_05	   25746
#define COS_06	   17734
#define COS_07	    9041

#define COS_10	   45451
#define COS_11	   63042
#define COS_12	   59384
#define COS_13	   53444
#define COS_14	   45451
#define COS_15	   35710
#define COS_16	   24598
#define COS_17	   12540

#define COS_20	   42813
#define COS_21	   59384
#define COS_22	   55938
#define COS_23	   50343
#define COS_24	   42813
#define COS_25	   33638
#define COS_26	   23170
#define COS_27	   11812

#define COS_30	   38531
#define COS_31	   53444
#define COS_32	   50343
#define COS_33	   45308
#define COS_34	   38531
#define COS_35	   30274
#define COS_36	   20853
#define COS_37	   10631

#define COS_40	   32768
#define COS_41	   45451
#define COS_42	   42813
#define COS_43	   38531
#define COS_44	   32768
#define COS_45	   25746
#define COS_46	   17734
#define COS_47	    9041

#define COS_50	   25746
#define COS_51	   35710
#define COS_52	   33638
#define COS_53	   30274
#define COS_54	   25746
#define COS_55	   20228
#define COS_56	   13933
#define COS_57	    7103

#define COS_60	   17734
#define COS_61	   24598
#define COS_62	   23170
#define COS_63	   20853
#define COS_64	   17734
#define COS_65	   13933
#define COS_66	    9598
#define COS_67	    4893

#define COS_70	    9041
#define COS_71	   12540
#define COS_72	   11812
#define COS_73	   10631
#define COS_74	    9041
#define COS_75	    7103
#define COS_76	    4893
#define COS_77	    2494

#endif // COSINES_H
