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
//  File:	XiliOpTreeNode.hh
//  Project:	XIL
//  Revision:	1.25
//  Last Mod:	10:20:50, 03/10/00
//
//  MT Level:   UNsafe
//
//  Description:
//	
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliOpTreeNode.hh	1.25\t00/03/10  "

#ifndef _XILI_OP_TREE_NODE_HH
#define _XILI_OP_TREE_NODE_HH

//
//  System Includes
//

//
//  C++ Includes
//
#include "_XilDefines.h"
#include "_XilClasses.hh"
#include "XiliUtils.hh"
#include "XiliMarker.hh"
#include "_XilFunctionInfo.hh"

//
//  Internal data structures for the elements used to comprise the XIL
//    function lists.  These are needed by deferred execution which
//    traverses the tree and searches the function definitions.
//

//------------------------------------------------------------------------
//
//  Class:	XiliFunctionDef
//
// Description:
//	
//	
//	
//	
//	
//	
// Deficiencies/ToDo:
//	This linked list should probably be turned into an XiliList.
//	
//------------------------------------------------------------------------
class XiliFunctionDef {
public:
    //
    //  This class stores the information set by the IHV for the particular
    //  function.
    //
    XilFunctionInfo     funcInfo;

    //
    //  The device manager for the function.  We use this to lookup the device
    //  names -- or call the compute member functions.
    //
    XilDeviceManager*   deviceManager;

    //
    //  The type of the function -- COMPUTE/IO/CODEC.
    //
    XiliFuncType        functionType;

    //
    //  The priority of the function.  The list is sorted but this helps us
    //  get the priority quickly for insertion.
    //
    unsigned int        priority;

    XiliFunctionDef*    next;
};

//------------------------------------------------------------------------
//
//  Class:	XiliOpTreeNode
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
//------------------------------------------------------------------------
class XiliOpTreeNode {
public:
    //
    //  Constructor/Destructor
    //
                       XiliOpTreeNode();
                       ~XiliOpTreeNode();
    
    //
    //  This returns a pointer to the node in the Op tree that
    //  contains a list of function pointers which perform the
    //  operation designated by XilOpName object.  Combining
    //  operations is accomplished by successive branches.
    //
    XiliOpTreeNode*    branch(int op_num);
    
    //
    //  Designate the function that implements the function described
    //  at this particular location in the Op tree.  If multiple
    //  functions are defined at this location in the Op tree, they
    //  are called based on their capabilities as described in the
    //  function information.  All will be called up until one
    //  succeeds or all of them fail.  The function is expected to
    //  implement the operation as described in the derived XilOp the
    //  XilOpName represents.  The given funtion will be called by the
    //  XIL core when the operation is requested the application.
    //
    XilStatus          addFunction(XilDeviceManager* compute_manager,
                                   XilFunctionInfo*  function_info);
    
    //
    //  Get the next state table of op tree nodes for the given branch.
    //
    XiliOpTreeNode**   getNextState()
    {
        return nextState;
    }
    
    //
    //  Get the next state table size for the given branch.
    //
    unsigned int       getStateSize()
    {
        return nextStateSize;
    }

    //
    //  Get the list of functions for this node.
    //
    XiliFunctionDef*   getFunctionList()
    {
        return functionList;
    }
    
private:
    //
    //  An array XiliOpTreeNode pointers.
    //
    XiliOpTreeNode**     nextState;
    unsigned int         nextStateSize;
    
    //
    //  A linked list of function information of those which are available at this
    //  node.
    //
    XiliFunctionDef*     functionList;
};
#endif // _XIL_OP_TREE_NODE_HH
