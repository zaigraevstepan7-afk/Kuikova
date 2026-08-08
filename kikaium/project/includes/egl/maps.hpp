#pragma once

#include <unistd.h>

#define PROCMAPS_LINE_MAX_LENGTH  (PATH_MAX + 100)

typedef struct procmaps_struct{
    uintptr_t addr_start;   //< start address of the area
    uintptr_t addr_end;   //< end address
    intptr_t length; //< size of the range

    char perm[5];    //< permissions rwxp
    bool is_r;      //< rewrote of perm with short flags
    bool is_w;
    bool is_x;
    bool is_p;

    long offset;  //< offset
    char dev[12];  //< dev major:minor
    int inode;    //< inode of the file that backs the area

    char pathname[600];    //< the path of the file that backs the area

    struct procmaps_struct* next;
} procmaps_struct;

typedef struct procmaps_iterator{
    procmaps_struct* head;
    procmaps_struct* current;
} procmaps_iterator;

procmaps_iterator* pmparser_parse(int pid);
procmaps_struct* pmparser_next(procmaps_iterator* p_procmaps_it);
void pmparser_free(procmaps_iterator* p_procmaps_it);
void _pmparser_split_line(const char*buf,char*addr1,char*addr2,char*perm, char* offset, char* device,char*inode,char* pathname);