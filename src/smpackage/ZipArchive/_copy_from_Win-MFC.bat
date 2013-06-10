@echo off
copy Windows\*.* *.*
copy mfc\*.* *.*
del __*.zcfg
echo Windows MFC > __Windows_MFC.zcfg
