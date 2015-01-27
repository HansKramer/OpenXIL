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
//  File:	XiliProcessEnv.cc
//  Project:	XIL
//  Revision:	1.11
//  Last Mod:	10:08:23, 03/10/00
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
#pragma ident	"@(#)XiliProcessEnv.cc	1.11\t00/03/10  "

#include "_XilDefines.h"
#include "XiliProcessEnv.hh"

//
//  Static class variables
//
XiliHashTable<XiliEnvRecord*>* XiliProcessEnv::envVarsTable = NULL;
XilMutex                       XiliProcessEnv::tableMutex;

//
//  Create our own hash table destructor so we can use free() for the values
//  instead of delete.
//
#if !defined(GCC) && !defined(_WINDOWS) && !defined(HPUX)
XiliHashTable<char*>::~XiliHashTable()
{
    if(table) {
        for(int i=0; i<tableSize; i++) {
            XiliHashRecord* tmp = table[i];
            while(tmp) {
                XiliHashRecord* next = tmp->next;

                //
                //  Always allocated using strdup().
                //
                free(tmp->key);
                
                if(deleteValues) {
                    free(tmp->value);
                }
                
                delete tmp;

                tmp = next;
            }
        }

        delete table;
    }
}
#endif // !GCC & !_WINDOWS & !HPUX

void
XiliProcessEnv::cleanup()
{
    delete envVarsTable;

    envVarsTable = NULL;
}

static XiliEnvRecord* _xili_env_rec = NULL;

XiliEnvRecord*
XiliProcessEnv::checkAndInsert(XiliString& env_var)
{
    if(envVarsTable == NULL) {
        envVarsTable = new XiliHashTable<XiliEnvRecord*>;

        if(envVarsTable == NULL) {
            XIL_ERROR(/* system state? */NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
            return NULL;
        }
    }

    //
    //  Have we got it already?
    //
    XiliEnvRecord* env_rec;
    if(envVarsTable->lookup(env_var, env_rec) == XIL_SUCCESS) {
        return env_rec;
    }

    //
    //  Go get it from the environment...
    //
    const char* getenv_var = getenv(env_var);
    if(getenv_var == NULL) {
        //
        //  Add this non-existant environment variable to our table as such so
        //  we don't need to keep calling getenv(). 
        //
        envVarsTable->insert(env_var, _xili_env_rec);
    
        return NULL;
    }

    //
    //  It's set in the environment so read it and build a hash table
    //  entry for it.
    //
    env_rec = new XiliEnvRecord(getenv_var);
    if(env_rec == NULL) {
        return NULL;
    }

    //
    //  Add this environment variable to our table.
    //
    envVarsTable->insert(env_var, env_rec);
    
    return env_rec;
}

const char*
XiliProcessEnv::getVar(XiliString& env_var)
{
    return getenv(env_var);
}

const char*
XiliProcessEnv::getValue(XiliString& env_var,
                         XiliString& key)
{
    tableMutex.lock();

    XiliEnvRecord* env_rec = checkAndInsert(env_var);
    if(env_rec == NULL) {
        tableMutex.unlock();
        return NULL;        
    }

    char* value;
    if(env_rec->varTable.lookup(key, value) == XIL_FAILURE) {
        tableMutex.unlock();
        return NULL;        
    }
    
    tableMutex.unlock();

    return value;
}

Xil_boolean
XiliProcessEnv::isSet(XiliString& env_var)
{
    return (getenv(env_var) == NULL) ? FALSE : TRUE;
}

Xil_boolean
XiliProcessEnv::isSet(XiliString& env_var,
                      XiliString& key)
{
    const char* value = getValue(env_var, key);
    
    return (value == NULL) ? FALSE : TRUE;
}
    
