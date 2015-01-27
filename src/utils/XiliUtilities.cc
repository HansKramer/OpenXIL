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
//  File:	XiliUtilities.cc
//  Project:	XIL
//  Revision:	1.36
//  Last Mod:	10:16:30, 03/10/00
//
//  Description:
//	Assorted utility routines.
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)XiliUtilities.cc	1.36\t00/03/10  "


//
// System includes

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#if !defined(_WINDOWS) && !defined(IRIX) && !defined(HPUX)
#include <libintl.h>
#endif

//
// XIL includes

#include "XiliUtils.hh"
#include "_XilGPIDefines.hh"
#include "XiliThread.hh"

//
//  This is a symbol that is needed to satisfy a libC dependency.  It's
//  something that shouldn't be called because we don't use streams, but we
//  need the symbol there just the same.
//
//  TODO: 5/8/96 jlf  Can we remove this?
//
void
_stream_abort(int) {
}

//
//  The xili_byte_to_float table.
//
#ifdef _XIL_USE_TABLE_FLT_CNV
float  xili_byte_to_float[256] = {
        0.0,   1.0,   2.0,   3.0,   4.0,   5.0,   6.0,   7.0,   8.0,   9.0,
       10.0,  11.0,  12.0,  13.0,  14.0,  15.0,  16.0,  17.0,  18.0,  19.0,
       20.0,  21.0,  22.0,  23.0,  24.0,  25.0,  26.0,  27.0,  28.0,  29.0,
       30.0,  31.0,  32.0,  33.0,  34.0,  35.0,  36.0,  37.0,  38.0,  39.0,
       40.0,  41.0,  42.0,  43.0,  44.0,  45.0,  46.0,  47.0,  48.0,  49.0,
       50.0,  51.0,  52.0,  53.0,  54.0,  55.0,  56.0,  57.0,  58.0,  59.0,
       60.0,  61.0,  62.0,  63.0,  64.0,  65.0,  66.0,  67.0,  68.0,  69.0,
       70.0,  71.0,  72.0,  73.0,  74.0,  75.0,  76.0,  77.0,  78.0,  79.0,
       80.0,  81.0,  82.0,  83.0,  84.0,  85.0,  86.0,  87.0,  88.0,  89.0,
       90.0,  91.0,  92.0,  93.0,  94.0,  95.0,  96.0,  97.0,  98.0,  99.0,
      100.0, 101.0, 102.0, 103.0, 104.0, 105.0, 106.0, 107.0, 108.0, 109.0,
      110.0, 111.0, 112.0, 113.0, 114.0, 115.0, 116.0, 117.0, 118.0, 119.0,
      120.0, 121.0, 122.0, 123.0, 124.0, 125.0, 126.0, 127.0, 128.0, 129.0,
      130.0, 131.0, 132.0, 133.0, 134.0, 135.0, 136.0, 137.0, 138.0, 139.0,
      140.0, 141.0, 142.0, 143.0, 144.0, 145.0, 146.0, 147.0, 148.0, 149.0,
      150.0, 151.0, 152.0, 153.0, 154.0, 155.0, 156.0, 157.0, 158.0, 159.0,
      160.0, 161.0, 162.0, 163.0, 164.0, 165.0, 166.0, 167.0, 168.0, 169.0,
      170.0, 171.0, 172.0, 173.0, 174.0, 175.0, 176.0, 177.0, 178.0, 179.0,
      180.0, 181.0, 182.0, 183.0, 184.0, 185.0, 186.0, 187.0, 188.0, 189.0,
      190.0, 191.0, 192.0, 193.0, 194.0, 195.0, 196.0, 197.0, 198.0, 199.0,
      200.0, 201.0, 202.0, 203.0, 204.0, 205.0, 206.0, 207.0, 208.0, 209.0,
      210.0, 211.0, 212.0, 213.0, 214.0, 215.0, 216.0, 217.0, 218.0, 219.0,
      220.0, 221.0, 222.0, 223.0, 224.0, 225.0, 226.0, 227.0, 228.0, 229.0,
      230.0, 231.0, 232.0, 233.0, 234.0, 235.0, 236.0, 237.0, 238.0, 239.0,
      240.0, 241.0, 242.0, 243.0, 244.0, 245.0, 246.0, 247.0, 248.0, 249.0,
      250.0, 251.0, 252.0, 253.0, 254.0, 255.0
};
#else
float*  xili_byte_to_float;
#endif // _XIL_USE_TABLE_FLT_CNV

