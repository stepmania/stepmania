@echo off
SET file=%1
SET tempfile=%file%.TEMP
echo %file%
pngcrush -q %file% %tempfile%
del %file%
rename %tempfile% %file%
