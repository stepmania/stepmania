# Microsoft Developer Studio Project File - Name="pui" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=pui - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pui.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pui.mak" CFG="pui - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pui - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "pui - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "pui"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pui - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "..\fnt" /I "..\sg" /I "..\util" /I "../../../glut/include" /I "../../glut/include" /I ".." /I "../glut/include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "GLUT_IS_PRESENT" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copy Library to plib directory
PostBuild_Cmds=copy release\*.lib ..\..\*.*	copy pu.h ..\..\pu.h
# End Special Build Tool

!ELSEIF  "$(CFG)" == "pui - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /Zi /Od /I "..\fnt" /I "..\sg" /I "..\util" /I "../../../glut/include" /I "../../glut/include" /I ".." /I "../glut/include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "GLUT_IS_PRESENT" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\pui_d.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copy Library to plib directory
PostBuild_Cmds=copy debug\*.lib ..\..\*.*	copy pu.h ..\..\pu.h
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "pui - Win32 Release"
# Name "pui - Win32 Debug"
# Begin Source File

SOURCE=.\pu.cxx
# End Source File
# Begin Source File

SOURCE=.\pu.h
# End Source File
# Begin Source File

SOURCE=.\puArrowButton.cxx
# End Source File
# Begin Source File

SOURCE=.\puBiSlider.cxx
# End Source File
# Begin Source File

SOURCE=.\puBox.cxx
# End Source File
# Begin Source File

SOURCE=.\puButton.cxx
# End Source File
# Begin Source File

SOURCE=.\puButtonBox.cxx
# End Source File
# Begin Source File

SOURCE=.\puComboBox.cxx
# End Source File
# Begin Source File

SOURCE=.\puDial.cxx
# End Source File
# Begin Source File

SOURCE=.\puDialogBox.cxx
# End Source File
# Begin Source File

SOURCE=.\puFilePicker.cxx
# End Source File
# Begin Source File

SOURCE=.\puFileSelector.cxx
# End Source File
# Begin Source File

SOURCE=.\puFont.cxx
# End Source File
# Begin Source File

SOURCE=.\puFrame.cxx
# End Source File
# Begin Source File

SOURCE=.\puGroup.cxx
# End Source File
# Begin Source File

SOURCE=.\puInput.cxx
# End Source File
# Begin Source File

SOURCE=.\puInterface.cxx
# End Source File
# Begin Source File

SOURCE=.\puLargeInput.cxx
# End Source File
# Begin Source File

SOURCE=.\puListBox.cxx
# End Source File
# Begin Source File

SOURCE=.\puLocal.h
# End Source File
# Begin Source File

SOURCE=.\puMenuBar.cxx
# End Source File
# Begin Source File

SOURCE=.\puObject.cxx
# End Source File
# Begin Source File

SOURCE=.\puOneShot.cxx
# End Source File
# Begin Source File

SOURCE=.\puPopup.cxx
# End Source File
# Begin Source File

SOURCE=.\puPopupMenu.cxx
# End Source File
# Begin Source File

SOURCE=.\puRange.cxx
# End Source File
# Begin Source File

SOURCE=.\puScrollBar.cxx
# End Source File
# Begin Source File

SOURCE=.\puSelectBox.cxx
# End Source File
# Begin Source File

SOURCE=.\puSlider.cxx
# End Source File
# Begin Source File

SOURCE=.\puSpinBox.cxx
# End Source File
# Begin Source File

SOURCE=.\puText.cxx
# End Source File
# Begin Source File

SOURCE=.\puTriSlider.cxx
# End Source File
# Begin Source File

SOURCE=.\puValue.cxx
# End Source File
# Begin Source File

SOURCE=.\puVerticalMenu.cxx
# End Source File
# End Target
# End Project
