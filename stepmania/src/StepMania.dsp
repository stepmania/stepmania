# Microsoft Developer Studio Project File - Name="StepMania" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=StepMania - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "StepMania.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "StepMania.mak" CFG="StepMania - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "StepMania - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "StepMania - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "StepMania - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "StepMania___Win32_Debug_OGL"
# PROP BASE Intermediate_Dir "StepMania___Win32_Debug_OGL"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug6"
# PROP Intermediate_Dir "../Debug6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gm /GX /ZI /Od /I "." /I "SDL-1.2.5\include" /I "SDL_image-1.2" /I "plib-1.6.0" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "DEBUG" /Fr /YX"global.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "vorbis" /I "libjpeg" /I "lua-5.0\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "WINDOWS" /D "_MBCS" /D "DEBUG" /FR /YX"global.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 $(intdir)\verstub.obj kernel32.lib shell32.lib user32.lib gdi32.lib advapi32.lib winmm.lib /nologo /subsystem:windows /pdb:"../debug6/StepMania-debug.pdb" /map /debug /machine:I386 /nodefaultlib:"libcmtd.lib" /out:"../StepMania-debug.exe"
# SUBTRACT BASE LINK32 /verbose /profile /pdb:none /incremental:no /nodefaultlib
# ADD LINK32 $(intdir)\verstub.obj kernel32.lib gdi32.lib shell32.lib user32.lib advapi32.lib winmm.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"msvcrt.lib" /out:"../Program/StepMania-debug.exe"
# SUBTRACT LINK32 /verbose /profile /pdb:none /incremental:no /nodefaultlib
# Begin Special Build Tool
IntDir=.\../Debug6
TargetDir=\cvs\stepmania\Program
TargetName=StepMania-debug
SOURCE="$(InputPath)"
PreLink_Cmds=archutils\Win32\verinc                                                                                                                                                                                            	cl                                                    /Zl                                                    /nologo                                                    /c                                                    verstub.cpp                                                    /Fo$(IntDir)\ 
PostBuild_Cmds=archutils\Win32\mapconv $(IntDir)\$(TargetName).map $(TargetDir)\StepMania.vdi
# End Special Build Tool

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "StepMania___Win32_Release_OGL"
# PROP BASE Intermediate_Dir "StepMania___Win32_Release_OGL"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release6"
# PROP Intermediate_Dir "../Release6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /O2 /Ob2 /I "." /I "SDL-1.2.5\include" /I "SDL_image-1.2" /I "plib-1.6.0" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /Ob2 /I "." /I "vorbis" /I "libjpeg" /I "lua-5.0\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WINDOWS" /D "_MBCS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"../StepMania-StackTrace.bsc"
# ADD BSC32 /nologo /o"../StepMania-StackTrace.bsc"
LINK32=link.exe
# ADD BASE LINK32 $(intdir)\verstub.obj kernel32.lib gdi32.lib shell32.lib user32.lib advapi32.lib winmm.lib /nologo /subsystem:windows /pdb:"../release6/StepMania.pdb" /map /debug /machine:I386
# SUBTRACT BASE LINK32 /verbose /pdb:none
# ADD LINK32 $(intdir)\verstub.obj kernel32.lib gdi32.lib shell32.lib user32.lib advapi32.lib winmm.lib /nologo /subsystem:windows /map /machine:I386 /out:"..\Program\StepMania.exe"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
IntDir=.\../Release6
TargetDir=\cvs\stepmania\Program
TargetName=StepMania
SOURCE="$(InputPath)"
PreLink_Cmds=archutils\Win32\verinc      	cl       /Zl       /nologo       /c       verstub.cpp       /Fo$(IntDir)\ 
PostBuild_Cmds=archutils\Win32\mapconv $(IntDir)\$(TargetName).map $(TargetDir)\StepMania.vdi
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "StepMania - Win32 Debug"
# Name "StepMania - Win32 Release"
# Begin Group "Rage"

# PROP Default_Filter ""
# Begin Group "Helpers"

# PROP Default_Filter ""
# Begin Group "pcre"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcre\get.c
# End Source File
# Begin Source File

SOURCE=.\pcre\internal.h
# End Source File
# Begin Source File

SOURCE=.\pcre\maketables.c
# End Source File
# Begin Source File

SOURCE=.\pcre\pcre.c
# End Source File
# Begin Source File

SOURCE=.\pcre\pcre.h
# End Source File
# Begin Source File

SOURCE=.\pcre\study.c
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=.\RageBitmapTexture.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

# ADD BASE CPP /YX
# ADD CPP /YX

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageBitmapTexture.h
# End Source File
# Begin Source File

SOURCE=.\RageDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\RageDisplay.h
# End Source File
# Begin Source File

SOURCE=.\RageDisplay_D3D.cpp
# End Source File
# Begin Source File

SOURCE=.\RageDisplay_D3D.h
# End Source File
# Begin Source File

SOURCE=.\RageDisplay_Null.cpp
# End Source File
# Begin Source File

SOURCE=.\RageDisplay_Null.h
# End Source File
# Begin Source File

SOURCE=.\RageDisplay_OGL.cpp
# End Source File
# Begin Source File

SOURCE=.\RageDisplay_OGL.h
# End Source File
# Begin Source File

SOURCE=.\RageDisplay_OGL_Extensions.cpp
# End Source File
# Begin Source File

SOURCE=.\RageDisplay_OGL_Extensions.h
# End Source File
# Begin Source File

SOURCE=.\RageException.cpp
# End Source File
# Begin Source File

SOURCE=.\RageException.h
# End Source File
# Begin Source File

SOURCE=.\RageFile.cpp
# End Source File
# Begin Source File

SOURCE=.\RageFile.h
# End Source File
# Begin Source File

SOURCE=.\RageFileBasic.cpp
# End Source File
# Begin Source File

SOURCE=.\RageFileBasic.h
# End Source File
# Begin Source File

SOURCE=.\RageFileDriver.cpp
# End Source File
# Begin Source File

SOURCE=.\RageFileDriver.h
# End Source File
# Begin Source File

SOURCE=.\RageFileDriverDeflate.cpp
# End Source File
# Begin Source File

SOURCE=.\RageFileDriverDeflate.h
# End Source File
# Begin Source File

SOURCE=.\RageFileDriverDirect.cpp
# End Source File
# Begin Source File

