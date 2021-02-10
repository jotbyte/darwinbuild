//
//  darwinbuild.h
//  darwintrace
//
//  Created by John Othwolo on 1/29/21.
//  Copyright © 2021 The DarwinBuild Project. All rights reserved.
//

#ifndef darwinbuild_h
#define darwinbuild_h

#include <cstdio>
#include <string>

#define PREFIX              "/usr/local"
#define PWDP                "PWD"
#define XREFDB              ".build/xref.db"
#define DMGFILE             ".build/buildroot.sparsebundle"
#define DARWINXREF          "/usr/local/bin/darwinxref"
#define DATADIR             "/usr/local/share/darwinbuild/"
#define DIGEST              "/usr/local/share/darwinbuild/digest"
#define DARWINTRACE         "/usr/local/share/darwinbuild/darwintrace.dylib"
#define DITTO               "ditto"
#define DEFAULTPLISTSIT    ""
#define DARWINBUILD_ROOT    PWDP

extern char *progname;

int checkDir(std::string dir, bool silent);
int checkfile(std::string file, bool silent);
int readFile(std::string path, char *out);
int copyfile(std::string src, std::string dest, bool replace);
void symlink(std::string src, std::string dest);

bool isURL(std::string str);
bool isSSH(std::string str);

int newDir(std::string dir);
int DownloadFile(std::string url, std::string filename);

#endif
