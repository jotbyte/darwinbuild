/* Copyright © 2021 The PureDarwin Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the organization nor the names of its contributors may
 *     be used to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * darwinbuild.c
 ***
 * Created by John Othwolo on 1/29/21.
 */

/*
 * This a replacement tool for the old Darwinbuild script.
 */

#include <cstdlib>
#include <cstdbool>
#include <iostream>

#include <libgen.h>
#include <getopt.h>
#include <sys/stat.h>
#include <curl/curl.h>

#include "darwinbuild.h"

#define nl std::endl
typedef std::string String;

String progname = String("(null)");

enum {
    ACTION_HEADERS = 0x1,
    ACTION_FETCH,
    ACTION_SOURCE,
    ACTION_LOAD,
    ACTION_LOADONLY,
    ACTION_GROUP,
};
typedef int actions_t;

typedef struct options {
    // bool args
    bool nosource;
    bool nopatch;
    bool noload;
    // string args
//    char configuration[255];
    String build;
    String logdeps;
    String depsbuild;
    String codesign;
    String target;
} options_t;

static void PrintInitHelp(){
    std::cout <<  "usage: " << progname << " -init <build> [-nodmg ]" << nl;
    std::cout <<  nl;
    std::cout <<  "  <build>       can be a standard build number or a path to a plist." << nl;
    std::cout <<  "                supported user@host:/dir/file.plist," << nl;
    std::cout <<  "                               http://host/dir/file.plist " << nl;
    std::cout <<  "                              paths: /dir/file.plist, " << nl;
    std::cout <<  "  -nodmg        do not use a sparse image for build root (use a regular directory)" << nl;
    exit(1);
}

static void DarwinbuildInit(int argc, char *argv[]){
    int ch;
    char *build = NULL;
    bool nodmg = false;
    char **margv = argv; // preserve argv
    struct option arg_options[] = {
        { "init",           required_argument,  0, 0x30 },
        { "nodmg",          no_argument,        0, 0x40 },
        { NULL, 0, 0, 0 }
    };
    
    opterr = 0; // ignore later getopt options if we're not initalizing anything.
    // it'll just skip over this loop like this function never happened.
    while ((ch = getopt_long_only(argc, margv, "", arg_options, NULL)) != (-1)) {
        switch (ch) {
            case 0x30:
                build = optarg;
                if (!strncmp("help", build, sizeof("help"))) {
                    PrintInitHelp();
                }
                break;
            case 0x40:
                if(build == NULL) PrintInitHelp(); // -nodmg shouldn't be used alone
                nodmg = true;
                break;
            default:
                if(build){ // if there's some unexpected argument, abort.
                    PrintInitHelp();
                }
                break;
        }
    }
    opterr = 1; // reset errors for later getopt
    
    margv += optind;
    // This is unecessary.
    if (margv[0] != NULL) {
        std::cout << "Unexpected argument " << margv[0] << nl;
        PrintInitHelp();
    }
    // If the arguments provided weren't for init, continue to next getopt
    if (build == NULL) {
        return;
    }
    // Otherwise initialize the build and exit.
    
    String filename = String(basename(build));
    String pdBuild = filename.substr(0, filename.length() + 1 - sizeof(".plist"));
        
    if (checkDir("Roots")) {
        newDir("Roots");
    }
    if (checkDir("Sources")) {
        newDir("Sources");
    }
    if (checkDir("Symbols")) {
        newDir("Symbols");
    }
    if (checkDir("Headers")) {
        newDir("Headers");
    }
    if (checkDir("Logs")) {
        newDir("Logs");
    }
    if (checkDir(".build")) {
        newDir(".build");
    }
    
    
    if(checkFile(filename) == 0){
        copyfile(filename, String(".build/") + filename , false);
    }
    else if (isURL(build)){
        DownloadFile(build, String(".build/") + filename);
    }
    else if (isSSH(build)){
        std::cout << "SSH files not supported yet" << nl;
        exit(0);
    }
    
    // TODO: Fnish init functionality
    if (nodmg) {
        newDir("BuildRoot");
    } else { // create DMG
        std::string dmgfile = String("BuildRoot_") + pdBuild;
        
    }
    
    std::cout << "Initializing darwinbuild" << nl;
    exit(0);
}

