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
//  File:   AdaptiveColormapSelection.cc
//  Project:    XIL
//  Revision:   1.8
//  Last Mod:   10:16:01, 03/10/00
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
//  COPYRIGHT
//------------------------------------------------------------------------
#pragma ident   "@(#)AdaptiveColormapSelection.cc	1.8\t00/03/10  "

#include "AdaptiveColormapSelection.hh"

#include "XiliUtils.hh"

#define LAZY_TABLE
#define MSE
#undef  KEEP_STATS

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

const unsigned int  BOXSIZE          = 64;
const unsigned int  BOX_RSHIFT       = 2;
const unsigned int  BOX_GSHIFT       = 4;
const unsigned int  BOX_BSHIFT       = 6;

const unsigned int  SHIFT3           = 2;
#ifdef NOCOMPILE
const unsigned int  SHIFT3_SIZE      = 4;
const unsigned int  SS3              = 3;
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

const float         MOVT             = 0.9F;
const float         MSTT             = 12.0F;
const unsigned int  RANDOM_TABLE_SIZE = 1027;

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

#define MAKE_KEY(r,g,b)  (rcode[(r)]|gcode[(g)]|bcode[b])
#define MAKE_PART(r,g,b) ((((r)&0x3)<<4)|(((g)&0x3)<<2)|((b)&3))
#define MAKE_SUBC(r,g,b) ((((r)&0xc)<<2)|((g)&0xc)|(((b)&0xc)>>2))

#ifdef _WINDOWS
#define seed48(seed)	srand((unsigned int) seed)
#define drand48()	rand()
#endif

// 
// TODO : Got from 1.2 fmv_internal.h. Seems quite useless.
//        Remove later judging by its need.
//
#define CAST(a) (a)

AdaptiveColormapSelection*
AdaptiveColormapSelection::ok(void)
{
    if(this == NULL) {
        return NULL;
    } else {
        if(isok == TRUE) {
            return this;
        } else {
            delete this;
            return NULL;
        }
    }
}

void
AdaptiveColormapSelection::printStats(void)
{
    float tot = (float)
        (kntfirst + kntquick + kntslow + kntquick2 + kntslow2 + kntnocache);

    fprintf(stderr, "%d Buckets\n", kntbuckets);
    
    fprintf(stderr, "kntfirst= %8d;  kntquick= %8d;  kntquick2= %8d; \
kntslow= %8d; kntslow2= %8d; kntnocache= %8d\n",
            kntfirst, kntquick, kntquick2, kntslow, kntslow2, kntnocache);
    
    fprintf(stderr, "kntfirst= %8.3f;  kntquick= %8.3f;  kntquick2= %8.3f; \
kntslow= %8.3f; kntslow2= %8.3f; kntnocache= %8.3f\n",
            kntfirst/tot, kntquick/tot, kntquick2/tot, kntslow/tot,
            kntslow2/tot, kntnocache/tot);
}

//------------------------------------------------------------------------
//
//  Function:    AdaptiveColormapSelection::AdaptiveColormapSelection
//
//  Description:
//    Primes the colormap selection routines.  This should be called 
//    before subsequent calls to cm_init.
//    
//    
//    
//    
//    
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
AdaptiveColormapSelection::AdaptiveColormapSelection(void)
{
    isok   = FALSE;
    
    //
    //  Initialize all pointers to NULL to ensure the destructor will succeed
    //  even if the constructor doesn't.
    //
    absval = NULL;
    colors = NULL;
    randtbl= NULL;
    
    int i;
    //
    //  Create and Initialize all of the values in our "previously
    //  used" big cube to -1 indicating that none of the entries have
    //  been found.
    //
    cache2 = new Xil_unsigned8[CACHE_SIZE][64];
    if (cache2 == NULL) {    
      // out of memory error  
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
      return;
    }

    //
    //  Initialize attributes
    //
    threlo = 0;
    threhi = 0;
    
    //
    //  Initialize the random table.
    //
    randtbl   = new double[RANDOM_TABLE_SIZE];
    if (randtbl == NULL) {    
      // out of memory error  
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
      return;
    }    

    //
    //  Seed the drand48 to ensure the same set of random numbers every time.
    //
    // Had to use short because of the way it's defined in /usr/include
    unsigned short seed[3] = { 90, 21, 0 };

    seed48(seed);

    randplace = 0;
    for (i=0; i<RANDOM_TABLE_SIZE; i++) {
      randtbl[i] = drand48();
    }

    //
    //  colors is an array of linked list pointers.  The list is used to
    //  maintain the set of colors that are needed for a given region.
    //
    colors = new (HeadBucket*[COLORSLIST_SIZE]);
    if (colors == NULL) {    
      // out of memory error  
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
      return;
    }    

    for (i=0; i<COLORSLIST_SIZE; i++)
      colors[i] = NULL;

    //
    //  Setup all of the tables for computing the distance to the nearest
    //  neighbor.  Notice that the table is of size 514.  [0] points to the
    //  middle of the table.  Thus, negative offsets provide the value for
    //  the corresponding negative value.
    //
    //    absval is simply the absolute value.
    //
    abs_table = new int[514];
    if (abs_table == NULL) {    
      // out of memory error  
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
      return;
    }

    for (i = 0; i<257; i++) abs_table[i] = (256-i);
    for (i = 0; i<257; i++) abs_table[256+i] = i;
    absval = &abs_table[256];

    //
    //  Pre-compute the offsets into the assorted tables.
    //
    //  The rcodes are used to build an offset into the big cube which is
    //  dependent upon the number of significant color bits.  The resultant
    //  key/offset is computed by simply ORing the table entries together.
    //  Thus, all of the expensive shifting and masking is done at this
    //  point.  (key = rcode[r] | gcode[g] | bcode[b])
    //
    //  The rentrs are used to build a regional offset into the big cube.
    //
    int c;
    for (i=0;i<256;i++) {
       c = CODE(i);
       rcode[i] = c<<RED_CODE_SHIFT;
       gcode[i] = c<<GRN_CODE_SHIFT;
       bcode[i] = c<<BLU_CODE_SHIFT;

        c = ENTRY(i);
       rentr[i] = c<<RED_ENTRY_SHIFT;
       gentr[i] = c<<GRN_ENTRY_SHIFT;
       bentr[i] = c<<BLU_ENTRY_SHIFT;
    }
    isok = TRUE;
}


