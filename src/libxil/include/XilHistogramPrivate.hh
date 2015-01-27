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
//  File:	XilHistogramPrivate.hh
//  Project:	XIL
//  Revision:	1.11
//  Last Mod:	10:21:26, 03/10/00
//
//  Description:
//		
//	Definition of private elements of Histogram Object
//	
//------------------------------------------------------------------------
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)XilHistogramPrivate.hh	1.11\t00/03/10  "
 
public:
    //
    //  Required virtual functions from XilObject
    //
    XilObject*    createCopy();

    //
    //  Constructor
    //
                  XilHistogram(XilSystemState* system_state,
                               unsigned int    nbands,
                               unsigned int*   nbins,
                               float*          low_value,
                               float*          high_value);

    //
    //  Get the number of bins for each band.
    //
    void          getNumBins(unsigned int* nbins);

    //
    //  Copies the array of floats that define the value of the first bin 
    //  for each band into the user-supplied array "low_values"
    //
    void          getLowValues(float* low_values);

    //
    //  Copies the array of floats that define the value of the last bin 
    //  for each band into the user-supplied array "high_values"
    //
    void          getHighValues(float* high_values);

    //
    //  Copies the histogram data into the user-supplied buffer "data"
    //
    void          getData(unsigned int* data);


protected:
                  ~XilHistogram();

private:
    unsigned int  nBands;
    unsigned int* nBins;
    unsigned int  nElements;
    float*        lowValue;
    float*        highValue;
    unsigned int* data;
