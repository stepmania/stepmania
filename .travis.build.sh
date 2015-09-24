#!/bin/sh
set -e

cd Build
$HOME/cmake/bin/cmake -G 'Unix Makefiles' ..
make