AdaptiveColormapSelection::~AdaptiveColormapSelection(void)
{
    if (colors) {
      for (int i=0;i<COLORSLIST_SIZE;i++)
         DeleteBucketList(colors[i]);

      delete colors;
    }

    delete   abs_table;
    delete [] cache2;
    delete   randtbl;
}

//------------------------------------------------------------------------
//
//  Function:    AdaptiveColormapSelection::useNewColormap
//
//  Description:
//    Initializes for a color selection.  Must be called prior to
//  selecting indexes.
//    
//  Parameters:
//    r, g, b:
//        The red, green, and blue components of the colormap.
//
//    u:
//        The array describing whether the corresponding colormap
//    entry was used.
//
//  Returns: void.
//    
//    
//  Side Effects:
//    
//    
//  Notes:
//    
//    
//  Deficiencies/ToDo:
//    Should we really be loosing all of the state that's in the
//  tables?  This should probably not wipe out all of the table, but
//  just accept a new colormap and use the state from the previous loop.
//  A function like this is needed because state shouldn't have to be
//  reconstructed for consecutive frames.
//    
//------------------------------------------------------------------------
void
AdaptiveColormapSelection::useNewColormap(const UsedCmapTable& cmap)
{
    int i;

    //
    //  First delete all previously allocated buckets.
    //
    for (i=0;i<COLORSLIST_SIZE;i++) {
       DeleteBucketList(colors[i]);
       colors[i] = NULL;
    }
    
    //
    //  Initialize counts...
    //
    colknt  = cmap.getNumEntries();
    

    //
    //  Start using the cache from the very top...
    //
    next_entry = 0;
    
    //
    //  Initialize the different arrays...
    //
    for (i = 0; i < colknt; i++) {
      use[i] = cmap.used(i);
    
      red[i] = cmap[i].band0();
      grn[i] = cmap[i].band1();
      blu[i] = cmap[i].band2();
    }

    clearStatTables();
}

void
AdaptiveColormapSelection::clearStatTables(void)
{
    //
    //  Zero statistic counters
    //
    kntbuckets= 0;
    kntfirst  = 0;
    kntquick  = 0;
    kntslow   = 0;
    kntquick2 = 0;
    kntslow2  = 0;
    kntnocache= 0;

    //
    //  Initialize the different arrays...
    //
    for (int i = 0; i < colknt; i++) {
      tkt[i] = 0;        // total count an entry was used
      trd[i] = 0;        // summation of red for an entry
      tgn[i] = 0;        // summation of green for an entry
      tbl[i] = 0;        // summation of blue for an entry
      tr2[i] = 0;        // square of red for an entry
      tg2[i] = 0;        // square of green for an entry
      tb2[i] = 0;        // square of blue for an entry
      erm[i] = 0;        // error for movement of that entry
    }
}    
    
//------------------------------------------------------------------------
//
//  Function:    AdaptiveColormapSelection::initBucket
//
//  Description:
//        Initializes a new bucket for a given search region.  This
//    does the nearest neighbor search over the specified region.
//    
//  Parameters:
//    k:
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
AdaptiveColormapSelection::initBucket(int key)
{
    //
    //  Extract the colors from the key
    //
    int cr    = EXTRACT_RED(key);
    int cg    = EXTRACT_GREEN(key);
    int cb    = EXTRACT_BLUE(key);
    
    //
    //  Compute the distances to each color.
    //
    int        mdist = 256*256*3;
    int        d1, d2;
    int        distance[256], dr[256], dg[256], db[256];
    
    for (int i=0; i<colknt; i++) {
      distance[i] = 256*256*3;
      if (use[i] <= 0)
        continue;

#ifdef TODO
      // Replace after sqrs_table is in
      dr[i] = absval[cr-red[i]]; d1  = sqrs_table[dr[i]];
      dg[i] = absval[cg-grn[i]]; d1 += sqrs_table[dg[i]];
      db[i] = absval[cb-blu[i]]; d1 += sqrs_table[db[i]];
#endif

      dr[i] = absval[cr-red[i]]; d1  = dr[i] * dr[i];
      dg[i] = absval[cg-grn[i]]; d1 += dg[i] * dg[i];
      db[i] = absval[cb-blu[i]]; d1 += db[i] * db[i];

      if (mdist > d1) mdist = d1;

      distance[i] = d1;
    }
    
    //
    //  The radius of the search circle
    //
    float adddist = (float) (BUMPDIST + sqrt((double)mdist));
    d1      = CAST(int)(adddist*adddist + 0.9999);
    d2      = CAST(int)(adddist-FACTOR  + 0.9999);

    xili_memset(init_colors, -1, 257*sizeof(int));
    int num_in_list = 0;
    
    for (i=0;i<colknt;i++) {
      if (use[i] <= 0)
        continue;
      if (distance[i] <= d1) {   // If in the circle
        //  If really in the region of interest...
        if ((dr[i] <= d2) && (dg[i] <= d2) && (db[i] <= d2)) {
          // Assume the color has not been found before
          int     notin = 1;
        
          // The list currently being constructed
          int* buck = init_colors;
        
          //
          //  Search the list to see if the color already
          //  exists for this region
          //
          while(*buck != -1) {
            if(*buck++ == i)
            notin = 0;
          }
        
          //
          //  If it wasn't, then add it.
          //
          if (notin) {
            num_in_list++;
            *buck = i;
          }
        }
      }
    }

    colors[key]->blist = new int[num_in_list+1];
                    
    if (colors[key]->blist == NULL) {    
      // out of memory error  
      XIL_ERROR( NULL, XIL_ERROR_RESOURCE,"di-1",TRUE);
      return;
    }

    xili_memcpy(colors[key]->blist, init_colors, (num_in_list+1)*sizeof(int));
}

