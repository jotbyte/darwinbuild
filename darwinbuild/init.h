//
//  init.h
//  darwinbuild
//
//  Created by John Othwolo on 2/9/21.
//  Copyright © 2021 The DarwinBuild Project. All rights reserved.
//

#ifndef init_h
#define init_h

typedef struct {
    bool initialize;
    bool dmg;
    std::string build;
} init_options_t;

void DarwinbuildInit(init_options_t &opts);

#endif /* init_h */
