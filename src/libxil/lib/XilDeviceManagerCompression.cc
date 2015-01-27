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
//  File:	XilDeviceManagerCompression.cc
//  Project:	XIL
//  Revision:	1.10
//  Last Mod:	10:09:02, 03/10/00
//
//  Description:
//	
//  Provides the ability to get and set attributes for this
//  compression type.
//	
//  MT-Level:  <?????>
//
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XilDeviceManagerCompression.cc	1.10\t00/03/10  "


#include "_XilDefines.h"
#include "_XilCis.hh"
#include "_XilDeviceCompression.hh"
#include "_XilDeviceManagerCompression.hh"
#include "_XilSystemState.hh"

#include "XiliHashTable.hh"

typedef XiliHashTable<AttrRecord*>* AttribHashTablePtr;

XilStatus
XilDeviceManagerCompression::getAttr(XilDeviceCompression* xdc,
                                     const char*           name,
                                     void**                value)
{
    //
    // Casting necessary to hide hash table implementation
    // TODO: Something a bit more elegant is needed ?
    //
    AttribHashTablePtr attr = (AttribHashTablePtr)attrTable;

    AttrRecord* symbol;
    if(attr->lookup(name, symbol) != XIL_SUCCESS) {
        // Tried to get unknown attribute 
        XIL_OBJ_ERROR((xdc->getCis())->getSystemState(), XIL_ERROR_USER,
                    "di-17", TRUE, xdc->getCis());
        return XIL_FAILURE;
    } else if (symbol->get == NULL) {
        // No get() function. This is a set-only attribute.
        XIL_OBJ_ERROR((xdc->getCis())->getSystemState(), XIL_ERROR_USER,
                    "di-18", TRUE, xdc->getCis());
        return XIL_FAILURE;
    } else {
        *value = (xdc->*(symbol->get))();
        return XIL_SUCCESS;
    }
}


XilStatus
XilDeviceManagerCompression::setAttr(XilDeviceCompression* xdc,
                                     const char*           name, 
                                     void*                 value)
{
    //
    // Casting necessary to hide hash table implementation
    // TODO: Something a bit more elegant is needed ?
    //
    AttribHashTablePtr attr = (AttribHashTablePtr)attrTable;

    AttrRecord* symbol;
    if(attr->lookup(name, symbol) != XIL_SUCCESS) {
        // Tried to set unknown attribute 
        XIL_OBJ_ERROR((xdc->getCis())->getSystemState(), XIL_ERROR_USER,
                    "di-19", TRUE, xdc->getCis());
    } else if (symbol->set == NULL) {
        // No set() function. This is a get-only attribute .
        XIL_OBJ_ERROR((xdc->getCis())->getSystemState(), XIL_ERROR_USER,
                    "di-20", TRUE, xdc->getCis());
    } else {
        (xdc->*(symbol->set))(value);
        return XIL_SUCCESS;
    }

    return XIL_FAILURE;
}


// Register attribute is setup to remember only the last registered
// version of an attribute.  Since the Compression Specific attributes
// are registered last, this allows a compression to override the default
// implementation of an attribute.

void 
XilDeviceManagerCompression::registerAttr(char*       name, 
                                          setAttrFunc set, 
                                          getAttrFunc get)
{
    //
    // Casting necessary to hide hash table implementation
    // TODO: Something a bit more elegant is needed ?
    //
    AttribHashTablePtr attr = (AttribHashTablePtr)attrTable;

    AttrRecord* symbol;
    if(attr->lookup(name, symbol) != XIL_SUCCESS) {
        //
        // No attribute of that name yet. So create a new one 
        // and insert it in the symbol table.
        //
        symbol = new AttrRecord(set, get);
        if(symbol == NULL) {
            XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
            return;  
        }
        attr->insert(name, symbol);
    } else {
        // Just change the set/get function ptrs
        symbol->set = set;
        symbol->get = get;
    }
}

    
Xil_boolean
XilDeviceManagerCompression::isOK()
{
    _XIL_ISOK_TEST();
}

XilDeviceManagerCompression::XilDeviceManagerCompression(
    char* cname, 
    char* ctype)
{
    isOKFlag = FALSE;
    
    compressor             = cname;
    compression_type       = ctype;

    //
    // Create a hash table to hold the attributes
    //
    attrTable = (void*) (new XiliHashTable<AttrRecord*>);
    if(attrTable == NULL) {
        XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1", TRUE);
    } 

    isOKFlag = TRUE;
}


XilDeviceManagerCompression::~XilDeviceManagerCompression()
{
    delete (AttribHashTablePtr)attrTable;
}

char*
XilDeviceManagerCompression::getCompressor() 
{ 
    return compressor; 
}

char*
XilDeviceManagerCompression::getCompressionType()
{
    return compression_type;
}
