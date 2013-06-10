#!/bin/sh
cp -f Linux/* ./
cp -f stl/* ./
rm -f __*.zcfg
echo Linux STL > __Linux_STL.zcfg
