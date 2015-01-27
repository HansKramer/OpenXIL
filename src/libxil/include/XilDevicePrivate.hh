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
//  File:	XilDevicePrivate.hh
//  Project:	XIL
//  Revision:	1.5
//  Last Mod:	10:22:08, 03/10/00
//
//  Description:
//	
//	
//	
//	
//	
//	
//	
//	
//  MT-level:  <??????>
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilDevicePrivate.hh	1.5\t00/03/10  "

public:
    //
    //  Required XilObject methods...
    //
    XilObject*         createCopy();

    //
    //  Set and attribute on this device.
    //
    void               setAttribute(const char* attr_name,
                                    void*       attr_value);

    //
    //  Constructors
    //
                       XilDevice(XilSystemState* state,
                                 const char*     device_name);

protected:
    //
    //  Destructor
    //
                       ~XilDevice();

private:
    char*              deviceName;

    unsigned int       arraySize;
    unsigned int       numAttrs;
    XilAttributeData*  attrTable;
