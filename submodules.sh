#!/bin/sh
pushd toolbox
   git submodule init
   git submodule update
   ./submodules.sh
popd

