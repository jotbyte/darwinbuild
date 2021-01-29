//
//  darwinbuild.h
//  darwintrace
//
//  Created by John Othwolo on 1/29/21.
//  Copyright © 2021 The DarwinBuild Project. All rights reserved.
//

#ifndef darwinbuild_h
#define darwinbuild_h

#include <stdio.h>

#define PREFIX              "%%PREFIX%%"
#define PWDP                "PWD"
#define XREFDB              ".build/xref.db"
#define DMGFILE             ".build/buildroot.sparsebundle"
#define DARWINXREF          "/bin/darwinxref"
#define DATADIR             "/share/darwinbuild"
#define DIGEST              "digest"
#define DARWINTRACE         "darwintrace.dylib"
#define DITTO               "ditto"
#define DEFAULTPLISTSITE    ""
#define DARWINBUILD_ROOT    PWDP

extern char *progname;

#endif /* darwinbuild_h */
