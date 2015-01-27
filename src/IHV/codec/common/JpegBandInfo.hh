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
//  File:       JpegBandInfo.hh
//  Project:    XIL
//  Revision:   1.3
//  Last Mod:   10:23:40, 03/10/00
//
//  Description:
//
//   Jpeg Band Information
//
//   Describes information about a single band such as its name,
//   the quantization table associated with it, and the dc and ac
//   tables associated with it.
//        
// Notes:
//
//   All data is made public and no constructor is used so that
//   an instance of BandInfo may be initialized via = { ... }
//        
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)JpegBandInfo.hh	1.3\t00/03/10  "



#ifndef JPEGBANDINFO_H
#define JPEGBANDINFO_H

#include <xil/xilGPI.hh>

class JpegBandInfo {
public:
    void setId( Xil_unsigned8 id_val )            { id = id_val; }
    void setH( Xil_unsigned8 h_val )              {  h = h_val;  }
    void setV( Xil_unsigned8 v_val )              {  v = v_val;  }   
    void setQtableId(Xil_unsigned8 qtable_id_val) { qtable_id = qtable_id_val;}
    void setDcHtableId(Xil_unsigned8 dc_ht_id_val) { dc_htable_id = dc_ht_id_val;} 
    void setAcHtableId(Xil_unsigned8 ac_ht_id_val) { ac_htable_id = ac_ht_id_val;} 
    void setHtableId(Xil_unsigned8 htable_id_val) {htable_id = htable_id_val;}

    Xil_unsigned8 getId( void )           { return id; }
    Xil_unsigned8 getH( void )            { return h; }
    Xil_unsigned8 getV( void )            { return v; }
    Xil_unsigned8 getQtableId( void )     { return qtable_id; }
    Xil_unsigned8 getDcHtableId( void )   { return dc_htable_id; }
    Xil_unsigned8 getAcHtableId( void )   { return ac_htable_id; }
    Xil_unsigned8 getHtableId( void )     { return htable_id; }

private:
    Xil_unsigned8 id;
    Xil_unsigned8 h;
    Xil_unsigned8 v;
    Xil_unsigned8 qtable_id;
    Xil_unsigned8 dc_htable_id;
    Xil_unsigned8 ac_htable_id;
    Xil_unsigned8 htable_id ;

};

#endif








