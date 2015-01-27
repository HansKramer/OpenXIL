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
//  File:       QTable.cc
//  Project:    XIL
//  Revision:   1.2
//  Last Mod:   10:16:18, 03/10/00
//
//  Description:
//
//    Implementation of QTable Object.
//
//
//------------------------------------------------------------------------
//    COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)QTable.cc	1.2\t00/03/10  "


#include "QTable.hh"

//------------------------------------------------------------------------
//
//  Function:	QTable::QTable(unsigned int p)
//  Created:	92/03/24
//
//  Description:
//	
//    Constructor for QTable Object. Sets precision and initializes
//    tables values to zeros and mask to its corresponding value.
//	
//  Parameters:
//	
//    unsigned int p:   precision
//
//------------------------------------------------------------------------

QTable::QTable(unsigned int p)
{
  // Initialize QTable
  
  int i,j;
  
  for (i=0; i<QTABLE_HEIGHT; i++)
    for (j=0;j<QTABLE_WIDTH; j++)
      orig_table[i][j] = table[i][j] = 0;
  
  precision = p;
  mask = (precision == BIT_PRECISION_16) ? 0xffff : 0xff;
}

//------------------------------------------------------------------------
//
//  Function:	QTable::QTable( const QTable & b)
//  Created:	92/03/24
//
//  Description:
//	
//   Constructs a QTable from a QTable
//	
//  Parameters:
//	
//   const QTable& b:  reference to a QTable
//
//------------------------------------------------------------------------

QTable::QTable( const QTable & b)
{
  // Copy QTable
  
  int i,j;
  
  for (i=0; i<QTABLE_HEIGHT; i++)
    for (j=0;j<QTABLE_WIDTH; j++)
      orig_table[i][j] = table[i][j] = b.table[i][j] & mask;

}

//------------------------------------------------------------------------
//
//  Function:	QTable::Initialize(int val)
//  Created:	92/03/24
//
//  Description:
//	
//   Initializes each entry in a QTable with the same masked value.
//   The mask is used to make sure the value being placed into the
//   table is of the correct precision.
//
//  Parameters:
//
//   int value:   value to place into each entry of the table
//	
//  Side Effects:
//	
//   table is modified
//
//------------------------------------------------------------------------

void QTable::Initialize(int val)
{
  // fill QTable with val
  
  int i,j;
  int value = val & mask;
  
  for (i=0; i<QTABLE_HEIGHT; i++)
    for (j=0; j<QTABLE_WIDTH; j++)
      orig_table[i][j] = table[i][j] = value;

}

//------------------------------------------------------------------------
//
//  Function:	QTable::Initialize(int x[QTABLE_HEIGHT][QTABLE_WIDTH])
//  Created:	92/03/24
//
//  Description:
//	
//   Initializes a QTable with the values in a matrix. Each value
//   in the matrix is masked with the mask memebr variable. The mask
//   is used to make sure the value being placed into the table is of
//   the correct precision.
//
//  Parameters:
//
//   int x[QTABLE_HEIGHT][QTABLE_WIDTH]): matrix of values to put into table
//	
//  Side Effects:
//	
//   table is modified
//
//------------------------------------------------------------------------

void QTable::Initialize(int x[QTABLE_HEIGHT][QTABLE_WIDTH])
{
  int i,j;
  
  for (i=0; i<QTABLE_HEIGHT; i++)
    for (j=0;j<QTABLE_WIDTH; j++)
      orig_table[i][j] = table[i][j] = x[i][j] & mask;

}

//------------------------------------------------------------------------
//
//  Function:	QTable::Scale
//  Created:	92/03/24
//
//  Description:
//	
//   Scales a QTable by a given value: where a value of 50 is
//   normalization. Each value is masked so that precision is
//   maintained.
//	
//  Parameters:
//	
//    int value:  value to scale to
//
//  Side Effects:
//
//   table is modified
//	
//  Notes:
//	
//    Does this lean toward a JPEG implementation
//	
//------------------------------------------------------------------------

void QTable::Scale(int value)
{
  int i,j;

  for (i=0; i<QTABLE_HEIGHT; i++)
    for (j=0;j<QTABLE_WIDTH; j++){
      table[i][j] =  ((orig_table[i][j]*value)/50);
      if (table[i][j] > mask)
        table[i][j] = mask;
      else if (table[i][j]<=0)
        table[i][j] = 1;
    }

}

//------------------------------------------------------------------------
//
//  Function:	operator=(const QTable& b)
//  Created:	92/03/24
//
//  Description:
//	
//    Sets the values in QTable equal to those in another
//	
//  Parameters:
//	
//    const QTable& b:  reference to a QTable
//
//  Side Effects:
//	
//    table is modified
//
//------------------------------------------------------------------------

void QTable::operator=(const QTable& b)
{
  // Copy QTable
  
  int i,j;
  
  for (i=0; i<QTABLE_HEIGHT; i++)
    for (j=0;j<QTABLE_WIDTH; j++)
      orig_table[i][j] = table[i][j] = b.table[i][j] & mask;
}