SOURCE=.\RageFileDriverDirect.h
# End Source File
# Begin Source File

SOURCE=.\RageFileDriverDirectHelpers.cpp
# End Source File
# Begin Source File

SOURCE=.\RageFileDriverDirectHelpers.h
# End Source File
# Begin Source File

SOURCE=.\RageFileDriverMemory.cpp
# End Source File
# Begin Source File

SOURCE=.\RageFileDriverMemory.h
# End Source File
# Begin Source File

SOURCE=.\RageFileDriverSlice.cpp
# End Source File
# Begin Source File

SOURCE=.\RageFileDriverSlice.h
# End Source File
# Begin Source File

SOURCE=.\RageFileDriverTimeout.cpp
# End Source File
# Begin Source File

SOURCE=.\RageFileDriverTimeout.h
# End Source File
# Begin Source File

SOURCE=.\RageFileDriverZip.cpp
# End Source File
# Begin Source File

SOURCE=.\RageFileDriverZip.h
# End Source File
# Begin Source File

SOURCE=.\RageFileManager.cpp
# End Source File
# Begin Source File

SOURCE=.\RageFileManager.h
# End Source File
# Begin Source File

SOURCE=.\RageInput.cpp
# End Source File
# Begin Source File

SOURCE=.\RageInput.h
# End Source File
# Begin Source File

SOURCE=.\RageInputDevice.cpp
# End Source File
# Begin Source File

SOURCE=.\RageInputDevice.h
# End Source File
# Begin Source File

SOURCE=.\RageLog.cpp
# End Source File
# Begin Source File

SOURCE=.\RageLog.h
# End Source File
# Begin Source File

SOURCE=.\RageMath.cpp
# End Source File
# Begin Source File

SOURCE=.\RageMath.h
# End Source File
# Begin Source File

SOURCE=.\RageModelGeometry.cpp
# End Source File
# Begin Source File

SOURCE=.\RageModelGeometry.h
# End Source File
# Begin Source File

SOURCE=.\RageSound.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSound.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundManager.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSoundManager.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundMixBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSoundMixBuffer.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundPosMap.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSoundPosMap.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Chain.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Chain.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_FileReader.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_FileReader.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_MP3.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_MP3.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Preload.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Preload.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Resample.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Resample.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Resample_Fast.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Resample_Fast.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Resample_Good.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Resample_Good.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Vorbisfile.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Vorbisfile.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_WAV.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_WAV.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundResampler.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSoundResampler.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSoundUtil.h
# End Source File
# Begin Source File

SOURCE=.\RageSurface.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSurface.h
# End Source File
# Begin Source File

SOURCE=.\RageSurface_Load.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSurface_Load.h
# End Source File
# Begin Source File

SOURCE=.\RageSurface_Load_BMP.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSurface_Load_BMP.h
# End Source File
# Begin Source File

SOURCE=.\RageSurface_Load_GIF.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSurface_Load_GIF.h
# End Source File
# Begin Source File

SOURCE=.\RageSurface_Load_JPEG.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSurface_Load_JPEG.h
# End Source File
# Begin Source File

SOURCE=.\RageSurface_Load_PNG.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSurface_Load_PNG.h
# End Source File
# Begin Source File

SOURCE=.\RageSurface_Load_XPM.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSurface_Load_XPM.h
# End Source File
# Begin Source File

SOURCE=.\RageSurface_Save_BMP.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSurface_Save_BMP.h
# End Source File
# Begin Source File

SOURCE=.\RageSurface_Save_JPEG.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSurface_Save_JPEG.h
# End Source File
# Begin Source File

SOURCE=.\RageSurfaceUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSurfaceUtils.h
# End Source File
# Begin Source File

SOURCE=.\RageSurfaceUtils_Dither.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSurfaceUtils_Dither.h
# End Source File
# Begin Source File

SOURCE=.\RageSurfaceUtils_Palettize.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSurfaceUtils_Palettize.h
# End Source File
# Begin Source File

SOURCE=.\RageSurfaceUtils_Zoom.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSurfaceUtils_Zoom.h
# End Source File
# Begin Source File

SOURCE=.\RageTexture.cpp
# End Source File
# Begin Source File

SOURCE=.\RageTexture.h
# End Source File
# Begin Source File

SOURCE=.\RageTextureID.cpp
# End Source File
# Begin Source File

SOURCE=.\RageTextureID.h
# End Source File
# Begin Source File

SOURCE=.\RageTextureManager.cpp
# End Source File
# Begin Source File

SOURCE=.\RageTextureManager.h
# End Source File
# Begin Source File

SOURCE=.\RageThreads.cpp
# End Source File
# Begin Source File

SOURCE=.\RageThreads.h
# End Source File
# Begin Source File

SOURCE=.\RageTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\RageTimer.h
# End Source File
# Begin Source File

SOURCE=.\RageTypes.h
# End Source File
# Begin Source File

SOURCE=.\RageUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\RageUtil.h
# End Source File
# Begin Source File

SOURCE=.\RageUtil_AutoPtr.h
# End Source File
# Begin Source File

SOURCE=.\RageUtil_BackgroundLoader.cpp
# End Source File
# Begin Source File

SOURCE=.\RageUtil_BackgroundLoader.h
# End Source File
# Begin Source File

SOURCE=.\RageUtil_CharConversions.cpp
# End Source File
# Begin Source File

SOURCE=.\RageUtil_CharConversions.h
# End Source File
# Begin Source File

SOURCE=.\RageUtil_FileDB.cpp
# End Source File
# Begin Source File

SOURCE=.\RageUtil_FileDB.h
# End Source File
# Begin Source File

SOURCE=.\RageUtil_WorkerThread.cpp
# End Source File
# Begin Source File

SOURCE=.\RageUtil_WorkerThread.h
# End Source File
# End Group
# Begin Group "Data Structures"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ActorCommands.cpp
# End Source File
# Begin Source File

SOURCE=.\ActorCommands.h
# End Source File
# Begin Source File

SOURCE=.\Attack.cpp
# End Source File
# Begin Source File

SOURCE=.\Attack.h
# End Source File
# Begin Source File

SOURCE=.\AutoKeysounds.cpp
# End Source File
# Begin Source File

SOURCE=.\AutoKeysounds.h
# End Source File
# Begin Source File

SOURCE=.\BackgroundUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\BackgroundUtil.h
# End Source File
# Begin Source File

