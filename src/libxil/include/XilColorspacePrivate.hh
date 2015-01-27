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
//  File:	XilColorspacePrivate.hh
//  Project:	XIL
//  Revision:	1.8
//  Last Mod:	10:21:49, 03/10/00
//
//  Description:
//      This defines the private components of the XilColorspace object
//
//
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------

public:
    //
    // Constructor
    //
                         XilColorspace(XilSystemState*     system_state,
                                       const char*         name,
                                       XilColorspaceOpCode op_code, 
                                       unsigned int        num_bands);

                         XilColorspace(XilSystemState*   system_state,
                                       XilColorspaceType cspace_type,
                                       void*             cspace_data,
                                       Xil_boolean       copyFlag = FALSE);

    //
    // This constructor is a convenience used in conjunction
    // with KCMS to create a profile based colorspace for
    // use with IO device window images
    //
                         XilColorspace(XilSystemState*   system_state,
                                       Display*          display,
                                       int               screen,
                                       Visual*           visual);


    XilObject*           createCopy();

protected:
                         ~XilColorspace();

private:
    XilColorspaceType    type;
    void*                data;

    XilColorspaceOpCode  opcode;
    unsigned int         nbands;

    //
    //  Did we load the profile?
    //
    Xil_boolean          profileLoadedFlag;
    char*                profileData;

#ifdef _XIL_HAS_KCMS
    XilStatus           setProfileNBands();
    XilStatus           loadKcsProcs();
#endif // _XIL_HAS_KCMS





    

