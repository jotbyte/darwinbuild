#!/bin/zsh -e

cp /usr/lib/libtcl.dylib $SRCROOT/darwinxref
chmod 644 $SRCROOT/darwinxref/libtcl8.6.dylib
install_name_tool -id '@rpath/libtcl8.6.dylib' $SRCROOT/darwinxref/libtcl8.6.dylib