SOURCE=.\BannerCache.cpp
# End Source File
# Begin Source File

SOURCE=.\BannerCache.h
# End Source File
# Begin Source File

SOURCE=.\CatalogXml.cpp
# End Source File
# Begin Source File

SOURCE=.\CatalogXml.h
# End Source File
# Begin Source File

SOURCE=.\Character.cpp
# End Source File
# Begin Source File

SOURCE=.\Character.h
# End Source File
# Begin Source File

SOURCE=.\CodeDetector.cpp
# End Source File
# Begin Source File

SOURCE=.\CodeDetector.h
# End Source File
# Begin Source File

SOURCE=.\Command.cpp
# End Source File
# Begin Source File

SOURCE=.\Command.h
# End Source File
# Begin Source File

SOURCE=.\CommonMetrics.cpp
# End Source File
# Begin Source File

SOURCE=.\CommonMetrics.h
# End Source File
# Begin Source File

SOURCE=.\Course.cpp
# End Source File
# Begin Source File

SOURCE=.\Course.h
# End Source File
# Begin Source File

SOURCE=.\CourseUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\CourseUtil.h
# End Source File
# Begin Source File

SOURCE=.\DateTime.cpp
# End Source File
# Begin Source File

SOURCE=.\DateTime.h
# End Source File
# Begin Source File

SOURCE=.\Difficulty.cpp
# End Source File
# Begin Source File

SOURCE=.\Difficulty.h
# End Source File
# Begin Source File

SOURCE=.\EnumHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\EnumHelper.h
# End Source File
# Begin Source File

SOURCE=.\Font.cpp
# End Source File
# Begin Source File

SOURCE=.\Font.h
# End Source File
# Begin Source File

SOURCE=.\FontCharAliases.cpp
# End Source File
# Begin Source File

SOURCE=.\FontCharAliases.h
# End Source File
# Begin Source File

SOURCE=.\FontCharmaps.cpp
# End Source File
# Begin Source File

SOURCE=.\FontCharmaps.h
# End Source File
# Begin Source File

SOURCE=.\Foreach.h
# End Source File
# Begin Source File

SOURCE=.\Game.cpp
# End Source File
# Begin Source File

SOURCE=.\Game.h
# End Source File
# Begin Source File

SOURCE=.\GameCommand.cpp
# End Source File
# Begin Source File

SOURCE=.\GameCommand.h
# End Source File
# Begin Source File

SOURCE=.\GameConstantsAndTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\GameConstantsAndTypes.h
# End Source File
# Begin Source File

SOURCE=.\GameInput.cpp
# End Source File
# Begin Source File

SOURCE=.\GameInput.h
# End Source File
# Begin Source File

SOURCE=.\GameTypes.h
# End Source File
# Begin Source File

SOURCE=.\Grade.cpp
# End Source File
# Begin Source File

SOURCE=.\Grade.h
# End Source File
# Begin Source File

SOURCE=.\HighScore.cpp
# End Source File
# Begin Source File

SOURCE=.\HighScore.h
# End Source File
# Begin Source File

SOURCE=.\Inventory.cpp
# End Source File
# Begin Source File

SOURCE=.\Inventory.h
# End Source File
# Begin Source File

SOURCE=.\LuaBinding.h
# End Source File
# Begin Source File

SOURCE=.\LuaFunctions.h
# End Source File
# Begin Source File

SOURCE=.\LuaReference.cpp
# End Source File
# Begin Source File

SOURCE=.\LuaReference.h
# End Source File
# Begin Source File

SOURCE=.\LyricsLoader.cpp
# End Source File
# Begin Source File

SOURCE=.\LyricsLoader.h
# End Source File
# Begin Source File

SOURCE=.\NoteData.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteData.h
# End Source File
# Begin Source File

SOURCE=.\NoteDataUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteDataUtil.h
# End Source File
# Begin Source File

SOURCE=.\NoteDataWithScoring.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteDataWithScoring.h
# End Source File
# Begin Source File

SOURCE=.\NoteFieldPositioning.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteFieldPositioning.h
# End Source File
# Begin Source File

SOURCE=.\NotesLoader.cpp
# End Source File
# Begin Source File

SOURCE=.\NotesLoader.h
# End Source File
# Begin Source File

SOURCE=.\NotesLoaderBMS.cpp
# End Source File
# Begin Source File

SOURCE=.\NotesLoaderBMS.h
# End Source File
# Begin Source File

SOURCE=.\NotesLoaderDWI.cpp
# End Source File
# Begin Source File

SOURCE=.\NotesLoaderDWI.h
# End Source File
# Begin Source File

SOURCE=.\NotesLoaderKSF.cpp
# End Source File
# Begin Source File

SOURCE=.\NotesLoaderKSF.h
# End Source File
# Begin Source File

SOURCE=.\NotesLoaderSM.cpp
# End Source File
# Begin Source File

SOURCE=.\NotesLoaderSM.h
# End Source File
# Begin Source File

SOURCE=.\NotesWriterDWI.cpp
# End Source File
# Begin Source File

SOURCE=.\NotesWriterDWI.h
# End Source File
# Begin Source File

SOURCE=.\NotesWriterSM.cpp
# End Source File
# Begin Source File

SOURCE=.\NotesWriterSM.h
# End Source File
# Begin Source File

SOURCE=.\NoteTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteTypes.h
# End Source File
# Begin Source File

SOURCE=.\OptionRowHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionRowHandler.h
# End Source File
# Begin Source File

SOURCE=.\PlayerAI.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerAI.h
# End Source File
# Begin Source File

SOURCE=.\PlayerNumber.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerNumber.h
# End Source File
# Begin Source File

SOURCE=.\PlayerOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerOptions.h
# End Source File
# Begin Source File

SOURCE=.\PlayerStageStats.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerStageStats.h
# End Source File
# Begin Source File

SOURCE=.\PlayerState.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerState.h
# End Source File
# Begin Source File

SOURCE=.\Preference.cpp
# End Source File
# Begin Source File

SOURCE=.\Preference.h
# End Source File
# Begin Source File

SOURCE=.\Profile.cpp
# End Source File
# Begin Source File

SOURCE=.\Profile.h
# End Source File
# Begin Source File

SOURCE=.\RadarValues.cpp
# End Source File
# Begin Source File

SOURCE=.\RadarValues.h
# End Source File
# Begin Source File

