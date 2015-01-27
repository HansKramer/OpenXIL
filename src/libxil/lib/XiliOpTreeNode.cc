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
//  File:	XiliOpTreeNode.cc
//  Project:	XIL
//  Revision:	1.43
//  Last Mod:	10:08:06, 03/10/00
//
//  Description:
//	
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliOpTreeNode.cc	1.43\t00/03/10  "

//
//  System Includes
//
#include <string.h>
#ifdef _WINDOWS
#include <time.h>
#else
#include <sys/time.h>
#endif

//
//  C++ Includes
//
#include "_XilDefines.h"
#include "_XilGlobalState.hh"
#include "_XilDeviceManagerCompute.hh"

//
//  libxil Priavte Includes
//
#include "XiliOpTreeNode.hh"
#include "XiliProcessEnv.hh"
#include "XiliUtils.hh"
#include "XiliThread.hh"

//------------------------------------------------------------------------
//
//  Function:	XiliOpTreeNode::XiliOpTreeNode
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
//  MT-level:  SAFE
//	
//------------------------------------------------------------------------
XiliOpTreeNode::XiliOpTreeNode()
{
    functionList         = NULL;
    nextState            = NULL;
    nextStateSize        = 0;
}

//------------------------------------------------------------------------
//
//  Function:	XiliOpTreeNode::~XiliOpTreeNode
//
//  Description:
//	
//	
//	
//	
//	
//	
//------------------------------------------------------------------------
XiliOpTreeNode::~XiliOpTreeNode()
{
    //
    //  Delete the function list at this node.
    //
    while(functionList) {
        XiliFunctionDef* tmpfunc = functionList->next;

        delete functionList;

        functionList = tmpfunc;
    }

    //
    //  Delete all of the nodes pointed to by the next state.
    //
    for(unsigned int i=0; i<nextStateSize; i++) {
        delete nextState[i];
    }

    //
    //  The nextState and sizes array.
    //
    if(nextState != NULL) {
        delete [] nextState;
    }
}

//------------------------------------------------------------------------
//
//  Function:	XiliOpTreeNode::branch
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
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
XiliOpTreeNode*
XiliOpTreeNode::branch(int op_num)
{
    if(this == NULL) {
        return NULL;
    }

    //
    //  If the op_num is greater than the number of states we have in our
    //     node, then grow the array of states to include op_num.
    //
    if(nextStateSize <= (unsigned int)op_num) {
        unsigned int new_size = (op_num+1)<<1;
        //
        //  Grow the op array
        //
        XiliOpTreeNode** op_nodes = new XiliOpTreeNode*[new_size];
        if(op_nodes == NULL) {
            XIL_ERROR(NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
            return NULL;
        }

        if(nextState)
            xili_memcpy(op_nodes, nextState, 
                                  sizeof(XiliOpTreeNode*)*nextStateSize);

        for(unsigned int i=nextStateSize; i<new_size; i++) {
            op_nodes[i] = NULL;
        }

        if(nextState)
            delete [] nextState;

        nextState = op_nodes;

        //
        //  Update the size of the table.
        //
        nextStateSize = new_size;
    }


    //
    //  Do we already have a branch for this op?  If we don't, then create
    //     a new node which is the branch for this operation.
    //
    if(nextState[op_num] == NULL) {
        nextState[op_num] = new XiliOpTreeNode;
        if(nextState[op_num] == NULL) {
            XIL_ERROR(NULL, XIL_ERROR_RESOURCE, "di-1", TRUE);
            return NULL;
        }
    }

    return nextState[op_num];
}

//------------------------------------------------------------------------
//
//  Function:	XiliOpTreeNode::addFunction
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
//  MT-level:  UNsafe
//	
//  Parameters:
//	
//	
//  Returns:
//	
//	
//  Side Effects:
//	
//	
//  Notes:
//	
//	
//  Deficiencies/ToDo:
//	
//	
//------------------------------------------------------------------------
XilStatus
XiliOpTreeNode::addFunction(XilDeviceManager* device_manager,
                            XilFunctionInfo*  func_info)
{
    //
    //  Allocate a new function definition
    //
    XiliFunctionDef*  fdef = new XiliFunctionDef;
    if(fdef == NULL) {
        XIL_ERROR(NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
        return XIL_FAILURE;
    }

    //
    //  Copy the information from func_info into our structure.
    //
    //  We don't use direction lists any more in the function description now
    //  that multi-branch molecules are gone, but they're used up to this
    //  point.  So, we delete the direction list in our fdef and then clear it
    //  so it's not destroyed later.
    //
    delete fdef->funcInfo.directionList;

    fdef->funcInfo = *func_info;

    fdef->funcInfo.directionList = NULL;
    
    //
    //  Determine which type of device this is...
    //
    if(func_info->getIOFunction() != NULL) {
        fdef->functionType = XILI_IO_FUNC;
    } else if(func_info->getCodecFunction() != NULL) {
        fdef->functionType = XILI_CODEC_FUNC;
    } else {
        fdef->functionType = XILI_COMPUTE_FUNC;
    }

    //
    //  Set the device manager.
    //
    fdef->deviceManager       = device_manager;

    //
    //  If it's a compute device, then we get priority from the manager.
    //
    //  For I/O and Codec devices, we used a fixed priority of 50 because
    //  they're not going to be layered on top of one another but they do mix
    //  with compute devices.
    //
    fdef->priority = 50;
    if(fdef->functionType == XILI_COMPUTE_FUNC) {
        fdef->priority =
            ((XilDeviceManagerCompute*)fdef->deviceManager)->getPriority();
    }

    //
    //  Insert the new function into list of functions at this node based
    //  on the priority of the compute device by skipping over higher
    //  priority functions.
    //
    XiliFunctionDef* prev = NULL;
    XiliFunctionDef* cur  = functionList;
    while(cur != NULL) {
        if(fdef->priority >= cur->priority) {
            //
            //  Insert the new function here where it is after the higher
            //  priority functions and before the lower or equal priority
            //  functions.
            //
            break;
        }

        prev = cur;
        cur  = cur->next;
    }

    //
    //  Actually insert the function...
    //
    fdef->next = cur;

    if(prev != NULL) {
        prev->next = fdef;
    } else {
        //
        //  Update functionList to point at the new head of the
        //  list.
        //
        functionList = fdef;
    }

    return XIL_SUCCESS;
}