static void PrintUsage(){
    std::cout << "Darwinbuild 1.0" << nl; // TODO: Work on versioning
    std::cout << "Copyright © 2021 The PureDarwin Project." << nl;
    std::cout << "This is free software; see the source for copying conditions." << nl;
    std::cout << nl;
    std::cout << "usage: " << progname << " -init <build> [-nodmg]" << nl;
    std::cout << "       " << progname << " [action] [options] <project> [<version>]" << nl;
    std::cout << "actions: [-help] [-headers] [-fetch] [-source] [-load] [-loadonly] [-group]" << nl;
    std::cout << "options: [-build <build>] [-target <target>]" << nl; // [-configuration=;
    std::cout << "         [-logdeps] [-nopatch] [-noload] [-nosource] [-codesign <identity>]" << nl;
    std::cout << "         [-depsbuild X [-depsbuild Y]]" << nl;
    std::cout << nl;
    std::cout << "Initialize Build:" << nl;
    std::cout << " -init                   Initialize a build (check '" << progname << " -init help' for more info)" << nl;
    std::cout << nl;
    std::cout << "Actions:" << nl;
    std::cout << " -headers                Do the installhdrs phase, instead of install." << nl;
    std::cout << " -fetch                  Only download necessary source and patch files." << nl;
    std::cout << " -source                 Extract, patch, and stage source." << nl;
    std::cout << " -load                   Populate the BuildRoot with one project." << nl;
    std::cout << " -loadonly               Only load dependencies into the build root, but don't build." << nl;
    std::cout << " -group                  Build all projects in the given darwinxref group." << nl;
    std::cout << nl;
    std::cout << "Options:" << nl;
    std::cout << " -nosource               Do not fetch or stage source. This assumes that the." << nl;
    std::cout << "                         source is already in place in the BuildRoot." << nl;
    std::cout << " -logdeps                Do magic to log the build-time dependencies." << nl;
    std::cout << " -nopatch                Don't patch sources before building." << nl;
    std::cout << " -noload                 Don't load dependencies into the build root." << nl;
    std::cout << " -target <target>        The makefile or xcode target to build." << nl;
    std::cout << " -configuration <config> Specify the build configuration to use." << nl;
    std::cout << " -build <build>          Specify the darwin build number to buld, e.g. 8B15." << nl;
    std::cout << " -depsbuild <build>      Specify the darwin build number to populate the BuildRoot." << nl;
    std::cout << " -codesign <identity>    Sign the built root, using the given CODE_SIGN_IDENTITY value." << nl;
    std::cout << nl;
    std::cout << " Parameters:" << nl;
    std::cout << "  <project> The name of the project to build" << nl;
    std::cout << "  <version> If specified, the version of the project to build" << nl;
    std::cout << "            this will default to the version associated with the" << nl;
    std::cout << "            currently running build." << nl;
    exit(1);
}

int main(int argc, char *argv[], char *env[]){
    actions_t actions = 0;
    options_t options;
    String project;
    String version;
    char ch;
    // set progrma name
    progname = basename(argv[0]);
    
    DarwinbuildInit(argc, argv);
    
    // In the future maybe add a 'world' option to build the whole OS.
    struct option arg_options[] = {
        { "headers",        no_argument,        0, 0x1  },
		{ "fetch",          no_argument,        0, 0x2  },
		{ "source",         no_argument,        0, 0x3  },
		{ "load",           no_argument,        0, 0x4  },
		{ "loadonly",       no_argument,        0, 0x5  },
		{ "group",          no_argument,        0, 0x6  },
		{ "nosource",       no_argument,        0, 0x7  },
        { "noload",         no_argument,        0, 0x8  },
        { "nopatch",        no_argument,        0, 0xa  },
        { "build",          required_argument,  0, 0xb  },
        { "logdeps",        required_argument,  0, 0xc  },
        { "depsbuild",      required_argument,  0, 0xd  },
		{ "target",         required_argument,  0, 0xe  },
		{ "codesign",       required_argument,  0, 0xf  },
        { "configuration",  required_argument,  0, 0x10 },
        { "help",           no_argument,        0, 'h'  },
		{ NULL, 0, 0, 0 }
	};

    while ((ch = getopt_long_only(argc, argv, "h", arg_options, NULL)) != (-1)) {
		switch (ch) {
            case 0x1  :
                actions |= ACTION_HEADERS;
                break;
            case 0x2  :
                actions |= ACTION_FETCH;
                break;
            case 0x3  :
                actions |= ACTION_SOURCE;
                break;
            case 0x4  :
                actions |= ACTION_LOAD;
                break;
            case 0x5  :
                actions |= ACTION_LOADONLY;
                break;
            case 0x6  :
                actions |= ACTION_GROUP;
                break;
            case 7  :
                options.nosource = true;
                break;
            case 0x8  :
                options.nopatch = true;
                break;
            case 0xa  :
                options.noload = true;
                break;
            case 0xb  :
                options.build = String(optarg);
                break;
            case 0xc  :
                options.logdeps = String(optarg);
                break;
            case 0xd  :
                options.depsbuild = String(optarg);
                printf("Not supported yet"); exit(0);
//                break;
            case 0xe  :
                options.target = String(optarg);
                break;
            case 0xf  :
                options.codesign = String(optarg);
                break;
            case 0x10 :
                printf("Not supported yet");
            case 'h'  : // fallthrough
			default:
                PrintUsage();
		}
	}

    argc -= optind;
    argv += optind;

    if (argc < 1){
        std::cerr << "Did not specify what project to build";
        PrintUsage();
    }
    // set the project
    project = argv[0];
    // set version if available
    if (argv[1] != NULL) {
        version = argv[1];
    }
    
    std::cout << "Building Project " << project << " with actions " << actions << nl;
    

    return 0;
}