SOURCE=.\RandomSample.cpp
# End Source File
# Begin Source File

SOURCE=.\RandomSample.h
# End Source File
# Begin Source File

SOURCE=.\ScoreKeeper.h
# End Source File
# Begin Source File

SOURCE=.\ScoreKeeperMAX2.cpp
# End Source File
# Begin Source File

SOURCE=.\ScoreKeeperMAX2.h
# End Source File
# Begin Source File

SOURCE=.\ScoreKeeperRave.cpp
# End Source File
# Begin Source File

SOURCE=.\ScoreKeeperRave.h
# End Source File
# Begin Source File

SOURCE=.\ScreenDimensions.cpp
# End Source File
# Begin Source File

SOURCE=.\Song.cpp
# End Source File
# Begin Source File

SOURCE=.\song.h
# End Source File
# Begin Source File

SOURCE=.\SongCacheIndex.cpp
# End Source File
# Begin Source File

SOURCE=.\SongCacheIndex.h
# End Source File
# Begin Source File

SOURCE=.\SongOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\SongOptions.h
# End Source File
# Begin Source File

SOURCE=.\SongUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\SongUtil.h
# End Source File
# Begin Source File

SOURCE=.\StageStats.cpp
# End Source File
# Begin Source File

SOURCE=.\StageStats.h
# End Source File
# Begin Source File

SOURCE=.\Steps.cpp
# End Source File
# Begin Source File

SOURCE=.\Steps.h
# End Source File
# Begin Source File

SOURCE=.\StepsUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\StepsUtil.h
# End Source File
# Begin Source File

SOURCE=.\Style.cpp
# End Source File
# Begin Source File

SOURCE=.\Style.h
# End Source File
# Begin Source File

SOURCE=.\StyleInput.h
# End Source File
# Begin Source File

SOURCE=.\StyleUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\StyleUtil.h
# End Source File
# Begin Source File

SOURCE=.\TimingData.cpp
# End Source File
# Begin Source File

SOURCE=.\TimingData.h
# End Source File
# Begin Source File

SOURCE=.\TitleSubstitution.cpp
# End Source File
# Begin Source File

SOURCE=.\TitleSubstitution.h
# End Source File
# Begin Source File

SOURCE=.\Trail.cpp
# End Source File
# Begin Source File

SOURCE=.\Trail.h
# End Source File
# Begin Source File

SOURCE=.\TrailUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\TrailUtil.h
# End Source File
# End Group
# Begin Group "File Types"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\IniFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IniFile.h
# End Source File
# Begin Source File

SOURCE=.\MsdFile.cpp
# End Source File
# Begin Source File

SOURCE=.\MsdFile.h
# End Source File
# Begin Source File

SOURCE=.\XmlFile.cpp
# End Source File
# Begin Source File

SOURCE=.\XmlFile.h
# End Source File
# End Group
# Begin Group "StepMania"

# PROP Default_Filter ""
# Begin Group "arch"

# PROP Default_Filter ""
# Begin Group "LoadingWindow"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\LoadingWindow\LoadingWindow.h
# End Source File
# Begin Source File

SOURCE=.\arch\LoadingWindow\LoadingWindow_Null.h
# End Source File
# Begin Source File

SOURCE=.\arch\LoadingWindow\LoadingWindow_Win32.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\LoadingWindow\LoadingWindow_Win32.h
# End Source File
# End Group
# Begin Group "Sound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\Sound\DSoundHelpers.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\DSoundHelpers.h
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver.h
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_DSound.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_DSound.h
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_DSound_Software.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_DSound_Software.h
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_Generic_Software.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_Generic_Software.h
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_Null.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_Null.h
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_WaveOut.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_WaveOut.h
# End Source File
# End Group
# Begin Group "ArchHooks"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\ArchHooks\ArchHooks.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\ArchHooks\ArchHooks.h
# End Source File
# Begin Source File

SOURCE=.\arch\ArchHooks\ArchHooks_none.h
# End Source File
# Begin Source File

SOURCE=.\arch\ArchHooks\ArchHooks_Win32.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\ArchHooks\ArchHooks_Win32.h
# End Source File
# End Group
# Begin Group "InputHandler"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler.h
# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_DirectInput.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_DirectInput.h
# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_DirectInputHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_DirectInputHelper.h
# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_MonkeyKeyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_MonkeyKeyboard.h
# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_Win32_MIDI.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_Win32_MIDI.h
# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_Win32_Para.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_Win32_Para.h
# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_Win32_Pump.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_Win32_Pump.h
# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\Selector_InputHandler.h
# End Source File
# End Group
# Begin Group "MovieTexture"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture.h
# End Source File
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture_DShow.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture_DShow.h
# End Source File
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture_DShowHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture_DShowHelper.h
# End Source File
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture_FFMpeg.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture_FFMpeg.h
# End Source File
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture_Null.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture_Null.h
# End Source File
# End Group
# Begin Group "LowLevelWindow"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\LowLevelWindow\LowLevelWindow.h
# End Source File
# Begin Source File

SOURCE=.\arch\LowLevelWindow\LowLevelWindow_Null.h
# End Source File
# Begin Source File

SOURCE=.\arch\LowLevelWindow\LowLevelWindow_Win32.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\LowLevelWindow\LowLevelWindow_Win32.h
# End Source File
# End Group
# Begin Group "Lights"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\Lights\LightsDriver.h
# End Source File
# Begin Source File

SOURCE=.\arch\Lights\LightsDriver_Null.h
# End Source File
# Begin Source File

SOURCE=.\arch\Lights\LightsDriver_SystemMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\Lights\LightsDriver_SystemMessage.h
# End Source File
# Begin Source File

SOURCE=.\arch\Lights\LightsDriver_Win32Parallel.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\Lights\LightsDriver_Win32Parallel.h
# End Source File
# End Group
# Begin Group "MemoryCard"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\MemoryCard\MemoryCardDriver.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\MemoryCard\MemoryCardDriver.h
# End Source File
# Begin Source File

SOURCE=.\arch\MemoryCard\MemoryCardDriver_Null.h
# End Source File
# Begin Source File

SOURCE=.\arch\MemoryCard\MemoryCardDriverThreaded_Windows.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\MemoryCard\MemoryCardDriverThreaded_Windows.h
# End Source File
# End Group
# Begin Group "Dialog"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\Dialog\Dialog.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\Dialog\Dialog.h
# End Source File
# Begin Source File

