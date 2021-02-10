//
//  xref.c
//  darwinbuild
//
//  Created by John Othwolo on 1/30/21.
//  Copyright © 2021 The DarwinBuild Project. All rights reserved.
//

#include <cstdio>
#include <string>
#include <filesystem>

#include <dlfcn.h>
#include <CoreFoundation/CoreFoundation.h>

#include "DBPlugin.h"
#include "DBPluginPriv.h"
#include "darwinbuild.h"
#include "xref.h"

namespace fs = std::filesystem;

int Xref(std::string build){
    int ret = 0;
    std::string dbfile;
    std::string plugins;
    char *c_dbfile = getenv("DARWINXREF_DB_FILE");
    char *c_plugins = getenv("DARWINXREF_PLUGIN_PATH");
    char *argv[] = {(char*)("loadIndex"), (char*)std::string(".build/" + build + ".plist").c_str()};
    
    if (c_dbfile != NULL)  dbfile = c_dbfile;
    else  dbfile = DEFAULT_DB_FILE;
    if (c_plugins != NULL) plugins = c_plugins;
    else  plugins = DEFAULT_PLUGIN_PATH;
    
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
    
    if (DBPluginLoadPlugins(plugins.c_str()) == -1) {
            fprintf(stderr, "Error: cannot load plugins!\n");
        exit(2);
    }
    
    if (run_plugin(2, argv) == -1) {
        print_usage(progname, 3, argv);
        exit(1);
    }
    
    return 0;
}
