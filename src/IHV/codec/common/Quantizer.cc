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
//  File:       Quantizer.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:16:22, 03/10/00
//
//  Description:
//
//    Implementation of Quantizer Class.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)Quantizer.cc	1.2\t00/03/10  "


#include "Quantizer.hh"

//------------------------------------------------------------------------
//
//  Function:	Quantizer::Quantizer
//  Created:	92/03/24
//
//  Description:
//
//    Sudo Constructor for Quantizer object. The number of tables and 
//    the precision of the tables are validated. If they are valid, 
//    then nt QTables are created of precision p.
//
//  Parameters:
//	
//    unsigned int nt:   number of tables
//
//    unsigned int p:    precision of tables
//
//    SingleBuffer*:       pointer to object through with data is added
//
//  Notes: This routine is meant to be called from the constructor
//    of a dervied class which should check bounds on num_tables and
//    table_precision before calling this routine.
//
//------------------------------------------------------------------------

void Quantizer::Init(unsigned int nt, unsigned int p, Xil_unsigned8 mark, SingleBuffer* buf)
{
  
  num_tables = nt;
  table_precision = p;
  
  if (num_tables != 0  && table_precision != UNKNOWN_PRECISION) {
    
    // Create array of QTable *'s
    qtables = new QTable*[num_tables];

    // create table usage array
    tables_in_use = new int[num_tables];
    tables_loaded = new int[num_tables];
    table_outputted = new int[num_tables];
    if (!qtables || !tables_in_use || !tables_loaded || !table_outputted) {
      num_tables = 0;
      return;
    }
    
    // Create each QTable
    for (int i=0; i<num_tables; i++){
      qtables[i] = new QTable(table_precision);
      if (!qtables[i]) {
	num_tables = 0;
	return;
      }
      tables_in_use[i] = 0;
      tables_loaded[i] = 0;
      table_outputted[i] = 0;
    }
        
  }
  
  buffer = buf;
  marker = mark;
  
}

//------------------------------------------------------------------------
//
//  Function:	Quantizer::~Quantizer
//  Created:	92/03/24
//
//  Description:
//	
//    Sudo Destructor for Quantizer object
//
//  Deficiencies/ToDo:
//
//    Use new methof for delete
//
//  Notes: This routine is meant to be called from the destructor
//    of a dervied class whose constructor called this class's
//    Init at some earlier time.
//
//------------------------------------------------------------------------

void Quantizer::Delete()
{
  for (int i=0; i<num_tables; i++)
    delete qtables[i];
  
  delete []qtables;
  delete []tables_in_use;
  delete []tables_loaded;
  delete []table_outputted;
}


//------------------------------------------------------------------------
//
//  Function:	Quantizer::Table
//  Created:	92/03/24
//
//  Description:
//	
//    Returns a pointer to the request table. Checks to see if the
//    table is legal. 
//	
//  Parameters:
//
//    int table:  table id 
//	
//  Returns:
//	
//    pointer to table if valid request. NULL otherwise.
//
//------------------------------------------------------------------------

QTable* Quantizer::Table(int table) const
{
  if (ValidTable(table))
    return qtables[table];
  else {
    // Invalid qtable identifier
    XIL_ERROR( NULL, XIL_ERROR_USER, "di-91", TRUE);
    return NULL;
  }
}

//------------------------------------------------------------------------
//
//  Function:	Quantizer::Add_Table(QTable* t, int as_table)
//  Created:	92/03/24
//
//  Description:
//	
//    Sets Quantization Table as_table to point to QTable passed in.
//    Table id as_table is verified; operation not completed if
//    id is not valid.
//
//  Parameters:
//
//    QTable* t:   Pointer to a QTable
//
//    int as_table:  table id
//	
//  Side Effects:
//	
//    If as_table id valid, current table pointed to at this id is
//    deleted and new table is set to point to t.
//
//  Notes:
//	
//    This may not be the best way to handle this case. Possible to
//    do a copy like Add_Table(QTable&, ...)
//	
//------------------------------------------------------------------------

void  Quantizer::Add_Table(QTable* t, int as_table)
{
  if (ValidTable(as_table)){
    QTable* tmp = qtables[as_table];
    qtables[as_table] = t;
    tables_loaded[as_table] = 1;
    table_outputted[as_table] = 0;
    delete tmp;
  } else {
    // Invalid qtable identifier
    XIL_ERROR( NULL, XIL_ERROR_USER, "di-91", TRUE); 
  }
}

//------------------------------------------------------------------------
//
//  Function:	Quantizer::Add_Table(QTable& t, int as_table)
//  Created:	92/03/24
//
//  Description:
//	
//	   Sets Quantization Table as_table equal to QTable passed in.
//	   Table id as_table is verified; operation not completed if
//    id is not valid. The equal operator is a copy.
//
//  Parameters:
//
//    QTable& t:   Reference to a QTable
//
//    int as_table:  table id
//	
//  Side Effects:
//	
//	   If as_table id valid, current table pointed to at this id is
//    changed. 
//
//------------------------------------------------------------------------

void  Quantizer::Add_Table(QTable& t, int as_table)
{
  if (ValidTable(as_table)){
    *qtables[as_table] = t;
    tables_loaded[as_table] = 1;
    table_outputted[as_table] = 0;
  } else {
    // Invalid qtable identifier
    XIL_ERROR( NULL, XIL_ERROR_USER, "di-91", TRUE);
  }
}