SOURCE=.\arch\Dialog\DialogDriver.h
# End Source File
# Begin Source File

SOURCE=.\arch\Dialog\DialogDriver_Win32.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\Dialog\DialogDriver_Win32.h
# End Source File
# End Group
# Begin Group "Threads"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\Threads\Threads.h
# End Source File
# Begin Source File

SOURCE=.\arch\Threads\Threads_Win32.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\Threads\Threads_Win32.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\arch\arch.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\arch.h
# End Source File
# Begin Source File

SOURCE=.\arch\arch_default.h
# End Source File
# Begin Source File

SOURCE=.\arch\arch_platform.h
# End Source File
# End Group
# Begin Group "system"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\archutils\Win32\AppInstance.cpp
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\AppInstance.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\arch_setup.cpp
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\arch_setup.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\Crash.cpp
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\Crash.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\DebugInfoHunt.cpp
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\DebugInfoHunt.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\GetFileInformation.cpp
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\GetFileInformation.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\GotoURL.cpp
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\GotoURL.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\GraphicsWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\GraphicsWindow.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\RegistryAccess.cpp
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\RegistryAccess.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\RestartProgram.cpp
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\RestartProgram.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\StepMania.ICO
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\USB.cpp
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\USB.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\VideoDriverInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\VideoDriverInfo.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\WindowIcon.cpp
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\WindowIcon.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\WindowsResources.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\WindowsResources.rc
# End Source File
# End Group
# Begin Source File

SOURCE=.\global.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

# ADD BASE CPP /Yc"global.h"
# ADD CPP /Yc"global.h"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\global.h
# End Source File
# Begin Source File

SOURCE=.\ScreenDimensions.h
# End Source File
# Begin Source File

SOURCE=.\StdString.h
# End Source File
# Begin Source File

SOURCE=.\StepMania.cpp
# End Source File
# Begin Source File

SOURCE=.\StepMania.h
# End Source File
# End Group
# Begin Group "Actors"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Actor.cpp
# End Source File
# Begin Source File

SOURCE=.\Actor.h
# End Source File
# Begin Source File

SOURCE=.\ActorCollision.h
# End Source File
# Begin Source File

SOURCE=.\ActorFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\ActorFrame.h
# End Source File
# Begin Source File

SOURCE=.\ActorScroller.cpp
# End Source File
# Begin Source File

SOURCE=.\ActorScroller.h
# End Source File
# Begin Source File

SOURCE=.\ActorUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\ActorUtil.h
# End Source File
# Begin Source File

SOURCE=.\AutoActor.cpp
# End Source File
# Begin Source File

SOURCE=.\AutoActor.h
# End Source File
# Begin Source File

SOURCE=.\BitmapText.cpp
# End Source File
# Begin Source File

SOURCE=.\BitmapText.h
# End Source File
# Begin Source File

SOURCE=.\Model.cpp
# End Source File
# Begin Source File

SOURCE=.\Model.h
# End Source File
# Begin Source File

SOURCE=.\ModelManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ModelManager.h
# End Source File
# Begin Source File

SOURCE=.\ModelTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\ModelTypes.h
# End Source File
# Begin Source File

SOURCE=.\Quad.cpp
# End Source File
# Begin Source File

SOURCE=.\Quad.h
# End Source File
# Begin Source File

SOURCE=.\RollingNumbers.cpp
# End Source File
# Begin Source File

SOURCE=.\RollingNumbers.h
# End Source File
# Begin Source File

SOURCE=.\Sprite.cpp
# End Source File
# Begin Source File

SOURCE=.\Sprite.h
# End Source File
# End Group
# Begin Group "Actors used in Gameplay & Menu"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Banner.cpp
# End Source File
# Begin Source File

SOURCE=.\Banner.h
# End Source File
# Begin Source File

SOURCE=.\BGAnimation.cpp
# End Source File
# Begin Source File

SOURCE=.\BGAnimation.h
# End Source File
# Begin Source File

SOURCE=.\BGAnimationLayer.cpp
# End Source File
# Begin Source File

SOURCE=.\BGAnimationLayer.h
# End Source File
# Begin Source File

SOURCE=.\DifficultyIcon.cpp
# End Source File
# Begin Source File

SOURCE=.\DifficultyIcon.h
# End Source File
# Begin Source File

SOURCE=.\FadingBanner.cpp
# End Source File
# Begin Source File

SOURCE=.\FadingBanner.h
# End Source File
# Begin Source File

SOURCE=.\MeterDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\MeterDisplay.h
# End Source File
# Begin Source File

SOURCE=.\StreamDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\StreamDisplay.h
# End Source File
# Begin Source File

SOURCE=.\Transition.cpp
# End Source File
# Begin Source File

SOURCE=.\Transition.h
# End Source File
# End Group
# Begin Group "Actors used in Gameplay"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ActiveAttackList.cpp
# End Source File
# Begin Source File

SOURCE=.\ActiveAttackList.h
# End Source File
# Begin Source File

SOURCE=.\ArrowEffects.cpp
# End Source File
# Begin Source File

SOURCE=.\ArrowEffects.h
# End Source File
# Begin Source File

SOURCE=.\AttackDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\AttackDisplay.h
# End Source File
# Begin Source File

SOURCE=.\Background.cpp
# End Source File
# Begin Source File

SOURCE=.\Background.h
# End Source File
# Begin Source File

SOURCE=.\BeginnerHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\BeginnerHelper.h
# End Source File
# Begin Source File

SOURCE=.\CombinedLifeMeterTug.cpp
# End Source File
# Begin Source File

SOURCE=.\CombinedLifeMeterTug.h
# End Source File
# Begin Source File

SOURCE=.\Combo.cpp
# End Source File
# Begin Source File

SOURCE=.\Combo.h
# End Source File
# Begin Source File

SOURCE=.\DancingCharacters.cpp
# End Source File
# Begin Source File

SOURCE=.\DancingCharacters.h
# End Source File
# Begin Source File

SOURCE=.\Foreground.cpp
# End Source File
# Begin Source File

SOURCE=.\Foreground.h
# End Source File
# Begin Source File

SOURCE=.\GhostArrow.cpp
# End Source File
# Begin Source File

SOURCE=.\GhostArrow.h
# End Source File
# Begin Source File

