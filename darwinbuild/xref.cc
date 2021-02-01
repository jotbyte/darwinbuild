//
//  xref.c
//  darwinbuild
//
//  Created by John Othwolo on 1/30/21.
//  Copyright © 2021 The DarwinBuild Project. All rights reserved.
//

#include <stdio.h>
#include <dlfcn.h>
#include <CoreFoundation/CoreFoundation.h>

#include "DBPlugin.h"
#include "DBPluginPriv.h"
#include "darwinbuild.h"

int Xref(){
    char* dbfile = getenv("DARWINXREF_DB_FILE");
    char* build = getenv("DARWINBUILD_BUILD");
    const char* plugins = getenv("DARWINXREF_PLUGIN_PATH");
    int ret = 0;
    
    if (dbfile == NULL) dbfile = DEFAULT_DB_FILE;
    if (plugins == NULL) plugins = DEFAULT_PLUGIN_PATH;
    
    if (build == NULL)
        ret = readFile(".build/build", build);
    if (build == NULL || ret != 0){
        CFDictionaryRef (*CFCopySystemVersionDictionary)(void) = dlsym(RTLD_DEFAULT, "_CFCopySystemVersionDictionary");
        if (CFCopySystemVersionDictionary) {
            CFDictionaryRef dict = CFCopySystemVersionDictionary();
            if (dict != NULL) {
                CFStringRef str = CFDictionaryGetValue(dict, CFSTR("ProductBuildVersion"));
                build = strdup_cfstr(str);
                CFRelease(dict);
            }
        }
    }
    
    DBDataStoreInitialize(dbfile);
    DBSetCurrentBuild(build);
    
    return 0;
}
