//
//  stdlib.c
//  darwintrace
//
//  Created by John Othwolo on 1/30/21.
//  Copyright © 2021 The DarwinBuild Project. All rights reserved.
//

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "darwinbuild.h"

int checkDir(const char *dir){
    int ret = 0;
    struct stat dstat;

    if (stat(dir, &dstat) != 0){
        fprintf(stderr, "unable to open directory \'%s\' (%s)\n", dir, strerror(errno));
    } else {
        if (S_ISDIR(dstat.st_mode) == 0){
            fprintf(stderr, "bad directory \'%s\' (%s)\n", dir, strerror(errno));
            return ENOTDIR;
        }
    }
    
    return ret;
}

int checkFile(const char *file){
    int ret = 0;
    struct stat fstat;

    if (stat(file, &fstat) != 0){
        fprintf(stderr, "unable to open file \'%s\' (%s)\n", file, strerror(errno));
        ret = errno;
    } else {
        if (S_ISREG(fstat.st_mode) == 0){
            fprintf(stderr, "The file '%s' is not supported\n", file);
            ret = ENOENT;
        }
    }
    return ret;
}

int readFile(const char *path, char *out){
    FILE *fp = NULL;
    char *data = NULL;
    int ret, sz = 1000;
    size_t len;
    
    if((ret = checkFile(path)) != 0)
        return ret;
    
    fp = fopen(path, O_RDONLY);
    data = (char*)malloc(sz);
    len = fread(data, sz, 1, fp);
    
    if ((ret = ferror(fp))) {
        goto bad_out;
    }
    data[len] = 0;
    if (data[len-1] == '\n')
        data[len-1] = 0;
    
    out = data;
    return 0;
    
bad_out:
    free(data);
    out = NULL;
    return ret;
}