SOURCE=.\GhostArrowRow.cpp
# End Source File
# Begin Source File

SOURCE=.\GhostArrowRow.h
# End Source File
# Begin Source File

SOURCE=.\HoldGhostArrow.cpp
# End Source File
# Begin Source File

SOURCE=.\HoldGhostArrow.h
# End Source File
# Begin Source File

SOURCE=.\HoldJudgment.cpp
# End Source File
# Begin Source File

SOURCE=.\HoldJudgment.h
# End Source File
# Begin Source File

SOURCE=.\Judgment.cpp
# End Source File
# Begin Source File

SOURCE=.\Judgment.h
# End Source File
# Begin Source File

SOURCE=.\LifeMeter.h
# End Source File
# Begin Source File

SOURCE=.\LifeMeterBar.cpp
# End Source File
# Begin Source File

SOURCE=.\LifeMeterBar.h
# End Source File
# Begin Source File

SOURCE=.\LifeMeterBattery.cpp
# End Source File
# Begin Source File

SOURCE=.\LifeMeterBattery.h
# End Source File
# Begin Source File

SOURCE=.\LifeMeterTime.cpp
# End Source File
# Begin Source File

SOURCE=.\LifeMeterTime.h
# End Source File
# Begin Source File

SOURCE=.\LyricDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\LyricDisplay.h
# End Source File
# Begin Source File

SOURCE=.\NoteDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteDisplay.h
# End Source File
# Begin Source File

SOURCE=.\NoteField.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteField.h
# End Source File
# Begin Source File

SOURCE=.\PercentageDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\PercentageDisplay.h
# End Source File
# Begin Source File

SOURCE=.\Player.cpp
# End Source File
# Begin Source File

SOURCE=.\Player.h
# End Source File
# Begin Source File

SOURCE=.\ReceptorArrow.cpp
# End Source File
# Begin Source File

SOURCE=.\ReceptorArrow.h
# End Source File
# Begin Source File

SOURCE=.\ReceptorArrowRow.cpp
# End Source File
# Begin Source File

SOURCE=.\ReceptorArrowRow.h
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplay.h
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayAliveTime.cpp
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayAliveTime.h
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayBattle.cpp
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayBattle.h
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayCalories.cpp
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayCalories.h
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayLifeTime.cpp
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayLifeTime.h
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayNormal.cpp
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayNormal.h
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayOni.cpp
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayOni.h
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayPercentage.cpp
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayPercentage.h
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayRave.cpp
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayRave.h
# End Source File
# End Group
# Begin Group "Actors used in Menus"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BPMDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\BPMDisplay.h
# End Source File
# Begin Source File

SOURCE=.\ComboGraph.cpp
# End Source File
# Begin Source File

SOURCE=.\ComboGraph.h
# End Source File
# Begin Source File

SOURCE=.\CourseContentsList.cpp
# End Source File
# Begin Source File

SOURCE=.\CourseContentsList.h
# End Source File
# Begin Source File

SOURCE=.\CourseEntryDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\CourseEntryDisplay.h
# End Source File
# Begin Source File

SOURCE=.\DifficultyDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\DifficultyDisplay.h
# End Source File
# Begin Source File

SOURCE=.\DifficultyList.cpp
# End Source File
# Begin Source File

SOURCE=.\DifficultyList.h
# End Source File
# Begin Source File

SOURCE=.\DifficultyMeter.cpp
# End Source File
# Begin Source File

SOURCE=.\DifficultyMeter.h
# End Source File
# Begin Source File

SOURCE=.\DifficultyRating.cpp
# End Source File
# Begin Source File

SOURCE=.\DifficultyRating.h
# End Source File
# Begin Source File

SOURCE=.\DualScrollBar.cpp
# End Source File
# Begin Source File

SOURCE=.\DualScrollBar.h
# End Source File
# Begin Source File

SOURCE=.\EditCoursesMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\EditCoursesMenu.h
# End Source File
# Begin Source File

SOURCE=.\EditCoursesSongMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\EditCoursesSongMenu.h
# End Source File
# Begin Source File

SOURCE=.\EditMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\EditMenu.h
# End Source File
# Begin Source File

SOURCE=.\GradeDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\GradeDisplay.h
# End Source File
# Begin Source File

SOURCE=.\GraphDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\GraphDisplay.h
# End Source File
# Begin Source File

SOURCE=.\GrooveGraph.cpp
# End Source File
# Begin Source File

SOURCE=.\GrooveGraph.h
# End Source File
# Begin Source File

SOURCE=.\GrooveRadar.cpp
# End Source File
# Begin Source File

SOURCE=.\GrooveRadar.h
# End Source File
# Begin Source File

SOURCE=.\GroupList.cpp
# End Source File
# Begin Source File

SOURCE=.\GroupList.h
# End Source File
# Begin Source File

SOURCE=.\HelpDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\HelpDisplay.h
# End Source File
# Begin Source File

SOURCE=.\MemoryCardDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\MemoryCardDisplay.h
# End Source File
# Begin Source File

SOURCE=.\MenuTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuTimer.h
# End Source File
# Begin Source File

SOURCE=.\ModeSwitcher.cpp
# End Source File
# Begin Source File

SOURCE=.\ModeSwitcher.h
# End Source File
# Begin Source File

SOURCE=.\MusicBannerWheel.cpp
# End Source File
# Begin Source File

SOURCE=.\MusicBannerWheel.h
# End Source File
# Begin Source File

SOURCE=.\MusicList.cpp
# End Source File
# Begin Source File

SOURCE=.\MusicList.h
# End Source File
# Begin Source File

SOURCE=.\MusicSortDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\MusicSortDisplay.h
# End Source File
# Begin Source File

SOURCE=.\MusicWheel.cpp
# End Source File
# Begin Source File

SOURCE=.\MusicWheel.h
# End Source File
# Begin Source File

SOURCE=.\MusicWheelItem.cpp
# End Source File
# Begin Source File

SOURCE=.\MusicWheelItem.h
# End Source File
# Begin Source File

SOURCE=.\OptionIcon.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionIcon.h
# End Source File
# Begin Source File

SOURCE=.\OptionIconRow.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionIconRow.h
# End Source File
# Begin Source File

SOURCE=.\OptionRow.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionRow.h
# End Source File
# Begin Source File

