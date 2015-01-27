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
//  File:	XiliNearestIndexSelector.cc
//  Project:	XIL
//  Revision:	1.5
//  Last Mod:	10:13:34, 03/10/00
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
#pragma ident	"@(#)XiliNearestIndexSelector.cc	1.5\t00/03/10  "

#include <math.h>

#include "XiliUtils.hh"
#include "XiliNearestIndexSelector.hh"

#ifdef NOCOMPILE
const unsigned int  BITS             = 16;
const unsigned int  SIGBITS          = 64;
#endif
const unsigned int  CODE_MASK        = 0xf;
const unsigned int  SHIFT1           = 4;
const unsigned int  RED_CODE_SHIFT   = 8;
const unsigned int  GRN_CODE_SHIFT   = 4;
const unsigned int  BLU_CODE_SHIFT   = 0;
const unsigned int  COLORSLIST_SIZE  = 4096; // BITS^3
const unsigned int  SUBCUBE_SIZE     = 64;

const unsigned int  SHIFT3           = 2;
#ifdef NOCOMPILE
const unsigned int  SHIFT3_SIZE      = 4;
#endif
const unsigned int  RED_ENTRY_SHIFT  = 12;
const unsigned int  GRN_ENTRY_SHIFT  = 6;
const unsigned int  BLU_ENTRY_SHIFT  = 0;
#ifdef NOCOMPILE
const unsigned int  CUBE_SIZE        = 262144; // SIG^3
#endif
const unsigned int  CACHE_SIZE       = 8192;   // Cache Entries
const unsigned int  HALF             = 8;
const double        BUMPDIST         = 27.712812921;  // 16*sqrt(3)
const double        FACTOR           = 5.856406461;   // bumpdst/2.0-8

inline unsigned int  
CODE(unsigned int x)  {  return (((x)>>4)&CODE_MASK); }

inline unsigned int  
ENTRY(unsigned int x) {  return ((x)>>SHIFT3); }

inline unsigned int  
EXTRACT_RED(unsigned int x) {
    return (((((x)>>RED_CODE_SHIFT)&CODE_MASK)<<SHIFT1)+HALF);
}

inline unsigned int  
EXTRACT_GREEN(unsigned int x) {
    return (((((x)>>GRN_CODE_SHIFT)&CODE_MASK)<<SHIFT1)+HALF);
}

inline unsigned int  
EXTRACT_BLUE(unsigned int x) {
    return (((((x)>>BLU_CODE_SHIFT)&CODE_MASK)<<SHIFT1)+HALF);
}

#define MAKE_KEY(r,g,b)  (codeB0[(r)]|codeB1[(g)]|codeB2[b])
#define MAKE_PART(r,g,b) ((((r)&0x3)<<4)|(((g)&0x3)<<2)|((b)&3))
#define MAKE_SUBC(r,g,b) ((((r)&0xc)<<2)|((g)&0xc)|(((b)&0xc)>>2))