//
// Define a table to produce the square of a byte value
//
const unsigned int 
_XILI_SQR[256] = {
        0,     1,     4,     9,    16,    25,    36,    49,    64,    81,
      100,   121,   144,   169,   196,   225,   256,   289,   324,   361,
      400,   441,   484,   529,   576,   625,   676,   729,   784,   841,
      900,   961,  1024,  1089,  1156,  1225,  1296,  1369,  1444,  1521,
     1600,  1681,  1764,  1849,  1936,  2025,  2116,  2209,  2304,  2401,
     2500,  2601,  2704,  2809,  2916,  3025,  3136,  3249,  3364,  3481,
     3600,  3721,  3844,  3969,  4096,  4225,  4356,  4489,  4624,  4761,
     4900,  5041,  5184,  5329,  5476,  5625,  5776,  5929,  6084,  6241,
     6400,  6561,  6724,  6889,  7056,  7225,  7396,  7569,  7744,  7921,
     8100,  8281,  8464,  8649,  8836,  9025,  9216,  9409,  9604,  9801,
    10000, 10201, 10404, 10609, 10816, 11025, 11236, 11449, 11664, 11881,
    12100, 12321, 12544, 12769, 12996, 13225, 13456, 13689, 13924, 14161,
    14400, 14641, 14884, 15129, 15376, 15625, 15876, 16129, 16384, 16641,
    16900, 17161, 17424, 17689, 17956, 18225, 18496, 18769, 19044, 19321,
    19600, 19881, 20164, 20449, 20736, 21025, 21316, 21609, 21904, 22201,
    22500, 22801, 23104, 23409, 23716, 24025, 24336, 24649, 24964, 25281,
    25600, 25921, 26244, 26569, 26896, 27225, 27556, 27889, 28224, 28561,
    28900, 29241, 29584, 29929, 30276, 30625, 30976, 31329, 31684, 32041,
    32400, 32761, 33124, 33489, 33856, 34225, 34596, 34969, 35344, 35721,
    36100, 36481, 36864, 37249, 37636, 38025, 38416, 38809, 39204, 39601,
    40000, 40401, 40804, 41209, 41616, 42025, 42436, 42849, 43264, 43681,
    44100, 44521, 44944, 45369, 45796, 46225, 46656, 47089, 47524, 47961,
    48400, 48841, 49284, 49729, 50176, 50625, 51076, 51529, 51984, 52441,
    52900, 53361, 53824, 54289, 54756, 55225, 55696, 56169, 56644, 57121,
    57600, 58081, 58564, 59049, 59536, 60025, 60516, 61009, 61504, 62001,
    62500, 63001, 63504, 64009, 64516, 65025 
};


void
xili_cspace_name_to_opcode(const char*          name,
                           XilColorspaceOpCode* op_code,
                           unsigned int*        num_bands)
{
    if(strcmp(name, "rgb709") == 0) {
        *op_code   = XIL_CS_RGB709;
        *num_bands = 3;
    } else if(strcmp(name, "rgblinear") == 0) {
        *op_code   = XIL_CS_RGBLINEAR;
        *num_bands = 3;
    } else if(strcmp(name, "ycc709") == 0) {
        *op_code   = XIL_CS_YCC709;
        *num_bands = 3;
    } else if(strcmp(name, "y709") == 0) {
        *op_code   = XIL_CS_Y709;
        *num_bands = 1;
    } else if(strcmp(name, "ylinear") == 0) {
        *op_code   = XIL_CS_YLINEAR;
        *num_bands = 1;
    } else if(strcmp(name, "photoycc") == 0) {
        *op_code   = XIL_CS_PHOTOYCC;
        *num_bands = 3;
    } else if(strcmp(name, "ycc601") == 0) {
        *op_code   = XIL_CS_YCC601;
        *num_bands = 3;
    } else if(strcmp(name, "y601") == 0) {
        *op_code   = XIL_CS_Y601;
        *num_bands = 1;
    } else if(strcmp(name, "cmy") == 0) {
        *op_code   = XIL_CS_CMY;
        *num_bands = 3;
    } else if(strcmp(name, "cmyk") == 0) {
        *op_code   = XIL_CS_CMYK;
        *num_bands = 4;
    } else {
        *op_code   = XIL_CS_INVALID;
        *num_bands = 0;
    }
}

const char *xili_strerror(int Errno)
{
#ifdef _WINDOWS
    static char errstr[1024];

    sprintf(errstr, "%d", Errno);  // at least Errno, if the func. below fails
    errstr[0] = '\0';
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 
                        NULL,
                        Errno,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        errstr,
                        1024,
                        NULL);

    if(errstr[0]) {
        if(errstr[strlen(errstr)-1] == '\n') {
            errstr[strlen(errstr)-1] = '\0';
        }
        if(errstr[strlen(errstr)-1] == '\r') {
            errstr[strlen(errstr)-1] = '\0';
        }
    }
    else
        sprintf(errstr, "<system errno : %d>", Errno);  // at least Errno
    
