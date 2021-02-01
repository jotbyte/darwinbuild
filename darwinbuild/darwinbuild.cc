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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <stdbool.h>
#include <getopt.h>
#include <sys/stat.h>

#include "darwinbuild.h"

char *progname = NULL;

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
    char *build;
    char *logdeps;
    char *depsbuild;
    char *codesign;
    char *target;
} options_t;

static void PrintInitHelp(){
    fprintf(stdout,  "usage: %s -init <build> [-nodmg ]\n", progname);
    fprintf(stdout,  "\n");
    fprintf(stdout,  "  <build>       can be a standard build number or a path to a plist.\n");
    fprintf(stdout,  "                supported user@host:/dir/file.plist,\n");
    fprintf(stdout,  "                               http://host/dir/file.plist \n");
    fprintf(stdout,  "                              paths: /dir/file.plist, \n");
    fprintf(stdout,  "  -nodmg        do not use a sparse image for build root (use a regular directory)\n");
    exit(1);
}

void DarwinbuildInit(int argc, char *argv[]){
    int ch;
    char *build = NULL;
    bool dmg = false;
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
                dmg = true;
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
        fprintf(stdout, "Unexpected argument %s\n", margv[0]);
        PrintInitHelp();
    }
    // If the arguments provided weren't for init, continue to next getopt
    if (build == NULL) {
        return;
    }
    int modes = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    // Otherwise initialize the build and exit.
    
    if (checkDir("Roots")) {
        mkdir("Roots", modes);
    }
    if (checkDir("Sources")) {
        mkdir("Sources", modes);
    }
    if (checkDir("Symbols")) {
        mkdir("Symbols", modes);
    }
    if (checkDir("Headers")) {
        mkdir("Headers", modes);
    }
    if (checkDir("Logs")) {
        mkdir("Logs", modes);
    }
    if (checkDir(".build")) {
        mkdir(".build", modes);
    }
    
    
    // TODO: Fnish init functionality
    
    printf("Initializing darwinbuild\n");
    exit(0);
}

static void PrintUsage(){
    fprintf(stdout, "Darwinbuild 1.0\n"); // TODO: Work on versioning
    fprintf(stdout, "Copyright © 2021 The PureDarwin Project.\n");
    fprintf(stdout, "This is free software; see the source for copying conditions.\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "usage: %s -init <build> [-nodmg]\n", progname);
    fprintf(stdout, "       %s [action] [options] <project> [<version>]\n", progname);
    fprintf(stdout, "actions: [-help] [-headers] [-fetch] [-source] [-load] [-loadonly] [-group]\n");
    fprintf(stdout, "options: [-build <build>] [-target <target>]\n"); // [-configuration=X]
    fprintf(stdout, "         [-logdeps] [-nopatch] [-noload] [-nosource] [-codesign <identity>]\n");
    fprintf(stdout, "         [-depsbuild X [-depsbuild Y]]\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "Initialize Build:\n");
    fprintf(stdout, " -init                   Initialize a build (check '%s -init help' for more info)\n", progname);
    fprintf(stdout, "\n");
    fprintf(stdout, "Actions:\n");
    fprintf(stdout, " -headers                Do the installhdrs phase, instead of install.\n");
    fprintf(stdout, " -fetch                  Only download necessary source and patch files.\n");
    fprintf(stdout, " -source                 Extract, patch, and stage source.\n");
    fprintf(stdout, " -load                   Populate the BuildRoot with one project.\n");
    fprintf(stdout, " -loadonly               Only load dependencies into the build root, but don't build.\n");
    fprintf(stdout, " -group                  Build all projects in the given darwinxref group.\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "Options:\n");
    fprintf(stdout, " -nosource               Do not fetch or stage source. This assumes that the.\n");
    fprintf(stdout, "                         source is already in place in the BuildRoot.\n");
    fprintf(stdout, " -logdeps                Do magic to log the build-time dependencies.\n");
    fprintf(stdout, " -nopatch                Don't patch sources before building.\n");
    fprintf(stdout, " -noload                 Don't load dependencies into the build root.\n");
    fprintf(stdout, " -target <target>        The makefile or xcode target to build.\n");
    fprintf(stdout, " -configuration <config> Specify the build configuration to use.\n");
    fprintf(stdout, " -build <build>          Specify the darwin build number to buld, e.g. 8B15.\n");
    fprintf(stdout, " -depsbuild <build>      Specify the darwin build number to populate the BuildRoot.\n");
    fprintf(stdout, " -codesign <identity>    Sign the built root, using the given CODE_SIGN_IDENTITY value.\n");
    fprintf(stdout, "\n");
    fprintf(stdout, " Parameters:\n");
    fprintf(stdout, "  <project> The name of the project to build\n");
    fprintf(stdout, "  <version> If specified, the version of the project to build\n");
    fprintf(stdout, "            this will default to the version associated with the\n");
    fprintf(stdout, "            currently running build.\n");
    exit(1);
}

int main(int argc, char *argv[], char *env[]){
    actions_t actions = 0;
    options_t options = {0};
    char *project = NULL;
    char *version = NULL;
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
                options.build = optarg;
                break;
            case 0xc  :
                options.logdeps = optarg;
                break;
            case 0xd  :
                options.depsbuild = optarg;
                printf("Not supported yet"); exit(0);
//                break;
            case 0xe  :
                options.target = optarg;
                break;
            case 0xf  :
                options.codesign = optarg;
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
        fprintf(stdout, "Did not specify what project to build");
        PrintUsage();
    }
    // set the project
    project = argv[0];
    // set version if available
    if (argv[1] != NULL) {
        version = argv[1];
    }
    
    printf("Building Project %s with actions %x\n", project, actions);
    

    return 0;
}