//------------------------------------------------------------------------
//
//  Function:  XiliNearestIndexSelector()
//
//  Description:
//    Primes the colormap selection routines.
//    
//  Parameters:
//    size:
//        The colormap table size.
//
//    rbits, gbits, bbits:
//        These define how the colormap space will be split up.
//    4,4,4 would correspond to a search grid of 16x16x16 or 4096
//    regions.  Usually values 3, 4, or 5 are appropriate.
//
//    rsig, gsig, bsig:
//        These define the number of bits that are significant in the
//    colors.  Essentially, the proximity of the colors that will be
//    grouped together.  Values of 6,6,6 would correspond to using the
//    significant 6 bits of each red, green, and blue in determining the
//    colormap groupings.  Thus, a table of 216 is built for estimating
//    whether a nearest color has already been found.
//
//  Returns:  void.
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
XiliNearestIndexSelector::XiliNearestIndexSelector(XilSystemState*  state,
                                                   XilLookupSingle* init_lut)
{
    isOKFlag = FALSE;

    systemState = state;

    //
    //  Initialize all pointers to NULL to ensure the destructor will succeed
    //  even if the constructor doesn't.
    //

    int i;

    //
    //  Create the second level cache.
    //
    cache2 = new Xil_unsigned8[CACHE_SIZE][64];
    if(cache2 == NULL) {    
        XIL_ERROR(systemState, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }

    //
    //  colors is an array of linked list pointers.  The list is used to
    //  maintain the set of colors that are needed for a given region.
    //
    colors = new XiliHeadBucket*[COLORSLIST_SIZE];
    if(colors == NULL) {    
        XIL_ERROR(systemState, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }

    for(i=0; i<COLORSLIST_SIZE; i++) {
        colors[i] = NULL;
    }

    //
    //  Setup all of the tables used for computing the distance to the nearest
    //  neighbor.  Notice that the table is of size 514.  [0] points to the
    //  middle of the table.  Thus, negative offsets provide the value for
    //  the corresponding negative value.
    //
    //    absVal is simply the absolute value.
    //
    for(i=0; i<257; i++) {
        absValTable[i] = (256-i);
    }

    for(i=0; i<257; i++) {
        absValTable[256+i] = i;
    }
    absVal = &absValTable[256];

    //
    //  Setup a squares table for computing the square of a number (bytes)
    //
    for(i=0; i<257; i++) {
        squareValTable[i] = (256-i)*(256-i);
    }

    for(i=0; i<257; i++) {
        squareValTable[256+i] = i*i;
    }
    squareVal = &squareValTable[256];

    //
    //  Pre-compute the offsets into the assorted tables.
    //
    //  The rcodes are used to build an offset into the big cube which is
    //  dependent upon the number of significant color bits.  The resultant
    //  key/offset is computed by simply ORing the table entries together.
    //  Thus, all of the expensive shifting and masking is done at this
    //  point.  (key = codeB0[b0] | codeB1[b1] | codeB2[b2])
    //
    //  The rentrs are used to build a regional offset into the big cube.
    //
    for(i=0; i<256; i++) {
       int code  = CODE(i);
       codeB0[i] = code<<RED_CODE_SHIFT;
       codeB1[i] = code<<GRN_CODE_SHIFT;
       codeB2[i] = code<<BLU_CODE_SHIFT;

       int entr  = ENTRY(i);
       entrB0[i] = entr<<RED_ENTRY_SHIFT;
       entrB1[i] = entr<<GRN_ENTRY_SHIFT;
       entrB2[i] = entr<<BLU_ENTRY_SHIFT;
    }

    //
    //  Initialize for the lookup we've been given.
    //
    newColormap(init_lut);

    isOKFlag = TRUE;
}


XiliNearestIndexSelector::~XiliNearestIndexSelector()
{
    if(colors != NULL) {
        for(int i=0; i<COLORSLIST_SIZE; i++) {
            DeleteBucketList(colors[i]);
        }

        delete [] colors;
    }

    delete [] cache2;
}

//
//  Update with a new system state...
//
void
XiliNearestIndexSelector::setNewSystemState(XilSystemState* state)
{
    systemState = state;
}

//------------------------------------------------------------------------
//
//  Function:    AdaptiveColormapSelection::useNewColormap
//
//  Description:
//    Initializes for a index selection.  Must be called prior to
//    selecting indexes.
//    
//------------------------------------------------------------------------
XilStatus
XiliNearestIndexSelector::newColormap(XilLookupSingle* lookup)
{
    unsigned int i, j;

    if(lookup == NULL) {
        return XIL_FAILURE;
    }

    //
    //  Test to see if it's the same version as the one we're representing...
    //
    if(lookup->isSameAs(&version)) {
        return XIL_SUCCESS;
    }

    //
    //  Nope, so store this version away...
    //
    lookup->getVersion(&version);

    //
    //  First delete all existing buckets.
    //
    for(i=0; i<COLORSLIST_SIZE; i++) {
        DeleteBucketList(colors[i]);
        colors[i] = NULL;
    }

    //
    //  Initialize counts...
    //
    numEntries = lookup->getNumEntries();
    offset     = lookup->getOffset();
    
    //
    //  Start using the cache from the very top...
    //
    next_entry = 0;
    
    //
    //  Initialize our color arrays...
    //
    Xil_unsigned8* data = (Xil_unsigned8*)lookup->getData();

    if(data == NULL) {
        return XIL_FAILURE;
    }

    for(i=0, j=0; i<numEntries; i++, j+=3) {
        b0[i] = data[j];
        b1[i] = data[j+1];
        b2[i] = data[j+2];
    }

    return XIL_SUCCESS;
}

//------------------------------------------------------------------------
//
//  Function:    XiliNearestIndexSelector::initBucket()
//
//  Description:
//        Initializes a new bucket for a given search region.  This
//    does the nearest neighbor search over the specified region.
//    
//  Parameters:
//    key:
//        The region of interest.
//    
//  Returns: void.
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
void
XiliNearestIndexSelector::initBucket(int key)
{
    //
    //  Extract the colors from the key
    //
    int c0 = EXTRACT_RED(key);
    int c1 = EXTRACT_GREEN(key);
    int c2 = EXTRACT_BLUE(key);
    
    //
    //  Compute the distances to each color.
    //
    int mdist = 256*256*3;
    int tmp1;
    int tmp2;
    int distance[256];
    int d0[256];
    int d1[256];
    int d2[256];
    
    for(unsigned int i=0; i<numEntries; i++) {
        distance[i] = 256*256*3;

        d0[i] = absVal[c0-b0[i]]; tmp1  = squareVal[d0[i]];
        d1[i] = absVal[c1-b1[i]]; tmp1 += squareVal[d1[i]];
        d2[i] = absVal[c2-b2[i]]; tmp1 += squareVal[d2[i]];

        if(mdist > tmp1) {
            mdist = tmp1;
        }

        distance[i] = tmp1;
    }

    //
    //  The radius of the search circle
    //
    float adddist = (float) (BUMPDIST + sqrt((double)mdist));

    tmp1          = (int)(adddist*adddist + 0.9999);
    tmp2          = (int)(adddist-FACTOR  + 0.9999);

    xili_memset(init_colors, -1, 257*sizeof(int));

    int in_list_cnt = 0;
    
    for(i=0; i<numEntries; i++) {
        //
        //  Is it in the circle?
        //
        if(distance[i] <= tmp1) {
            //
            //  Is it really in the region of interest...
            //
            if((d0[i] <= tmp2) && (d1[i] <= tmp2) && (d2[i] <= tmp2)) {
                //
                //  Assume the color has not been found before.
                //
                int notin = 1;

                //
                //  The of colors list currently being constructed...
                //
                int* buck = init_colors;

                //
                //  Search the list to see if the color already exists for
                //  this region
                //
                while(*buck != -1) {
                    if(*buck++ == (int)i) {
                        notin = 0;
                    }
                }
        
                //
                //  If it wasn't in the list, then add it.
                //
                if(notin) {
                    in_list_cnt++;
                    *buck = i;
                }
            }
        }
    }

    //
    //  Initialize the bucket list
    //
    colors[key]->blist = new int[in_list_cnt+1];

    if(colors[key]->blist == NULL) {    
        XIL_ERROR(systemState, XIL_ERROR_RESOURCE, "di-1", TRUE);
        return;
    }

    //
    //  Copy the list into it's final location...
    //
    xili_memcpy(colors[key]->blist, init_colors, (in_list_cnt+1)*sizeof(int));
}


//------------------------------------------------------------------------
//
//  Function:    XiliNearestIndexSelector::selectCheck()
//
//  Description:
//        This function is called when cm_select finds a -1 in the
//    big cube.  Thus indicating that the corresponding color has not
//    been located so a search of the region must be performed.  This is
//    also called if we're in "lazy" mode and not using the big cube.
//    
//  Parameters:
//    r,g,b:
//        The red, green, and blue components of the desired color.
//    
//  Returns:
//    unsigned char:
//        The index into the colormap.
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
int
XiliNearestIndexSelector::selectCheck(int key,
                                      int ib0,
                                      int ib1,
                                      int ib2)
{
    int* buck = colors[key]->blist;

    int closest;

    //
    //  If there is only one entry for this region
    //
    if(*(buck+1) == -1) {
        closest = *buck;
    } else {
        //
        //  Run through the bucket and find which entry is the closest.
        //
        int mindist;
        int dist;
    
        closest = 0;
        mindist = 256*256*3;
        while(*buck != -1) {

            dist = squareVal[ib0-b0[*buck]] +
                   squareVal[ib1-b1[*buck]] +
                   squareVal[ib2-b2[*buck]];

            if(dist < mindist) {
                mindist = dist;
                closest = *buck;
            }
            buck++;
        }
    }

    return closest;
}

//------------------------------------------------------------------------
//
//  Function:    XiliNearestIndexSelector::selectIndex()
//
//  Description:
//        Selects a colormap entry based on the given red, green, and
//    blue components.
//    
//  Parameters:
//    r,g,b:
//        The red, green, and blue components of the desired color.
//    
//  Returns:
//    unsigned char:
//        The offset into the colormap.
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
int
XiliNearestIndexSelector::selectIndex(Xil_unsigned8 ib0,
                                      Xil_unsigned8 ib1,
                                      Xil_unsigned8 ib2)
{
    int entry;
    int key  = MAKE_KEY((int)ib0,  (int)ib1, (int)ib2);
    int subc = MAKE_SUBC((int)ib0, (int)ib1, (int)ib2);

    //
    //  If we haven't to this part of the cube before...
    //
    if(colors[key] == NULL) {
        colors[key]     = new XiliHeadBucket;
        colors[key]->sc = new XiliSubCube[SUBCUBE_SIZE];
        initBucket(key);
    }

    XiliSubCube* sc = &colors[key]->sc[subc];

    int  part = MAKE_PART(ib0, ib1, ib2);
    int  bit;
    int* mask;

    if(part > 31) {
        mask  = &sc->mask[1];
        bit   = (1<<(part-32));
    } else {
        mask  = &sc->mask[0];
        bit   = (1<<part);
    }

    //
    //  Have we ever been in this subcube before?
    //
    if(sc->mask[0] || sc->mask[1]) {
        if(sc->index > 255) {
            //
            //  It's a pointer to an array...
            //
            if(bit & (*mask)) {
                entry = sc->ptr[part];
            } else {
                entry = selectCheck(key, ib0, ib1, ib2);
                
                (*mask) |= bit;

                sc->ptr[part] = entry;
            }
      } else if (sc->index >= 0) {
          //
          //  We've found our index
          //
          if(bit & (*mask)) {
              entry = sc->index;
          } else {
              entry = selectCheck(key, ib0, ib1, ib2);

              if(entry != sc->index) {
                  //
                  //  Non-homogeneous subcube -- search
                  //
                  if(next_entry < CACHE_SIZE) {
                      //
                      //  But, there is still room in the cache
                      //
                      int tmp = sc->index;
                      
                      sc->ptr = cache2[next_entry++];
                      
                      //
                      //  Copy all things pointed at index into the table
                      //
                      for(int i=0; i<32; i++) {
                          if((1<<i) & sc->mask[0]) {
                              sc->ptr[i] = tmp;
                          }
                      }
                      
                      for(i=0; i<32; i++) {
                          if((1<<i) & sc->mask[1]) {
                              sc->ptr[i+32] = tmp;
                          }
                      }   

                      //
                      //  Set the new entry
                      //
                      sc->ptr[part] = entry;
                  } else {
                      //
                      //  Flag this as non-homogeneous without cache
                      //
                      sc->index = -1;
                  }
              }
              (*mask) |= bit;
          }
      } else {
          //
          //  We've run out of secondary cache -- search every time.
          //
          entry = selectCheck(key, ib0, ib1, ib2);
      }
    } else {
        //
        //  First time in this subcube
        //
        entry = selectCheck(key, ib0, ib1, ib2);
        
        (*mask) |= bit;
        sc->index = entry;
    }

    return entry + offset;
}

