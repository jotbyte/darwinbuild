//
//  DarwinbuildInit.cpp
//  darwintrace
//
//  Created by John Othwolo on 2/2/21.
//  Copyright © 2021 The DarwinBuild Project. All rights reserved.
//

#include <cstdio>
#include <cstdlib>
#include <cstdbool>
#include <iostream>
#include <vector>
#include <filesystem>

#include <libgen.h>
#include <getopt.h>
#include <sys/stat.h>
#include <curl/curl.h>

#include "darwinbuild.h"
#include "xref.h"
#include "init.h"

typedef std::string String;
namespace fs = std::filesystem;

void DarwinbuildInit(init_options_t &opts){
    time_t now;
    String build = opts.build;
    bool dmg = opts.dmg;
    
    // If the arguments provided weren't for init, continue to next getopt
    if (build.empty()) {
        exit(EINVAL);
    }
    // Otherwise initialize the build and exit.
    
    String filename = String(basename((char*)build.c_str()));
    String pdBuild = filename.substr(0, filename.length() + 1 - sizeof(".plist"));
    String buildstr = filename.substr(0, filename.length() - strlen(".plist"));
        
    if (checkDir("Roots", true))
        newDir("Roots");
    if (checkDir("Sources", true))
        newDir("Sources");
    if (checkDir("Symbols", true))
        newDir("Symbols");
    if (checkDir("Headers", true))
        newDir("Headers");
    if (checkDir("Logs", true))
        newDir("Logs");
    if (checkDir(".build", true))
        newDir(".build");
    
    if(checkfile("./build/" + filename, true)){
        copyfile(build, String(".build/") + filename , false);
    }
    else if (isURL(build)){
        DownloadFile(build, String(".build/") + filename);
    }
    else if (isSSH(build)){
        std::cout << "SSH files not supported yet" << std::endl;
        exit(0);
    }
    time(&now);
    // TODO: Fnish init functionality
    if (dmg) { // create DMG
        String dmgfile(".build/buildroot.sparsebundle");
        if(checkfile(dmgfile, true)){
            String volname = "BuilRoot_" + buildstr + String("-") + std::to_string(now/3600);
            char temp[PATH_MAX];
            std::string buildroot;
            int status = 0;
            char args[1024] = {0};
            
            buildroot = (getcwd(&temp[0], PATH_MAX) + String("/") + dmgfile);
            snprintf(&args[0], 1024, "hdiutil create -size 1TB -fs HFSX -type SPARSEBUNDLE -quiet -uid %s -gid %s -volname \"%s\" %s",
                    std::to_string(getuid()).c_str(),
                    std::to_string(getgid()).c_str(),
                    volname.c_str(),
                     buildroot.c_str());
            if (checkfile(buildroot, true) == 0) {
                std::cout << "buildroot already exists" << std::endl;
                exit(0);
            }
            if ((status = system(args)) != 0) {
                perror("Error");
                exit(-1);
            }
            // create symlink
            symlink("/Volumes/" + volname, "./BuildRoot");
        }
    } else { // -nodmg
        String path = String(fs::current_path()) + "/BuildRoot";
        struct stat buf;
        if(lstat(path.c_str(), &buf) == 0){
            remove(path.c_str());
        }
        newDir(path);
    }
    
//    char *dwbroot = getenv("DARWIN_BUILDROOT");
//    if (dwbroot != NULL) {
//        if (dwbroot[0] != '/') {
//            fprintf(stderr, "DARWIN_BUILDROOT must be an absolute path");
//        } else {
//            checkDir(String(dwbroot), false);
//            chdir(dwbroot);
//        }
//    }
    
    Xref(buildstr);
    
    std::cout << "Initialization Complete" << std::endl;
    exit(0);
}
