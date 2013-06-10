@echo off
copy Windows\*.* *.*
copy stl\*.* *.*
del __*.zcfg
echo Windows STL > __Windows_STL.zcfg
