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
 * This a replacement tool for the old clunky Darwinbuild script. Xcode's tools show that it's possible to
 * turn a tool like this into a C program, so this is exactly that.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <stdbool.h>
#include "darwinbuild.h"

#define check_arg() ({                              \
    if (argv[i+1] != NULL && argv[i+1][0] == '-') { \
        project = NULL;                             \
        PrintUsage();                               \
    }                                               \
})

#define check_noarg() ({                            \
    if (argv[i+1] != NULL && argv[i+1][0] != '-') { \
        project = NULL;                             \
        PrintUsage();                               \
    }                                               \
})

char *progname = NULL;

enum {
    ACTION_HEADERS,
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

static void PrintUsageLong(){
    fprintf(stdout, "Darwinbuild 1.0\n"); // TODO: Work on versioning
    fprintf(stdout, "Copyright © 2021 The PureDarwin Project.\n");
    fprintf(stdout, "This is free software; see the source for copying conditions.\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "usage: %s [action] [options] <project> [<version>]\n", progname);
    fprintf(stdout, "actions: [-help] [-headers] [-fetch] [-source] [-load] [-loadonly] [-group]\n");
    fprintf(stdout, "options: [-build <build>] [-target <target>]\n"); // [-configuration=X]
    fprintf(stdout, "         [-logdeps] [-nopatch] [-noload] [-nosource] [-codesign <identity>]\n");
//    fprintf(stdout, "         [-depsbuild X [-depsbuild Y]]\n");
    fprintf(stdout, "\n");
    fprintf(stdout, " Actions:\n");
    fprintf(stdout, "  -help      show detailed help\n");
    fprintf(stdout, "  -headers   Do the installhdrs phase, instead of install\n");
    fprintf(stdout, "  -fetch     Only download necessary source and patch files\n");
    fprintf(stdout, "  -source    Extract, patch, and stage source\n");
    fprintf(stdout, "  -load      Populate the BuildRoot with one project\n");
    fprintf(stdout, "  -loadonly  Only load dependencies into the build root, but don't build.\n");
    fprintf(stdout, "  -group     Build all projects in the given darwinxref group,\n");
    fprintf(stdout, "             as with darwinbuild -recursive\n");
    fprintf(stdout, "\n");
    fprintf(stdout, " Options:\n");
    fprintf(stdout, "  -nosource Do not fetch or stage source. This assumes that the\n");
    fprintf(stdout, "             source is already in place in the BuildRoot.\n");
    fprintf(stdout, "  -logdeps  Do magic to log the build-time dependencies\n");
    fprintf(stdout, "  -nopatch  Don't patch sources before building.\n");
    fprintf(stdout, "  -noload   Don't load dependencies into the build root.\n");
    fprintf(stdout, "  -target=X The makefile or xcode target to build\n");
    fprintf(stdout, "  -configuration=X Specify the build configuration to use\n");
    fprintf(stdout, "  -build=X  Specify the darwin build number to buld, e.g. 8B15\n");
//    fprintf(stdout, "  -depsbuild=X Specify the darwin build number to populate the BuildRoot\n");
    fprintf(stdout, "  -codesign=<identity> Sign the built root, using the given CODE_SIGN_IDENTITY value\n");
    fprintf(stdout, "\n");
    fprintf(stdout, " Parameters:\n");
    fprintf(stdout, "  <project> The name of the project to build\n");
    fprintf(stdout, "  <version> If specified, the version of the project to build\n");
    fprintf(stdout, "            this will default to the version associated with the\n");
    fprintf(stdout, "            currently running build.\n");
    exit(1);
    __builtin_unreachable();
}

/*
 * Interpret our arguments:
 *
 *  Actions:
 *   -headers   Do the installhdrs phase, instead of install
 *   -fetch     Only download necessary source and patch files
 *   -source    Extract, patch, and stage source
 *   -load      Populate the BuildRoot with one project
 *   -loadonly  Only load dependencies into the build root, but don't build.
 *   -recursive Build given project(s) and all required dependencies
 *   -group     Build all projects in the given darwinxref group,
 *              as with darwinbuild -recursive
 *
 *  Options:
 *   -nosource Do not fetch or stage source. This assumes that the
 *              source is already in place in the BuildRoot.
 *   -logdeps  Do magic to log the build-time dependencies
 *   -nopatch  Don't patch sources before building.
 *   -noload   Don't load dependencies into the build root.
 *   -target=X The makefile or xcode target to build
 *   -configuration=X Specify the build configuration to use
 *   -build=X  Specify the darwin build number to buld, e.g. 8B15
 *   -depsbuild=X Specify the darwin build number to populate the BuildRoot
 *   -codesign=<identity> Sign the built root, using the given CODE_SIGN_IDENTITY value
 *
 *  Parameters:
 *   <project> The name of the project to build
 *   <version> If specified, the version of the project to build
 *             this will default to the version associated with the
 *             currently running build.
 */

static void PrintUsage() {
    fprintf(stdout, "Darwinbuild 1.0\n"); // TODO: Work on versioning
    fprintf(stdout, "Copyright © 2021 The PureDarwin Project.\n");
    fprintf(stdout, "This is free software; see the source for copying conditions.\n");
    fprintf(stdout, "usage: %s [action] [options] <project> [<version>]\n", progname);
    fprintf(stdout, "actions: [-help] [-headers] [-fetch] [-source] [-load] [-loadonly] [-group]\n");
    fprintf(stdout, "options: [-build <build>] [-target <target>]\n"); // [-configuration=X]
    fprintf(stdout, "         [-logdeps] [-nopatch] [-noload] [-nosource] [-codesign <identity>]\n");
//    fprintf(stdout, "         [-depsbuild X [-depsbuild Y]]\n");
    exit(1);
    __builtin_unreachable();
}

int main(int argc, char *argv[], char *env[]){
    actions_t actions = -1;
    options_t options = {0};
    char *project = NULL;
    char *version = NULL;
    
    // set progrma name
    progname = basename(argv[0]);
    argc--;
    argv++;
    
    if (argc < 1)
        PrintUsage();
    // Parse args
    for (int i=0; i<argc && argv[i] != NULL; i++) {

        if (argv[i][0] == '-') {
            const char *carg = ++argv[i]; // get rid of the hyphen
            // Actions
            if (!strncmp("help", carg, sizeof("help"))) {
                project = NULL;
                PrintUsageLong();
            }
            if (!strncmp("headers", carg, sizeof("headers"))) {
                actions |= ACTION_HEADERS;
            }
            if (!strncmp("fetch", carg, sizeof("fetch"))) {
                actions |= ACTION_FETCH;
            }
            if (!strncmp("source", carg, sizeof("source"))) {
                actions |= ACTION_SOURCE;
            }
            if (!strncmp("load", carg, sizeof("load"))) {
                actions |= ACTION_LOAD;
            }
            if (!strncmp("loadonly", carg, sizeof("loadonly"))) {
                actions |= ACTION_LOADONLY;
            }
            if (!strncmp("group", carg, sizeof("group"))) {
                actions |= ACTION_GROUP;
            }
            
            // Options
            // boolean options. shouldn't have arguments.
            if (!strncmp("nosource", carg, sizeof("nosource"))) {
                // this checks if the next argv has a '-'.
                check_noarg();
                options.nosource = true;
            }
            if (!strncmp("nopatch", carg, sizeof("nopatch"))) {
                check_noarg();
                options.nopatch = true;
            }
            if (!strncmp("noload", carg, sizeof("noload"))) {
                check_noarg();
                options.noload = true;
            }
            
            // string options. they should have arguments
            if (!strncmp("build", carg, sizeof("build"))) {
                check_arg();
                options.build = argv[++i];
            }
            if (!strncmp("logdeps", carg, sizeof("logdeps"))) {
                check_arg();
            }
            if (!strncmp("depsbuild", carg, sizeof("depsbuild"))) {
                check_arg();
                options.depsbuild = argv[++i];
            }
            if (!strncmp("target", carg, sizeof("target"))) {
                check_arg();
                options.target = argv[++i];
            }
            if (!strncmp("codesign", carg, sizeof("codesign"))) {
                check_arg();
                options.codesign = argv[++i];
            }
            
        } else {
            if (project == NULL) {
                project = argv[i];
            }
            else if (version == NULL){
                version = argv[i];
            } else { // there shouldn't be anything after the version.
                PrintUsage();
            }
        }
    }
    
    if (project == NULL) {
        fprintf(stdout, "Did not specify what project to build"); // In the future maybe add a world option to build the whole OS.
        PrintUsage();
    }
    
    
    return 0;
}

