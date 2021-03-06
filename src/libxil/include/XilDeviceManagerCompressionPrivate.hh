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
//  File:	XilDeviceManagerCompressionPrivate.hh
//  Project:	XIL
//  Revision:	1.4
//  Last Mod:	10:21:54, 03/10/00
//
//  Description:
//	
//	Provides the entry point for the dynamic loading of
//   a compression (XilCis)
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------

#ifdef _XIL_PRIVATE_INCLUDES

#include "XiliHashTable.hh"

class AttrRecord
{
public:
  AttrRecord(setAttrFunc p_set, getAttrFunc p_get)
    { get = p_get; set = p_set; }
  setAttrFunc set;
  getAttrFunc get;
};

#endif // _XIL_PRIVATE_INCLUDES
