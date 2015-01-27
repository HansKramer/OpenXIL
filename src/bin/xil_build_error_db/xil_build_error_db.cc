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
//  File:	xil_build_error_db.cc
//  Project:	XIL
//  Revision:	1.8
//  Last Mod:	10:16:37, 03/10/00
//
//  Description:
//	
//	Takes the xil.po file and builds an XIL error database.
//	
//	
//	
//	
//	
//	
//------------------------------------------------------------------------
//	COPYRIGHT
//------------------------------------------------------------------------
#pragma ident	"@(#)xil_build_error_db.cc	1.8\t00/03/10  "

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NUM_ERRORS  1024
#define MAX_ERR_SIZE    8192
#define DB_VERSION      1

struct XiliErrorDBHeader {
    unsigned int version;
    unsigned int num_entries;

    unsigned int extra_data[32];
};

int
main(int argc, char* argv[]) {
    //
    //  Verify program arguments.
    //
    if(argc < 2) {
        fprintf(stderr, "usage: %s infile outfile [maxerrors]\n", argv[0]);
        exit(1);
    }

    FILE* infile = fopen(argv[1], "rb");
    if(infile == NULL) {
        fprintf(stderr, "ERROR:  Could not open infile %s\n", argv[1]);
        exit(1);
    }

    FILE* outfile = fopen(argv[2], "wb");
    if(outfile == NULL) {
        fprintf(stderr, "ERROR:  Could not open outfile %s\n", argv[2]);
        exit(1);
    }

    FILE* tmpfile = fopen(tempnam("/tmp", "xildb"), "wb+");
    if(tmpfile == NULL) {
        fprintf(stderr, "ERROR:  Could not open tmpfile\n");
        exit(1);
    }

    unsigned int max_errors = MAX_NUM_ERRORS;
    if(argc > 3) {
        max_errors = atoi(argv[3]);
    }

    int*  map_array = new int[max_errors];
    if(map_array == NULL) {
        fprintf(stderr,
                "ERROR:  new failed to allocate array of %d ints\n", max_errors);
        exit(1);
    }

    unsigned int prev_id = 0;
    unsigned int offset  = 0;

    char  buffer[MAX_ERR_SIZE];
    while(fgets(buffer, MAX_ERR_SIZE, infile)) {
        //
        //  Check to see if the first characters indicate it's a msgid.
        //
        if(!strncmp(buffer, "msgid", 5)) {
            //
            //  Pull off the first token.
            //
            char* tok = strtok(buffer, "\"");
            if(tok != NULL) {
                tok = strtok(NULL, "-");
                if(tok == NULL) {
                    fprintf(stderr, "ERROR:  no hyphen in id\n");
                    continue;
                }

                if(strncmp(tok, "di", 2)) {
                    continue;
                }

                tok = strtok(NULL, "\"");
                if(tok == NULL) {
                    fprintf(stderr, "ERROR:  malformed id -- missing end quote\n");
                    continue;
                }
                
                unsigned int id_num = atoi(tok);
                if(id_num == 0) {
                    fprintf(stderr, "ERROR:  id '%s' is malformed number\n", tok);
                    continue;
                }

                if(id_num != (prev_id+1)) {
                    fprintf(stderr, "WARNING:  gap from id %d to %d is missing\n",
                            prev_id+1, id_num-1);
                }

                for(unsigned int i = prev_id+1; i <= id_num-1; i++) {
                    map_array[i] = -1;
                }

                prev_id = id_num;

//                printf("processing id# %3d\n", id_num);

                if(fgets(buffer, MAX_ERR_SIZE, infile) == NULL ||
                   strncmp(buffer, "msgstr", 6)) {
                    fprintf(stderr, "ERROR:  msgid doesn't have msgstr\n");
                    exit(1);
                }

                if(strtok(buffer, "\"") != NULL) {
                    char* str = strtok(NULL, "\"");
                    if(str == NULL) {
                        fprintf(stderr,
                                "ERROR:  malformed msgstr -- missing end quote\n");
                        continue;
                    }

//                    printf("id %d @ %d:  %s\n", id_num, offset, str);

                    map_array[id_num] = offset;

                    offset += strlen(str) + 1;

                    fprintf(tmpfile, "%s", str);

                    char zero = '\0';
                    fwrite(&zero, 1, 1, tmpfile);
                } else {
                    fprintf(stderr, "ERROR:  msgid has malformed msgstr\n");
                    continue;
                }
            } else {
                fprintf(stderr, "ERROR:  message id is missing first quote\n");
            }
        }
    }

    struct XiliErrorDBHeader header;

    header.version     = DB_VERSION;
    header.num_entries = prev_id;

    if(fwrite(&header, sizeof(XiliErrorDBHeader), 1, outfile) == 0) {
        perror("writing header to outfile");
        exit(1);
    }

//    printf("adjusting offset by %d bytes\n",
//           sizeof(header) + header.num_entries*sizeof(unsigned int));

//    printf("sizeof(XiliErrorDBHeader) = %d\n", sizeof(XiliErrorDBHeader));
    for(unsigned int i=0; i<=header.num_entries; i++) {
        if(map_array[i] != -1) {
            map_array[i] += sizeof(header);
            map_array[i] += (header.num_entries*sizeof(unsigned int));
        }
//        printf("map_array[%d] = %d\n", i, map_array[i]);
    }

    if(fwrite(&map_array[1], sizeof(unsigned int), header.num_entries, outfile) == 0) {
        perror("writing map_array to outfile");
        exit(1);
    }

    if(fseek(tmpfile, 0L, SEEK_SET) < 0) {
        perror("fseek on tmpfile failed");
        exit(1);
    }

    unsigned int cnt;
    while(cnt = fread(buffer, 1, 8192, tmpfile)) {
        if(fwrite(buffer, 1, cnt, outfile) == 0) {
            perror("copying tmpfile to outfile");
            exit(1);
        }
    }

    fflush(stderr);

    printf("%d error entries done.\n", header.num_entries);

    fclose(infile);
    fclose(outfile);
    fclose(tmpfile);

    return 0;
}