#else
    char *errstr;

    if (!(errstr = strerror(Errno)))
        errstr = "<NULL>";

#endif
    return errstr;
}

char *xili_dgettext(const char *str1, const char *str2)
{
#ifdef SOLARIS
    return(dgettext(str1, str2));
#else
    return ((char *)str2);
#endif
}

long xili_sysconf(int sc_num)
{
    long sc_val;
#ifdef _WINDOWS
    SYSTEM_INFO sysinfo;

    GetSystemInfo(&sysinfo);

    switch (sc_num)
    {
        case _SC_PAGESIZE : {
            sc_val = sysinfo.dwPageSize;
            break;
        }
        case _SC_PHYS_PAGES: {
            MEMORYSTATUS memstatus;

            memset(&memstatus, '\0', sizeof(memstatus));
            memstatus.dwLength = sizeof(MEMORYSTATUS);
            GlobalMemoryStatus(&memstatus);
            sc_val = memstatus.dwTotalPhys/sysinfo.dwPageSize;
            break;
        }
        case _SC_NPROCESSORS_ONLN: {
            sc_val = sysinfo.dwNumberOfProcessors;
            break;
        }
        default : {
            sc_val=-1;
            break;
        }
    }
#elif LINUX
    switch(sc_num) {
        size_t page_size;

        case _SC_PAGESIZE:
            page_size = getpagesize();
            sc_val = (long)page_size;
            break;
        case _SC_NPROCESSORS_ONLN:
            //
            // Assume uniprocessor.
            //
            sc_val = 1;
            break;
        case _SC_PHYS_PAGES:
            page_size = getpagesize();
            //
            // Calculate bogus value equivalent to 32 Mb of RAM.
            //
            sc_val = (32 * 1024 * 1024) / page_size;
            break;
        default:
            sc_val = sysconf(sc_num);
    }
#else
    sc_val = sysconf(sc_num);
#endif
    return (sc_val);
}

XilThreadId xili_thread_self()
{
#if defined(_XIL_USE_PTHREADS)
    return pthread_self();
#elif defined(_XIL_USE_SOLTHREADS)
    return thr_self();
#elif defined(_XIL_USE_WINTHREADS)
    //
    // thrd identifier instead of thrd handle, check with John Furlani
    //
    return ((XilThreadId) GetCurrentThreadId());
#else
    return 0;
#endif
}

#ifndef DEBUG
void xili_print_debug(char* , ...)
#else
void xili_print_debug(char* format, ...)
#endif
{
#ifndef DEBUG
    return;
#else
    va_list ap;
    char emsg[1024];
    static FILE* fp = NULL;

    if(!fp) {
        char* envptr = getenv("XIL_LOG");

        if(!envptr)
            return;

        if(!strcmp(envptr, "stdout")) {
            fp = stdout;
        } else if(!strcmp(envptr, "stderr")) {
            fp = stderr;
        }
        else {
            fp = fopen(envptr, "a+");
        }

        if(!fp)
            return;
    }

    va_start(ap, format);
    vsprintf(emsg, format, ap);
    va_end(ap);

    fprintf(fp, "--t%05d-- %s\n", xili_thread_self(), emsg);
    fflush(fp);
#endif // DEBUG
}

#ifdef _WINDOWS

#define REG_APP_INSTALL_PATH_KEY  \
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\XIL"

#define REG_APP_INSTALL_PATH_VALUE          "Path"
#define REG_APP_INSTALL_PATH_DEFAULT_VALUE  "."

char *xili_get_install_path()
{

    HKEY        hKey;
    long        nStatus;
    DWORD       dwSize = XILI_PATH_MAX;
    static char installPath[XILI_PATH_MAX];

    if(installPath[0]) {
        return installPath;
    }

    //
    // Open the key
    //
    nStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                           REG_APP_INSTALL_PATH_KEY,
                           0,
                           KEY_QUERY_VALUE, 
                           &hKey);

    if(nStatus != ERROR_SUCCESS) {
        return NULL;
    }


    //
    // Query the values of "Path" data
    //
    nStatus = RegQueryValueEx(hKey,
                           REG_APP_INSTALL_PATH_VALUE,
                           NULL,
                           NULL,
                           (LPBYTE) installPath,
                           &dwSize);

    //
    // If "Path" value is not set then look for the "Default"
    //
    if(nStatus != ERROR_SUCCESS) {
        nStatus = RegQueryValueEx(hKey,
                                  REG_APP_INSTALL_PATH_DEFAULT_VALUE,
                                  NULL,
                                  NULL,
                                  (LPBYTE) installPath,
                                  &dwSize);
    }

    RegCloseKey(hKey);

    if((nStatus != ERROR_SUCCESS) || !installPath[0]) {
        return NULL;
    } else {
        return installPath;
    }
}
#endif
