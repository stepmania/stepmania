SET PATH=%PATH%;c:\stepmania\stepmania\utils

del /q ..\stepmania.chm
del /s /q temp
rmdir /s /q temp

doxygen.exe doxygen_config

rem forfiles -ptemp -s -m*.png -c"pngcrushinplace.bat 0x22@FILE0x22"

doxygen\hhc temp\html\index.hhp
move temp\html\index.chm ..\stepmania.chm

