//
//  xref.c
//  darwinbuild
//
//  Created by John Othwolo on 1/30/21.
//  Copyright © 2021 The DarwinBuild Project. All rights reserved.
//

#include <cstdio>
#include <dlfcn.h>
#include <string>
#include <CoreFoundation/CoreFoundation.h>

#include "DBPlugin.h"
#include "DBPluginPriv.h"
#include "darwinbuild.h"

int Xref(){
    std::string dbfile = std::string(getenv("DARWINXREF_DB_FILE"));
    std::string build = std::string(getenv("DARWINBUILD_BUILD"));
    std::string plugins = std::string(getenv("DARWINXREF_PLUGIN_PATH"));
    int ret = 0;
    
    if (dbfile.empty()) dbfile = std::string(DEFAULT_DB_FILE);
    if (plugins.empty()) plugins = DEFAULT_PLUGIN_PATH;
    
    if (build.empty())
        ret = readFile(".build/build", (char*)build.c_str());
    if (build.empty() || ret != 0){
        CFDictionaryRef (*CFCopySystemVersionDictionary)(void) = (CFDictionaryRef (*)())dlsym(RTLD_DEFAULT, "_CFCopySystemVersionDictionary");
        if (CFCopySystemVersionDictionary) {
            CFDictionaryRef dict = CFCopySystemVersionDictionary();
            if (dict != NULL) {
                CFStringRef str = (CFStringRef)CFDictionaryGetValue(dict, CFSTR("ProductBuildVersion"));
                build = strdup_cfstr(str);
                CFRelease(dict);
            }
        }
    }
    
    DBDataStoreInitialize(dbfile.c_str());
    DBSetCurrentBuild((char*)build.c_str());
    
    return 0;
}