SOURCE=.\OptionsCursor.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsCursor.h
# End Source File
# Begin Source File

SOURCE=.\PaneDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\PaneDisplay.h
# End Source File
# Begin Source File

SOURCE=.\RoomWheel.cpp
# End Source File
# Begin Source File

SOURCE=.\RoomWheel.h
# End Source File
# Begin Source File

SOURCE=.\ScrollBar.cpp
# End Source File
# Begin Source File

SOURCE=.\ScrollBar.h
# End Source File
# Begin Source File

SOURCE=.\ScrollingList.cpp
# End Source File
# Begin Source File

SOURCE=.\ScrollingList.h
# End Source File
# Begin Source File

SOURCE=.\SnapDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\SnapDisplay.h
# End Source File
# Begin Source File

SOURCE=.\TextBanner.cpp
# End Source File
# Begin Source File

SOURCE=.\TextBanner.h
# End Source File
# Begin Source File

SOURCE=.\WheelBase.cpp
# End Source File
# Begin Source File

SOURCE=.\WheelBase.h
# End Source File
# Begin Source File

SOURCE=.\WheelItemBase.cpp
# End Source File
# Begin Source File

SOURCE=.\WheelItemBase.h
# End Source File
# Begin Source File

SOURCE=.\WheelNotifyIcon.cpp
# End Source File
# Begin Source File

SOURCE=.\WheelNotifyIcon.h
# End Source File
# End Group
# Begin Group "Screens"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Screen.cpp
# End Source File
# Begin Source File

SOURCE=.\Screen.h
# End Source File
# Begin Source File

SOURCE=.\ScreenAttract.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenAttract.h
# End Source File
# Begin Source File

SOURCE=.\ScreenBookkeeping.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenBookkeeping.h
# End Source File
# Begin Source File

SOURCE=.\ScreenBranch.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenBranch.h
# End Source File
# Begin Source File

SOURCE=.\ScreenCenterImage.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenCenterImage.h
# End Source File
# Begin Source File

SOURCE=.\ScreenCredits.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenCredits.h
# End Source File
# Begin Source File

SOURCE=.\ScreenDebugOverlay.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenDebugOverlay.h
# End Source File
# Begin Source File

SOURCE=.\ScreenDemonstration.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenDemonstration.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenEdit.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEditCoursesMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenEditCoursesMenu.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEditMenu.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

# ADD BASE CPP /YX"global.h"
# ADD CPP /YX"global.h"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenEditMenu.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEnding.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenEnding.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEndlessBreak.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenEndlessBreak.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEvaluation.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenEvaluation.h
# End Source File
# Begin Source File

SOURCE=.\ScreenExit.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenExit.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEz2SelectMusic.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenEz2SelectMusic.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEz2SelectPlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenEz2SelectPlayer.h
# End Source File
# Begin Source File

SOURCE=.\ScreenGameplay.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenGameplay.h
# End Source File
# Begin Source File

SOURCE=.\ScreenGameplayMultiplayer.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenGameplayMultiplayer.h
# End Source File
# Begin Source File

SOURCE=.\ScreenHowToPlay.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenHowToPlay.h
# End Source File
# Begin Source File

SOURCE=.\ScreenInstructions.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenInstructions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenJukebox.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenJukebox.h
# End Source File
# Begin Source File

SOURCE=.\ScreenLogo.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenLogo.h
# End Source File
# Begin Source File

SOURCE=.\ScreenMapControllers.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenMapControllers.h
# End Source File
# Begin Source File

SOURCE=.\ScreenMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenMessage.h
# End Source File
# Begin Source File

SOURCE=.\ScreenMiniMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenMiniMenu.h
# End Source File
# Begin Source File

SOURCE=.\ScreenMusicScroll.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenMusicScroll.h
# End Source File
# Begin Source File

SOURCE=.\ScreenNameEntry.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenNameEntry.h
# End Source File
# Begin Source File

SOURCE=.\ScreenNameEntryTraditional.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenNameEntryTraditional.h
# End Source File
# Begin Source File

SOURCE=.\ScreenNetEvaluation.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenNetEvaluation.h
# End Source File
# Begin Source File

SOURCE=.\ScreenNetRoom.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenNetRoom.h
# End Source File
# Begin Source File

SOURCE=.\ScreenNetSelectBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenNetSelectBase.h
# End Source File
# Begin Source File

SOURCE=.\ScreenNetSelectMusic.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenNetSelectMusic.h
# End Source File
# Begin Source File

SOURCE=.\ScreenNetworkOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenNetworkOptions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenOptions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenOptionsMaster.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenOptionsMaster.h
# End Source File
# Begin Source File

SOURCE=.\ScreenOptionsMasterPrefs.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenOptionsMasterPrefs.h
# End Source File
# Begin Source File

SOURCE=.\ScreenPackages.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenPackages.h
# End Source File
# Begin Source File

SOURCE=.\ScreenPlayerOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenPlayerOptions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenProfileOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenProfileOptions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenPrompt.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenPrompt.h
# End Source File
# Begin Source File

SOURCE=.\ScreenRanking.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenRanking.h
# End Source File
# Begin Source File

SOURCE=.\ScreenReloadSongs.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenReloadSongs.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSandbox.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSandbox.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSaveSync.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSaveSync.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelect.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSelect.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectCharacter.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectCharacter.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectDifficulty.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectDifficulty.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectGroup.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectGroup.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectMaster.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectMaster.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectMode.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectMode.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectMusic.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectMusic.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectStyle.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectStyle.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSetTime.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSetTime.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSMOnlineLogin.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSMOnlineLogin.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSongOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSongOptions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSplash.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSplash.h
# End Source File
# Begin Source File

SOURCE=.\ScreenStage.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenStage.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSyncOverlay.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSyncOverlay.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSystemLayer.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSystemLayer.h
# End Source File
# Begin Source File

SOURCE=.\ScreenTest.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenTest.h
# End Source File
# Begin Source File

SOURCE=.\ScreenTestFonts.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenTestFonts.h
# End Source File
# Begin Source File

SOURCE=.\ScreenTestInput.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenTestInput.h
# End Source File
# Begin Source File

SOURCE=.\ScreenTestLights.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenTestLights.h
# End Source File
# Begin Source File

SOURCE=.\ScreenTestSound.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenTestSound.h
# End Source File
# Begin Source File