//------------------------------------------------------------------------
//
//  Function:    AdaptiveColormapSelection::selectCheck
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
Xil_unsigned8
AdaptiveColormapSelection::selectCheck(int key, int r, int g, int b)
{
    int* buck = colors[key]->blist;

    int closest;
    if (*(buck+1) == -1) {    // If only entry for this region
      closest = *buck;
    } else {            // Find which entry is the closest
      int  mindist, dist;
    
      closest = 0;
      mindist = 256*256*3;
      while(*buck != -1) {

#ifdef TODO
      // Replace after sqrs_table is in
        dist = sqrs_table[r-red[*buck]] +
               sqrs_table[g-grn[*buck]] +
               sqrs_table[b-blu[*buck]];
#endif

        dist = ((r-red[*buck]) * (r-red[*buck])) +
               ((g-grn[*buck]) * (g-grn[*buck])) +
               ((b-blu[*buck]) * (b-blu[*buck]));

        if (dist < mindist) {
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
//  Function:    AdaptiveColormapSelection::selectIndex
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
AdaptiveColormapSelection::selectIndex(int r, int g, int b)
{
    int    entry;
    int key  = MAKE_KEY(r,g,b);

#ifdef LAZY_TABLE
    int  subc = MAKE_SUBC((int)r,(int)g,(int)b);

    if (colors[key] == NULL) {        // If we haven't been here before...
#ifdef KEEP_STATS
      kntbuckets++;
#endif
      colors[key] = new HeadBucket;
      colors[key]->sc = new SubCube[SUBCUBE_SIZE];
      initBucket(key);
    }
    
    SubCube* sc   = &colors[key]->sc[subc];

    int  part = MAKE_PART(r,g,b);
    int  bit;
    int* mask;

    if (part>31) {
      mask  = &sc->mask[1];
      bit   = (1<<(part-32));
    } else {
      mask  = &sc->mask[0];
      bit   = (1<<part);
    }

    //  Have we ever been in this subcube before?
    if (sc->mask[0] || sc->mask[1]) {  // Yes.
      if (sc->index > 255) {  // It's a pointer to an array
        if (bit&(*mask)) {
#ifdef KEEP_STATS
          kntquick2++;
#endif
          entry = sc->ptr[part];
        } else {
#ifdef KEEP_STATS
          kntslow2++;
#endif
          entry = selectCheck(key,r,g,b);
          (*mask) |= bit;
          sc->ptr[part] = entry;
        }                
      } else if (sc->index >= 0) {  // We've found our index
        if (bit&(*mask)) {
#ifdef KEEP_STATS
          kntquick++;
#endif
          entry = sc->index;
        } else {
#ifdef KEEP_STATS
          kntslow++;
#endif
          entry = selectCheck(key,r,g,b);

          if (entry != sc->index) {  // non-homogeneous subcube
            if (next_entry < CACHE_SIZE) {  // still room in cache
              int tmp = sc->index;
              sc->ptr = cache2[next_entry++];
                        
              // copy all things pointed at index into the table
              for (int i=0;i<32;i++) {
                 if ((1<<i)&sc->mask[0]) sc->ptr[i] = tmp;
              }
              for (i=0;i<32;i++) {
                 if ((1<<i)&sc->mask[1]) sc->ptr[i+32] = tmp;
              }

              // set the new entry
              sc->ptr[part] = entry;
            } else { // flag this as non-homogeneous without cache
              sc->index = -1;
            }
          }
          (*mask) |= bit;
        }
      } else {  // Ran outta secondary cache
#ifdef KEEP_STATS
        kntnocache++;
#endif
        entry = selectCheck(key,r,g,b);
      }
    } else {  // First time in this subcube
#ifdef KEEP_STATS
      kntfirst++;
#endif

      entry = selectCheck(key,r,g,b);

      (*mask) |= bit;
      sc->index = entry;
    }
    
#else
    
    if (colors[key] == NULL) {        // If we haven't been here before...
      colors[key] = new HeadBucket;
#ifdef KEEP_STATS
      kntbuckets++;
#endif
      initBucket(key);
    }
    
    entry = selectCheck(key,r,g,b);
#endif

    return entry;
}

//------------------------------------------------------------------------
//
//  Function:    selectIndexAdaptive
//
//  Description:
//    Does a selectIndex and then gathers statistics for the
//      adaptive phase.
//
//------------------------------------------------------------------------
int
AdaptiveColormapSelection::selectIndexAdaptive(int r, int g, int b)
{
    int entry = selectIndex(r,g,b);
    
    //
    //  Take statistics for adaptive colormap portion
    //
    
    // Count how many times this entry has been used
    tkt[entry]++;
    
    // Total of each color component for average
    trd[entry] += r;
    tgn[entry] += g;
    tbl[entry] += b;

    // Total of each color component squared
    // for standard deviation

#ifdef TODO
      // Replace after sqrs_table is in
    tr2[entry] += sqrs_table[r];
    tg2[entry] += sqrs_table[g];
    tb2[entry] += sqrs_table[b];
#endif
        
    tr2[entry] += r * r;
    tg2[entry] += g * g;
    tb2[entry] += b * b;
        
    //
    //  The euclidian distance between where I am and where I want to be.
    //

#ifdef TODO
      // Replace after sqrs_table is in
    int temp = sqrs_table[r-red[entry]] +
               sqrs_table[g-grn[entry]] +
               sqrs_table[b-blu[entry]];
#endif

    int temp = ((r-red[entry]) * (r-red[entry])) +
               ((g-grn[entry]) * (g-grn[entry]))+
               ((b-blu[entry]) * (b-blu[entry]));

    //
    //  Compute total error for the entry
    //
#ifdef MSE
    erm[entry] += temp;
#else
    erm[entry] += sqrt((float)temp);
#endif

    return entry;
}

//------------------------------------------------------------------------
//
//  Function:    AdaptiveColormapSelection::getNextColorMap
//
//  Description:
//    Completes the colormap selection process and returns the adapted colormap.
//    
//  Parameters:
//    r, g, b:
//        The red, green, and blue components of the next colormap.
//    u:
//        Denotes whether the color is used.
//    
//  Returns:  int - The colormap size.
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
AdaptiveColormapSelection::getNextColorMap(UsedCmapTable& cmap)
{
    int         i;
    int         newred[256], newgrn[256], newblu[256];
    int         boxes[BOXSIZE];

#ifdef VERBOSE
    //
    //  Statistics
    //
    int        ored[256], ogrn[256], oblu[256], olderm[256];
    int        turnoff=0;
    int        brekknt=0;
    int        resetknt=0;
    int        zeroknt=0;
    int        neg1knt=0;
    int        subtknt=0;
    int        sub4tknt=0;
    int        usedknt=0;
    int        useknt=0;
    int        cod2knt=0;
    int        cod3knt=0;
    int        errknt=0;
    int        histknt=0;
#endif

    //
    //  Compute Weighted Average and Initialize Outgoing Colors
    //
    unsigned long total_calls=0;
    unsigned long rcnt = 0L;
    unsigned long gcnt = 0L;
    unsigned long bcnt = 0L;
    for (i=0; i<colknt; i++) {
      total_calls += tkt[i];
      rcnt += red[i]*tkt[i];
      gcnt += grn[i]*tkt[i];
      bcnt += blu[i]*tkt[i];

      newred[i] = red[i];
      newgrn[i] = grn[i];
      newblu[i] = blu[i];
    }


#ifdef VERBOSE
    fprintf(stderr, "rave = %d; gave = %d; bave = %d\n", rave, gave, bave);
#endif
    
    //
    //  Partition colormap into boxes
    //
    for (i=0; i<BOXSIZE; i++) {
      boxes[i] = -1;
    }
    
    for (i=0; i<colknt; i++) {
      int boxindex =
            (red[i]>>BOX_RSHIFT)|(grn[i]>>BOX_GSHIFT)|(blu[i]>>BOX_BSHIFT);
      boxes[boxindex]++;
    }
    
    //
    //  Usage array from user falls into three classifications.
    //
    //     -1 -  the caller has indicated that the color can be
    //                 altered with no restrictions 
    //     0  -  nobody can use the color
    //     1  -  the color can be used
    //
    //  Go through every color and classify them based on usage statistics
    //  gathered from selectIndex.
    //  use[]:
    //     -1 - the color can be slammed to any value
    //      0 - the color can not be used until the user sets
    //          it's usage value to  -1
    //  avail[]:
    //      0 - nobody uses the color
    //      1 -  the color usage is under the threshold.  It is turned off and
    //            can not be used until the user sets it's usage to -1
    //      2 -  the color usage is under 4 times the threshold
    //      3 -  the color usage is over 4 times the threshold
    //

    int        redo[256];      // The entries that can be changed completely
    int        avail[256];     // Each entry's availability
    double     otkt[256];      // Used to dynamically modify the entry's count
    int        redoknt = 0;    // The number of redo entries.

#ifdef TODO
      // Replace after sqrs_table is in
    int        locut = 3*sqrs_table[32];
    int        hicut = 3*sqrs_table[48];
#endif

    long       total_err = 0L;

    for (i=0; i<colknt; i++) {

#ifdef VERBOSE
    ored[i] = newred[i];
    ogrn[i] = newgrn[i];
    oblu[i] = newblu[i];
    olderm[i] = erm[i];
#endif

      //
      //  otkt is for adjusting the count of each entry
      //
      otkt[i] = tkt[i];
        
      //
      //  Compute the Average Error of the image
      //
      total_err += erm[i];
        
      //
      //  Compute current threshold based on random location
      //    between optdist/10 and optdist/2 where optdist is the
      //    optimal distribution of colors based on the number of
      //    calls and the number of colors in the colormap
      //
      int  thre = (int)((double)threhi - (randtbl[randplace])*(double)(threhi-threlo));

#ifdef VERBOSE
        fprintf(stderr, "Using Threshold of %d, %8.3f between %d and %d\n",
            thre, randtbl[randplace], threlo, threhi);
#endif

      if(++randplace >= RANDOM_TABLE_SIZE) randplace = 0;
    
      if (tkt[i] == 0) {
        avail[i] = 0;    // Set classification

        //
        //  We'll only change those entries that can be slammed
        //  because they are not being used in the current image
        //  on the screen.
        //
        if (use[i] == -1) {
          redo[redoknt++] = i; // We can change this entry however we want
#ifdef VERBOSE
        neg1knt++;
#endif
        } else {
          use[i] = 0;    // Nobody uses it so turn it off until
                // the user sets the use value to -1
        }
#ifdef VERBOSE
        zeroknt++;
#endif
      } else if(tkt[i] < thre) {
        int boxindex = (newred[i]>>BOX_RSHIFT) |
                       (newgrn[i]>>BOX_GSHIFT) |
                       (newblu[i]>>BOX_BSHIFT);

        //
        //  Only leave one entry for movement in the designated box.
        //
        if (boxes[boxindex] > 1) { // turn it off
         avail[i] = 1;
         use[i]   = 0;
         boxes[boxindex]--;
#ifdef VERBOSE
        subtknt++;
#endif
        } else {  // turn it on
          avail[i] = 2;
          use[i]   = 2;
#ifdef VERBOSE
          sub4tknt++;
#endif
        }
      } else if (tkt[i] < 4*thre) {
        avail[i] = 2;    // Set classification
        use[i]   = 2;    // Set usage
#ifdef VERBOSE
        sub4tknt++;
#endif
      } else {
        avail[i] = 3;    // Set classification
        use[i]   = 2;    // Set usage
#ifdef VERBOSE
        usedknt++;
#endif
      }
    }

    average_err = ((double)total_err/(double)total_calls);

    //
    //  Move all class 2 and class 3 entries toward their average.
    //
#ifdef VERBOSE
    float  maxt=0.0,gott=0.0,actt=0.0;
#endif
    
    float  tmp,tmpr,tmpg,tmpb;
    float  fknt;
    for (i=0;i<colknt;i++) {
      if (avail[i] == 2 || avail[i] == 3) {
        fknt = (float) tkt[i];
        
        tmpr = trd[i]/fknt - newred[i];
        tmpg = tgn[i]/fknt - newgrn[i];
        tmpb = tbl[i]/fknt - newblu[i];
        tmp  = (float) sqrt(tmpr*tmpr+tmpg*tmpg+tmpb*tmpb);

#ifdef VERBISE
        if (tmp > maxt) maxt = tmp;
        if (MOVT*tmp > gott) gott = MOVT*tmp;
#endif
        //
        //  Check constraints
        //
        if (tmp > 1.75) {
          if (tmp*MOVT <= MSTT) {
            newred[i] += CAST(int)(MOVT*tmpr+0.5);
            newgrn[i] += CAST(int)(MOVT*tmpg+0.5);
            newblu[i] += CAST(int)(MOVT*tmpb+0.5);
#ifdef VERBOSE
            if (MOVT*tmp > actt) actt = MOVT*tmp;
            fprintf(stderr, "Moved2/3 %3d (%3d, %3d, %3d) toward (%8.3f, %8.3f, %8.3f)\n\t\ by (%3d, %3d, %3d) %8.3f wanted %8.3f\n",
                            i, r,g,b, trd[i]/fknt, tgn[i]/fknt, tbl[i]/fknt,
                            CAST(int)(MOVT*tmpr/tmp+0.5),
                            CAST(int)(MOVT*tmpg/tmp+0.5),
                            CAST(int)(MOVT*tmpb/tmp+0.5),
                            MOVT*tmp, tmp);
#endif
        } else {
          newred[i] += CAST(int)(MSTT*tmpr/tmp+0.5);
          newgrn[i] += CAST(int)(MSTT*tmpg/tmp+0.5);
          newblu[i] += CAST(int)(MSTT*tmpb/tmp+0.5);
#ifdef VERBOSE
          if (MSTT > actt) actt = MSTT;
            fprintf(stderr,
                            "Moved2/3 %3d (%3d, %3d, %3d) toward (%8.3f, %8.3f, %8.3f)\n\t\
 by (%3d, %3d, %3d) %8.3f wanted %8.3f\n",
                            i, r,g,b, trd[i]/fknt, tgn[i]/fknt, tbl[i]/fknt,
                            CAST(int)(MSTT*tmpr/tmp+0.5),
                            CAST(int)(MSTT*tmpg/tmp+0.5),
                            CAST(int)(MSTT*tmpb/tmp+0.5),
                            MSTT, tmp);
#endif
        }
        
        //
        //  Re-adjust error
        //
        erm[i] = (long) (erm[i] * 0.50);
      }
#ifdef VERBOSE
            else {
            fprintf(stderr, "Move2/3 %3d nowhere wanted %8.3f\n",
                i, tmp);
                }
#endif
    }
    }

#ifdef VERBOSE
    fprintf(stderr, "\n-------------------------\n\n");
#endif
    
    //
    //  Move all of the unused colors as necessary
    //
    int  placement[256];
    for (i=0; i<colknt; i++) placement[i] = 1;

    int  posneg = 1;
    int  slamval = -1;
    int  j;
    int  errvshist = -1;
    int     brekon;         // Which entry did we stop on?
    
    for (i=0; i<redoknt; i++) {
      //
      //  Find the pixel with the greatest average error or highest
      //  usage.  We'll flip based upon errvshist which is flipped after
      //  we're done moving cmap entries toward the current choice.
      //
      long   emax1 = 0L;
      double hmax1 = 0.0;
      int  cod1 = 0;

      //
      //  In this searching algorithm it is possible to not find a
      //  suitable candidate so brekon never gets set to anything.
      //  We check for this after the search.
      //
      brekon = -1;
        
      if (errvshist > 0) {    // Error
#ifdef VERBOSE
        fprintf(stderr, "Using Error\n");
        errknt++;
#endif
        for (j=0;j<colknt;j++) {
          if ((avail[j]==2||avail[j]==3) &&
             ((erm[j]/tkt[j]) > emax1)  &&
              (placement[j] > 0)) {
            brekon = j;
            emax1  = erm[j]/tkt[j];
            cod1   = avail[j];
          }
        }
      } else {        //  Count
#ifdef VERBOSE
        fprintf(stderr, "Using History\n");
        histknt++;
#endif
        for (j=0;j<colknt;j++) {
          if ((avail[j]==2||avail[j]==3) &&
              (otkt[j] > hmax1)          &&
              (erm[j] > 0)               &&
              (placement[j] > 0)) {
            brekon = j;
            hmax1  = otkt[j];
            cod1   = avail[j];
          }
        }
      }

      //
      //  Did we actually find a suitable entry?  If not, we're done.
      //
      if (brekon < 0) break;
        
#ifdef VERBOSE
    brekknt++;

    if (cod1 == 2)
      cod2knt++;
    else
      cod3knt++;
#endif
        
    //
    //  I start putting colors within the standard deviation or
    //  the variance of the color with the highest error or count.  The
    //  placement array is used to track where to place new colors
    //  when moving toward the color with the highest error or count.
    //  The posneg flag is used to place indexes at both the
    //  positive and negative variance.
    //
    //  The placements are as follows where variance_shift=variance*posneg:
    //    1 - Move redo to red+variance_shift,
    //               green+variance_shift, and blue+variance_shift 
    //    2 - Move redo to red, green+variance_shift, blue+variance_shift
    //    3 - Move redo to red+variance_shift, green+variance_shift, blue
    //    4 - Move redo to red+variance_shift, green, blue+variance_shift
    //
    
    //
    //  When we're moving to +variance, reset which color to move
    //  toward.
    //
    if(posneg == 1) {
        slamval = brekon;
#ifdef VERBOSE
        fprintf(stderr, "Reset slamval to %d\n", slamval);
#endif
    }

    //
    //  Need the count of the element as a float to compute the variances.
    //
    float  knt     = (float) tkt[slamval];
    float  redvari = (tr2[slamval]/knt - (trd[slamval]/knt)*(trd[slamval]/knt));
    float  grnvari = (tg2[slamval]/knt - (tgn[slamval]/knt)*(tgn[slamval]/knt));
    float  bluvari = (tb2[slamval]/knt - (tbl[slamval]/knt)*(tbl[slamval]/knt));
    
#ifdef VERBOSE
        fprintf(stderr, "Slamval %3d %3d\n", slamval, posneg);
        fprintf(stderr,
                "Vari %3d (%3d, %3d, %3d) %8.3f (%3d, %3d, %3d) (%3d, %3d, %3d) \
is (%8.3f, %8.3f, %8.3f)\n",
                slamval,
                newred[slamval], newgrn[slamval], newblu[slamval],
                knt,
                tr2[slamval], 
                tg2[slamval], 
                tb2[slamval], 
                trd[slamval], 
                tgn[slamval], 
                tbl[slamval], 
                sqrt((double)redvari),
                sqrt((double)grnvari),
                sqrt((double)bluvari));
#endif
    
    //
    //  If the variance is large, then use the standard
    //  deviation when reassigning the values.  The threshold for
    //  determining when to use standard deviation instead of
    //  variance is defined by these constants.
    //
    
#define RVTHRESH    4
#define GVTHRESH    4
#define BVTHRESH    4
        
        switch(placement[slamval]) {
        case 1:
        if (redvari > RVTHRESH || grnvari > GVTHRESH || bluvari > BVTHRESH) {
#ifdef VERBOSE
            fprintf(stderr, "Used Standard Deviation case 1\n");
#endif
                    
          newred[redo[i]] = (int)(trd[slamval]/knt + sqrt((double)redvari)*posneg);
          newgrn[redo[i]] = (int)(tgn[slamval]/knt + sqrt((double)grnvari)*posneg);
          newblu[redo[i]] = (int)(tbl[slamval]/knt + sqrt((double)bluvari)*posneg);
        } else {
#ifdef VERBOSE
            fprintf(stderr, "Used Variance case 1\n");
#endif
          newred[redo[i]] = (int)(trd[slamval]/knt + redvari*posneg);
          newgrn[redo[i]] = (int)(tgn[slamval]/knt + grnvari*posneg);
          newblu[redo[i]] = (int)(tbl[slamval]/knt + bluvari*posneg);
        }
        break;

        case 2:
        if (grnvari > GVTHRESH || bluvari > BVTHRESH) {
#ifdef VERBOSE
            fprintf(stderr, "Used Standard Deviation case 2\n");
#endif
          newred[redo[i]] = (int)(trd[slamval]/knt);
          newgrn[redo[i]] = (int)(tgn[slamval]/knt + sqrt((double)grnvari)*posneg);
          newblu[redo[i]] = (int)(tbl[slamval]/knt + sqrt((double)bluvari)*posneg);
        } else {
#ifdef VERBOSE
            fprintf(stderr, "Used Variance case 2\n");
#endif
          newred[redo[i]] = (int)(trd[slamval]/knt);
          newgrn[redo[i]] = (int)(tgn[slamval]/knt + grnvari*posneg);
          newblu[redo[i]] = (int)(tbl[slamval]/knt + bluvari*posneg);
        }
        break;

        case 3:
        if (redvari > RVTHRESH || grnvari > GVTHRESH) {
#ifdef VERBOSE
            fprintf(stderr, "Used Standard Deviation case 3\n");
#endif
          newred[redo[i]] = (int)(trd[slamval]/knt + sqrt((double)redvari)*posneg);
          newgrn[redo[i]] = (int)(tgn[slamval]/knt + sqrt((double)grnvari)*posneg);
          newblu[redo[i]] = (int)(tbl[slamval]/knt);
        } else {
#ifdef VERBOSE
            fprintf(stderr, "Used Variance case 3\n");
#endif
          newred[redo[i]] = (int)(trd[slamval]/knt + redvari*posneg);
          newgrn[redo[i]] = (int)(tgn[slamval]/knt + grnvari*posneg);
          newblu[redo[i]] = (int)(tbl[slamval]/knt);
        }
        break;

        case 4:
        if (redvari > RVTHRESH || bluvari > BVTHRESH) {
#ifdef VERBOSE
            fprintf(stderr, "Used Standard Deviation case 4\n");
#endif
          newred[redo[i]] = (int)(trd[slamval]/knt + sqrt((double)redvari)*posneg);
          newgrn[redo[i]] = (int)(tgn[slamval]/knt);
          newblu[redo[i]] = (int)(tbl[slamval]/knt + sqrt((double)bluvari)*posneg);
        } else {
#ifdef VERBOSE
            fprintf(stderr, "Used Variance case 4\n");
#endif
          newred[redo[i]] = (int)(trd[slamval]/knt + redvari*posneg);
          newgrn[redo[i]] = (int)(tgn[slamval]/knt);
          newblu[redo[i]] = (int)(tbl[slamval]/knt + bluvari*posneg);
        }
        break;

        
        default:
#ifdef VERBOSE
        fprintf(stderr, "Default Case\n");
#endif

          //
          //  It's more useful to set these to random values.
          //  Obviously, we're breaking apart a seriously one-sided colormap.
          //
#ifdef RGB
        newred[redo[i]] = (int)(drand48()*255.0);
        newgrn[redo[i]] = (int)(drand48()*255.0);
        newblu[redo[i]] = (int)(drand48()*255.0);
#else // YUV
        newred[redo[i]] = (int)(drand48()*219.0+16.0);
        newgrn[redo[i]] = (int)(drand48()*224.0+16.0);
        newblu[redo[i]] = (int)(drand48()*224.0+16.0);
#endif

        }
    
        //
        //  Clamp.
        //
#ifdef RGB
    if(newred[redo[i]] < 0) newred[redo[i]] = 0;
    if(newgrn[redo[i]] < 0) newgrn[redo[i]] = 0;
    if(newblu[redo[i]] < 0) newblu[redo[i]] = 0;
    
    if(newred[redo[i]] > 255) newred[redo[i]] = 255;
    if(newgrn[redo[i]] > 255) newgrn[redo[i]] = 255;
    if(newblu[redo[i]] > 255) newblu[redo[i]] = 255;
#else  // YUV
    if(newred[redo[i]] < 16) newred[redo[i]] = 16;
    if(newgrn[redo[i]] < 16) newgrn[redo[i]] = 16;
    if(newblu[redo[i]] < 16) newblu[redo[i]] = 16;
    
    if(newred[redo[i]] > 235) newred[redo[i]] = 235;
    if(newgrn[redo[i]] > 240) newgrn[redo[i]] = 240;
    if(newblu[redo[i]] > 240) newblu[redo[i]] = 240;
#endif
        
        //
        //  Set usage so the color will be used again and such that it
        //  has not chance of being turned off.
        //
        use[redo[i]] = 1;
    
#ifdef VERBOSE
        fprintf(stderr,
                "Set %3d (%3d, %3d, %3d) toward (%8.3f, %8.3f, %8.3f) to \
(%3d, %3d, %3d) aveerm %5ld\n", 
                redo[i], r,g,b,
                trd[slamval]/knt, tgn[slamval]/knt, tbl[slamval]/knt,
                newred[redo[i]], newgrn[redo[i]], newblu[redo[i]],
                (erm[slamval]/tkt[slamval]));
#endif
    
        //
        //  Re-adjust error and set direction. 
        //
        if (posneg == 1) {
          posneg = -1;
        } else {
          erm[slamval] = (long) (erm[slamval] * 0.5F); //  Cut error by half
          otkt[slamval] *= 0.66;                       //  Cut usage by 1/3rd
          errvshist = -errvshist;             //  Flip usage of error vs count
          posneg = 1;
        
          //
          //  If we've put four colors around a single color, do not
          //  put any more colors out there.
          //
          if (placement[slamval] && ++placement[slamval] > 4) {
            erm[slamval] = 0;
            otkt[slamval] = 0;
            placement[slamval] = 0;
          }
        }
    }

    //
    //  Check for reused/multiple entries.
    //
    //  Since we're slamming values around, I don't think this check
    //  is ver necessary any more.  My tests indicate that I was never
    //  turning off colors here.  It is still probably a good idea to
    //  leave this in.
    //
    for (i=0;i<colknt;i++) {
      if (use[i] == 2) {
        for (j=i;j<colknt;j++) {
          if ((i != j) && (use[j] > 0)) {
            int r,g,b;

            r = newred[i]-newred[j];
            g = newgrn[i]-newgrn[j];
            b = newblu[i]-newblu[j];
            if (r < 0) r = -r;
            if (g < 0) g = -g;
            if (b < 0) b = -b;

            if (r == 0 && g == 0 && b == 0) {
              use[j] = 0;
#ifdef VERBOSE
            turnoff++;
            
fprintf(stderr, "Turned off %d (%3d %3d %3d) because of %d\
 (%3d %3d %3d)\n",
                 j, newred[j], newgrn[j], newblu[j], i, newred[i], newgrn[i], newblu[i]);
#endif
            }
          }
        }
      }
    }


#ifdef VERBOSE
    //
    //  Finish and printout statistics
    //
    for (i=0; i<colknt; i++)
      if (use[i]) useknt++;

    fprintf(stderr,"\n");
    fprintf(stderr,"Threshold Low %d; Threshold High %d\n",threlo, threhi);
    fprintf(stderr,"Cells with -1 usage set %d\n",neg1knt);
    fprintf(stderr,"Cells with zero kount   %d\n",zeroknt);
    fprintf(stderr,"Cells under threshold   %d\n",subtknt);
    fprintf(stderr,"Cells under 4*threshold %d\n",sub4tknt);
    fprintf(stderr,"Cells over  threshold   %d\n",usedknt);
    fprintf(stderr,"Cells that were split       %d type 3 %d type 2 %d\n",
            resetknt,cod3knt,cod2knt);
    fprintf(stderr,"Maximum move  for threshold %8.3f %8.3f %8.3f\n",maxt,gott,actt);
    fprintf(stderr,"ErrMove:  %3d;  HistMove:  %3d\n", errknt, histknt);
    fprintf(stderr,"Cells turned off %d\n",turnoff);
    fprintf(stderr,"Colors in map %d\n",useknt);
    int k = kntfirst+kntquick+kntslow;
    fprintf(stderr,"first %d %5.2f quick %d %5.2f slow %d %5.2f \n",
            kntfirst,100.0*kntfirst/k,
            kntquick,100.0*kntquick/k,
            kntslow,100.0*kntslow/k);
    fprintf(stderr, "Error:   %8ld total   %8.3f ave\n",
            total_err, ((double)total_err)/((double)total_calls));
    fprintf(stderr,"\n");
    
    for(i=0; i<colknt; i++) {
        if(ored[i]==newred[i] && ogrn[i]==newgrn[i] && oblu[i]==newblu[i]) {
            if(tkt[i] != 0) {
                fprintf(stderr, "color %3d:(%3d %3d %3d) (%3d %3d %3d) (%3d %3d %3d) use%1d %5d error %8ld\n",
                        i, ored[i], ogrn[i], oblu[i],
                        newred[i], newgrn[i], newblu[i],
                        trd[i]/tkt[i], tgn[i]/tkt[i], tbl[i]/tkt[i],
                        use[i], tkt[i], olderm[i]);
            } else {
                fprintf(stderr, "color %3d:(%3d %3d %3d) (%3d %3d %3d) (--- --- ---) use%1d %5d error %8ld\n",
                        i, ored[i], ogrn[i], oblu[i],
                        newred[i], newgrn[i], newblu[i],
                        use[i], tkt[i], olderm[i]);
            }
        } else {
            if(tkt[i] != 0) {
                fprintf(stderr, "color %3d:(%3d %3d %3d)*(%3d %3d %3d) (%3d %3d %3d) use%1d %5d error %8ld\n",
                        i, ored[i], ogrn[i], oblu[i],
                        newred[i], newgrn[i], newblu[i],
                        trd[i]/tkt[i], tgn[i]/tkt[i], tbl[i]/tkt[i],
                        use[i], tkt[i], olderm[i]);
        } else {
            fprintf(stderr, "color %3d:(%3d %3d %3d)*(%3d %3d %3d) (--- --- ---) use%1d %5d error %8ld\n",
                i, ored[i], ogrn[i], oblu[i],
                newred[i], newgrn[i], newblu[i],
                use[i], tkt[i], olderm[i]);
        }
        }
    }
    fprintf(stderr,"\n");

    for(i=0; i<colknt; i++) {
        if(placement[i] != 1)
            fprintf(stderr, "Placement of %d is %d\n", i, placement[i]);
    }
    
    fprintf(stderr,"\n");

    //
    //  Compute average value in band,
    //  standard deviation in band,  and print histogram
    //
    int   sqradd = 0, sqgadd = 0, sqbadd = 0;
    int   radd = 0, gadd = 0, badd = 0;
    
    for(i = 0; i < colknt; i++) {
        radd    += newred[i];
        gadd    += newgrn[i];
        badd    += newblu[i];
        
#ifdef TODO
      // Replace after sqrs_table is in
        sqradd  += sqrs_table[newred[i]];
        sqgadd  += sqrs_table[newgrn[i]];
        sqbadd  += sqrs_table[newblu[i]];
#endif
        sqradd  += (newred[i] * newred[i]);
        sqgadd  += (newgrn[i] * newgrn[i]);
        sqbadd  += (newblu[i] * newblu[i]);
    }
    
    float averval, avegval, avebval;
    float sdrval, sdgval, sdbval;
    
    averval = radd/colknt;
    avegval = gadd/colknt;
    avebval = badd/colknt;

    sdrval  = sqrt((double)(colknt*sqradd - radd*radd)/(colknt*(colknt-1)));
    sdgval  = sqrt((double)(colknt*sqgadd - gadd*gadd)/(colknt*(colknt-1)));
    sdbval  = sqrt((double)(colknt*sqbadd - badd*badd)/(colknt*(colknt-1)));
    
    fprintf(stderr, "Average (%8.3f, %8.3f, %8.3f)\n", averval, avegval, avebval);
    fprintf(stderr, "Std Dev (%8.3f, %8.3f, %8.3f)\n", sdrval, sdgval, sdbval);
    
    fprintf(stderr,"\f\n");
#endif

    //
    //  Adjust Threshold
    //
    optdist = ((double)total_calls/colknt);
    threlo  = (int)optdist/10;
    threhi  = (int)optdist/2;

#ifdef VERBOSE
    fprintf(stderr, "total_calls = %d\n", total_calls);
    fprintf(stderr, "optdist      = %8.3f\n", optdist);
#endif

    //
    //  Copy the new colormap into the given entry.
    //
    for (i=0;i<colknt;i++) {
      cmap[i].band0() = newred[i];
      cmap[i].band1() = newgrn[i];
      cmap[i].band2() = newblu[i];
      if(use[i])
        cmap.used(i) = 1;
      else
        cmap.used(i) = 0;
    }
    cmap.setNumEntries(colknt);

    return;
}