//------------------------------------------------------------------------
//
//  Function:	Quantizer::Add_Table(int x[QTABLE_WIDTH][QTABLE_HEIGHT],
//                                  int as_table)
//  Created:	92/03/24
//
//  Description:
//	
//    Initializes Quantization Table as_table with matrix passed in.
//    Table id as_table is verified; operation not completed if
//    id is not valid. 
//
//  Parameters:
//
//    int x[QTABLE_WIDTH][QTABLE_HEIGHT]: an matrix of quantization values
//
//    int as_table:  table id
//	
//  Side Effects:
//	
//    If as_table id valid, current table pointed to at this id is
//    changed. 
//
//------------------------------------------------------------------------

void  Quantizer::Add_Table(int x[QTABLE_WIDTH][QTABLE_HEIGHT], int as_table)
{
  if (ValidTable(as_table)){
    qtables[as_table]->Initialize(x);
    tables_loaded[as_table] = 1;
    table_outputted[as_table] = 0;
  } else {
    // Invalid qtable identifier
    XIL_ERROR( NULL, XIL_ERROR_USER, "di-91", TRUE);
  }
}

//------------------------------------------------------------------------
//
//  Function:  Quantizer::Scale
//  Created:	92/03/24
//
//  Description:
//	
//    Scales quantization table given table id by a given value.
//    If the table id is equal to ALL_QTABLES, then all tables
//    are scaled by the same amount.
//	
//  Parameters:
//	
//    int value:    value to scale to
//    int table:    quantization table id
//
//  Side Effects:
//	
//    Quantization tables are modified
//
//------------------------------------------------------------------------

void  Quantizer::Scale( int value, int table)
{
  if (table != ALL_QTABLES){
    if (ValidTable(table))
      qtables[table]->Scale(value);
    else{
      // Invalid qtable identifier
      XIL_ERROR( NULL, XIL_ERROR_USER, "di-91", TRUE);
    }
  }
  else
    for (int i = 0; i < num_tables; i++){
      qtables[i]->Scale(value);
      table_outputted[i] = 0;
    }
}


//------------------------------------------------------------------------
//
//  Function:	Quantizer::resetTableUsage()
//  Created:	92/05/ 6
//
//  Description:
//	
//	Resets table usage. A table is marked as being used if a
//      band of the current image uses it.
//	
//------------------------------------------------------------------------

void Quantizer::resetTableUsage()
{
  for (int i = 0; i < num_tables; i++)
    tables_in_use[i] = 0;
}


//------------------------------------------------------------------------
//
//  Function:	Quantizer::tableInUse(int table)
//  Created:	92/05/ 6
//
//  Description:
//	
//	Returns if a table is being used by a band of the current image
//      being compressed.
//	
//------------------------------------------------------------------------

int Quantizer::tableInUse(int table)
{
  if (ValidTable(table))
    return tables_in_use[table];
  else {
    // Invalid qtable identifier
    XIL_ERROR( NULL, XIL_ERROR_USER, "di-91", TRUE);
    return 0;
  }
}


//------------------------------------------------------------------------
//
//  Function:	Quantizer::usingTable(int table)
//  Created:	92/05/ 6
//
//  Description:
//	
//	Sets the usage of a table to true.
//	
//------------------------------------------------------------------------

void Quantizer::usingTable(int table)
{
  if (ValidTable(table))
    tables_in_use[table] = 1;
  else {
    // Invalid qtable identifier
    XIL_ERROR( NULL, XIL_ERROR_USER, "di-91", TRUE);
  }
}


//------------------------------------------------------------------------
//
//  Function:	Quantizer::numTablesInUse()
//  Created:	92/05/ 6
//
//  Description:
//	
//	Returns the number of tables being used.
//	
//------------------------------------------------------------------------

int Quantizer::numTablesInUse()
{
  int num_tables_in_use = 0;

  for (int t = 0; t < num_tables; t++)
    if (tables_in_use[t])
      num_tables_in_use++;

  return num_tables_in_use;
}


//------------------------------------------------------------------------
//
//  Function:	Quantizer::tableLoaded(int table)
//  Created:	92/05/ 6
//
//  Description:
//	
//	Returns the fact that a table has been loadd into the designated
//      slot.
//	
//------------------------------------------------------------------------

int Quantizer::tableLoaded(int table)
{
  if (ValidTable(table))
    return tables_loaded[table];
  else {
    // Invalid qtable identifier
    XIL_ERROR( NULL, XIL_ERROR_USER, "di-91", TRUE);
    return 0;
  }
}

//------------------------------------------------------------------------
//
//  Function:	Quantizer::tableOutputted(int table)
//  Created:	92/05/ 6
//
//  Description:
//	
//	Returns the fact that a given table has been outputted previously
//      into the byte-stream.
//	
//------------------------------------------------------------------------

int Quantizer::tableOutputted(int table)
{
  if (ValidTable(table))
    return table_outputted[table];
  else {
    // Invalid qtable identifier
    XIL_ERROR( NULL, XIL_ERROR_USER, "di-91", TRUE);
    return 0;
  }
}