SOURCE=.\ScreenTextEntry.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenTextEntry.h
# End Source File
# Begin Source File

SOURCE=.\ScreenTitleMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenTitleMenu.h
# End Source File
# Begin Source File

SOURCE=.\ScreenUnlock.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenUnlock.h
# End Source File
# Begin Source File

SOURCE=.\ScreenWithMenuElements.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenWithMenuElements.h
# End Source File
# End Group
# Begin Group "Global Singletons"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AnnouncerManager.cpp
# End Source File
# Begin Source File

SOURCE=.\AnnouncerManager.h
# End Source File
# Begin Source File

SOURCE=.\Bookkeeper.cpp
# End Source File
# Begin Source File

SOURCE=.\Bookkeeper.h
# End Source File
# Begin Source File

SOURCE=.\CryptManager.cpp
# End Source File
# Begin Source File

SOURCE=.\CryptManager.h
# End Source File
# Begin Source File

SOURCE=.\ezsockets.cpp
# End Source File
# Begin Source File

SOURCE=.\ezsockets.h
# End Source File
# Begin Source File

SOURCE=.\FontManager.cpp
# End Source File
# Begin Source File

SOURCE=.\FontManager.h
# End Source File
# Begin Source File

SOURCE=.\GameManager.cpp
# End Source File
# Begin Source File

SOURCE=.\GameManager.h
# End Source File
# Begin Source File

SOURCE=.\GameSoundManager.cpp
# End Source File
# Begin Source File

SOURCE=.\GameSoundManager.h
# End Source File
# Begin Source File

SOURCE=.\GameState.cpp
# End Source File
# Begin Source File

SOURCE=.\GameState.h
# End Source File
# Begin Source File

SOURCE=.\InputFilter.cpp
# End Source File
# Begin Source File

SOURCE=.\InputFilter.h
# End Source File
# Begin Source File

SOURCE=.\InputMapper.cpp
# End Source File
# Begin Source File

SOURCE=.\InputMapper.h
# End Source File
# Begin Source File

SOURCE=.\InputQueue.cpp
# End Source File
# Begin Source File

SOURCE=.\InputQueue.h
# End Source File
# Begin Source File

SOURCE=.\LightsManager.cpp
# End Source File
# Begin Source File

SOURCE=.\LightsManager.h
# End Source File
# Begin Source File

SOURCE=.\LuaManager.cpp
# End Source File
# Begin Source File

SOURCE=.\LuaManager.h
# End Source File
# Begin Source File

SOURCE=.\MemoryCardManager.cpp
# End Source File
# Begin Source File

SOURCE=.\MemoryCardManager.h
# End Source File
# Begin Source File

SOURCE=.\MessageManager.cpp
# End Source File
# Begin Source File

SOURCE=.\MessageManager.h
# End Source File
# Begin Source File

SOURCE=.\NetworkSyncManager.cpp
# End Source File
# Begin Source File

SOURCE=.\NetworkSyncManager.h
# End Source File
# Begin Source File

SOURCE=.\NetworkSyncServer.cpp
# End Source File
# Begin Source File

SOURCE=.\NetworkSyncServer.h
# End Source File
# Begin Source File

SOURCE=.\NoteSkinManager.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteSkinManager.h
# End Source File
# Begin Source File

SOURCE=.\PrefsManager.cpp
# End Source File
# Begin Source File

SOURCE=.\PrefsManager.h
# End Source File
# Begin Source File

SOURCE=.\ProfileManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ProfileManager.h
# End Source File
# Begin Source File

SOURCE=.\ScreenManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenManager.h
# End Source File
# Begin Source File

SOURCE=.\SongManager.cpp
# End Source File
# Begin Source File

SOURCE=.\SongManager.h
# End Source File
# Begin Source File

SOURCE=.\StatsManager.cpp
# End Source File
# Begin Source File

SOURCE=.\StatsManager.h
# End Source File
# Begin Source File

SOURCE=.\ThemeManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ThemeManager.h
# End Source File
# Begin Source File

SOURCE=.\UnlockManager.cpp
# End Source File
# Begin Source File

SOURCE=.\UnlockManager.h
# End Source File
# End Group
# Begin Group "Crypto"

# PROP Default_Filter ""
# Begin Group "crypto++"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\crypto51\algebra.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto51\algebra.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\algparam.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto51\algparam.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\argnames.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\asn.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto51\asn.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\config.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\cryptlib.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto51\cryptlib.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\files.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto51\files.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\filters.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto51\filters.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\fltrimpl.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\integer.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto51\integer.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\iterhash.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto51\iterhash.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\mdc.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\misc.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto51\misc.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\modarith.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\mqueue.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto51\mqueue.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\nbtheory.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto51\nbtheory.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\oids.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\osrng.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto51\osrng.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\pch.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\pkcspad.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto51\pkcspad.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\pubkey.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto51\pubkey.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\queue.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto51\queue.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\rng.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\rsa.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto51\rsa.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\secblock.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\seckey.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\sha.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto51\sha.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\simple.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\smartptr.h
# End Source File
# Begin Source File

SOURCE=.\crypto51\words.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\crypto\CryptBn.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto\CryptBn.h
# End Source File
# Begin Source File

SOURCE=.\CryptHelpers.cpp
# End Source File
# Begin Source File

SOURCE=.\CryptHelpers.h
# End Source File
# Begin Source File

SOURCE=.\crypto\CryptMD5.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto\CryptMD5.h
# End Source File
# Begin Source File

SOURCE=.\crypto\CryptNoise.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto\CryptPrime.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto\CryptPrime.h
# End Source File
# Begin Source File

SOURCE=.\crypto\CryptRand.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto\CryptRand.h
# End Source File
# Begin Source File

SOURCE=.\crypto\CryptRSA.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto\CryptRSA.h
# End Source File
# Begin Source File

SOURCE=.\crypto\CryptSH512.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto\CryptSH512.h
# End Source File
# Begin Source File

SOURCE=.\crypto\CryptSHA.cpp
# End Source File
# Begin Source File

SOURCE=.\crypto\CryptSHA.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\error.bmp
# End Source File
# Begin Source File

SOURCE=.\error2.bmp
# End Source File
# Begin Source File

SOURCE=.\loading.bmp
# End Source File
# Begin Source File

SOURCE=.\StepMania.xpm
# End Source File
# End Target
# End Project
