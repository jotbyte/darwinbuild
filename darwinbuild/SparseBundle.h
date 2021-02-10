//
//  SparseBundle.h
//  darwinbuild
//
//  Created by John Tipo James Othwolo on 2/1/21.
//  Copyright © 2021 The DarwinBuild Project. All rights reserved.
//

#ifndef SparseBundle_h
#define SparseBundle_h

#include <cstdio>
#include <string>
#include <CoreFoundation/CoreFoundation.h>

// Switch to PlistCPP
static const CFStringRef kBandSizeKey = CFSTR("band-size");
static const CFStringRef kDevSizeKey = CFSTR("size");
static std::string plist =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
"<plist version=\"1.0\">\n"
"<dict>\n"
                "\t<key>CFBundleInfoDictionaryVersion</key>\n"
                "\t<string>6.0</string>\n"
                "\t<key>band-size</key>\n"
                "\t<integer>%d</integer>\n"
                "\t<key>bundle-backingstore-version</key>\n"
                "\t<integer>1</integer>\n"
                "\t<key>diskimage-bundle-type</key>\n"
                "\t<string>com.apple.diskimage.sparsebundle</string>\n"
                "\t<key>size</key>\n"
                "\t<integer>%llu</integer>\n"
"</dict>\n"
"</plist>\n";

class SparseBundle {
    std::string path;
//    uint64_t bandSize;
//    uint64_t size;
    
public:
    
};

#endif /* SparseBundle_h */
