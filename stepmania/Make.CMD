@ECHO OFF
:CheckArgs
IF NOT %CMDEXTVERSION% GTR 0 GOTO NoExtensions
IF %1x EQU x GOTO NoArgs
IF %2x EQU x GOTO Banner
IF /I %2x EQU NOCLEANx GOTO Banner
IF /I %2x NEQ CLEANx GOTO BadArgs
IF %VSCOMNTOOLS%x EQU x GOTO BadEnv
IF NOT EXIST %VSCOMNTOOLS%\..\IDE\DEVENV.EXE GOTO NoEnv
IF EXIST .\STEPMANIA.SLN CD ..
IF NOT EXIST src\Stepmania.SLN GOTO NoSLN
IF EXIST BUILDOUT.TXT DEL BUILDOUT.TXT

IF /I %4x EQU NOBANNERx GOTO AFTERBANNER
REM Skip banner if we have the undocumented fourth argument.
REM (For batch files and such)
:BANNER
ECHO +-----------------------------------------------+
ECHO  Stepmania Batch build script by Eric Holbrook 
ECHO    for use with VS7 and Windows 2000.  (C)2003 
ECHO +-----------------------------------------------+ 
:AFTERBANNER

:Clean
ECHO Cleaning %1...
%VSCOMNTOOLS%\..\IDE\DEVENV.EXE /clean %1 Stepmania.SLN
IF NOT ERRORLEVEL 0 GOTO 
ECHO Done.

:Build
ECHO Performing %1 build...
%VSCOMNTOOLS%\..\IDE\DEVENV.EXE /build %1 StepMania.SLN /out .\BUILDOUT.TXT
ECHO ...Done!
IF ERRORLEVEL 0 GOTO SUCCESS
ECHO Problem reported: returned with Errorlevel %ERRORLEVEL%!
GOTO END

:CleanFailed
ECHO ERROR: Errorlevel %ERRORLEVEL% reported attempting to clean.
ECHO Press CTRL+C to abort build attempt.
PAUSE
GOTO BUILD
GOTO END

SETLOCAL
:Success
IF /I %3x EQU WINDOWx START .\BUILDOUT.TXT
IF /I %3x EQU VERBOSEx MORE .\BUILDOUT.TXT
ECHO No problems reported.
GOTO END

:NoArgs
ECHO ERROR - No arguments supplied!
GOTO Usage

:BadArgs
ECHO ERROR - Bad argument supplied!
GOTO Usage

:Usage
ECHO Usage: Make.CMD debug/release clean/noclean quiet/verbose/window
ECHO    Or: Make.CMD debug/release (noclean implied) (verbose implied)
ECHO.
ECHO Examples:
ECHO MAKE.CMD debug clean
ECHO MAKE.CMD release 
ECHO MAKE.CMD "unicode release" noclean
ECHO MAKE.CMD "debug" noclean window
ECHO.
ECHO Attempts to build the specified Stepmania.SLN from 
ECHO the current directory.  If 'clean' is specified as
ECHO the second argument, clean is performed before a build.
ECHO If 'verbose' is specified as the third argument, output
ECHO from the build will be suppressed, while 'window' causes
ECHO output to be displayed in a new notepad window.
GOTO END

:BadEnv
ECHO ERROR-  Your VSCOMNTOOLS environment variable is not set.
ECHO Consequently, I cant find your DEVENV.EXE file.  Sorry.  
GOTO END

:NoEnv
ECHO ERROR: Couldn't find DEVENV.EXE in the expected place.  
ECHO To abort, press CTRL+C now.
PAUSE
GOTO BUILD

:NoSLN
ECHO ERROR: No STEPMANIA.SLN file in src directory!
GOTO END

:NoExtensions
ECHO ERROR: You must have Windows NT Command Extensions installed
ECHO       in order to use this fine batch file.  Sorry.
GOTO END

:END
ENDLOCAL
