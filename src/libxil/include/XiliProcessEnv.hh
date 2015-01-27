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
//  File:	XiliProcessEnv.hh
//  Project:	XIL
//  Revision:	1.5
//  Last Mod:	10:21:30, 03/10/00
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
#pragma ident	"@(#)XiliProcessEnv.hh	1.5\t00/03/10  "

#ifndef _XILI_PROCESS_ENV
#define _XILI_PROCESS_ENV

#include "XiliHashTable.hh"
#include "XiliCharHashTable.hh"
#include "XiliTokenizer.hh"

//
//  Record for storing environment variables.
//
class XiliEnvRecord {
public:
    XiliEnvRecord(const char* var) {
        //
        //  Split the line up into colon separated tokens.
        //
        XiliTokenizer colon_tok(var, ":");
        
        char* next_colon_tok = colon_tok.getNext();
        
        while(next_colon_tok[0] != '\0') {
            //
            //  For each of these colon separated tokens, split them up into
            //  token separated by "=".
            //
            XiliTokenizer equal_tok(next_colon_tok, "=");

            //
            //  Our key and value for the table.
            //
            char* key    = equal_tok.getNext();
            char* value;

            //
            // Hack to look for link_? lines ala XIL 1.2
            //
            // Check to see if we have a LINK_? in the begining of the string.
            //
            if(!strncmp(key, "link_", 5)) {
                //
                //  Value is the remainder of the string after link --
                //  including the underscore -- and the key is LINK.
                //
                value = key+4;
                key   = "LINK";
            } else {
                value = equal_tok.getNext();
            }

            if((key = strdup(key)) == NULL) {
                XIL_ERROR(/* TODO: system state? */ NULL,
                          XIL_ERROR_RESOURCE, "di-1", TRUE);
                return;
            }
            
            if((value = strdup(value)) == NULL) {
                XIL_ERROR(/* TODO: system state? */ NULL,
                          XIL_ERROR_RESOURCE, "di-1", TRUE);
                return;
            }
            
            varTable.insert(key, value);

            free(key);

            next_colon_tok = colon_tok.getNext();
        }
    }

    XiliCharHashTable  varTable;
};

//------------------------------------------------------------------------
//
//  Class:	XiliProcessEnv
//
//  Description:
//    A class which processes the environment.
//
//    It will parse environment variables into key/value lists given the
//    following syntax:
//
//        setenv EnvVar "key1=value1:key2=value2:key3"
//
//    getVar() on EnvVar returns the whole string, NULL if not set.
//
//    getValue() on EnvVar, key1, returns "value1", if Value wasn't
//    set it returns NULL.
//
//    getValue() on EnvVar, key3 will return a pointer to a 
//    string with "" in it.
//
//    isVarSet() returns TRUE if the value was set in the given
//    environemnt variable, else FALSE.
//
//    isValueSet() returns TRUE if key was in envVar else FALSE.
//
//    getValue() with only the key checks XIL_DEBUG
//
//    isSet() with only the key checks XIL_DEBUG
//
//
//  MT-level:  Safe
//	
//  Notes:
//	The XiliString class is used here because it will cache the hash
//      value with the string so it doesn't have to be recomputed every
//      time.
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------

class XiliProcessEnv {
public:
    //
    //  Returns the value associated with key in the environment variable env_var.
    //
    static const char*    getValue(XiliString& env_var,
                                   XiliString& key);
    
    //
    // Checks to see if key has been set in env_var.
    //
    static Xil_boolean    isSet(XiliString& env_var,
                                XiliString& key);
    
    //
    //  Returns the unparsed string associated with env_var.
    //
    static const char*    getVar(XiliString& env_var);

    //
    //  Checks to see if env_var has been set by the user..
    //
    static Xil_boolean    isSet(XiliString& env_var);

    //
    //  Cleanup.  This is called by the global state when it's destroyed to
    //  cleanup any remaining data.
    //
    static void           cleanup();

private:    
    static XiliHashTable<XiliEnvRecord*>* envVarsTable;
    static XilMutex              tableMutex;
    static XiliEnvRecord*        checkAndInsert(XiliString& env_var);
};

#endif // _XILI_PROCESS_ENV
