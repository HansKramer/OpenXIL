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
#include "H261CompressorData.hh"

H261CompressorData::H261CompressorData()
{
    version = 0;
    reset();
}



void H261CompressorData::reset() 
{
    version++;
    BitsPerImage  = 5069;
    ImageSkip     = 0;
    searchX       = 15;
    searchY       = 15;
    LoopFilter    = TRUE;
    EncodeIntra   = FALSE;
    FreezeRelease = FALSE;
    SplitScreen   = FALSE;
    DocCamera     = FALSE;
}
