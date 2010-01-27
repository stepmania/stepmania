# Microsoft Developer Studio Generated NMAKE File, Based on baseclasses.dsp
!IF "$(CFG)" == ""
CFG=BaseClasses - Win32 Debug
!MESSAGE No configuration specified. Defaulting to BaseClasses - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "BaseClasses - Win32 Release" && "$(CFG)" != "BaseClasses - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "baseclasses.mak" CFG="BaseClasses - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "BaseClasses - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "BaseClasses - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "BaseClasses - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\STRMBASE.lib"


CLEAN :
	-@erase "$(INTDIR)\amextra.obj"
	-@erase "$(INTDIR)\amfilter.obj"
	-@erase "$(INTDIR)\amvideo.obj"
	-@erase "$(INTDIR)\baseclasses.pch"
	-@erase "$(INTDIR)\combase.obj"
	-@erase "$(INTDIR)\cprop.obj"
	-@erase "$(INTDIR)\ctlutil.obj"
	-@erase "$(INTDIR)\ddmm.obj"
	-@erase "$(INTDIR)\dllentry.obj"
	-@erase "$(INTDIR)\dllsetup.obj"
	-@erase "$(INTDIR)\mtype.obj"
	-@erase "$(INTDIR)\outputq.obj"
	-@erase "$(INTDIR)\pstream.obj"
	-@erase "$(INTDIR)\pullpin.obj"
	-@erase "$(INTDIR)\refclock.obj"
	-@erase "$(INTDIR)\renbase.obj"
	-@erase "$(INTDIR)\schedule.obj"
	-@erase "$(INTDIR)\seekpt.obj"
	-@erase "$(INTDIR)\source.obj"
	-@erase "$(INTDIR)\strmctl.obj"
	-@erase "$(INTDIR)\sysclock.obj"
	-@erase "$(INTDIR)\transfrm.obj"
	-@erase "$(INTDIR)\transip.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\videoctl.obj"
	-@erase "$(INTDIR)\vtrans.obj"
	-@erase "$(INTDIR)\winctrl.obj"
	-@erase "$(INTDIR)\winutil.obj"
	-@erase "$(INTDIR)\wxdebug.obj"
	-@erase "$(INTDIR)\wxlist.obj"
	-@erase "$(INTDIR)\wxutil.obj"
	-@erase "$(OUTDIR)\STRMBASE.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /Gi /GX /O2 /I "." /I "..\..\..\..\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WIN32_DCOM" /Fp"$(INTDIR)\baseclasses.pch" /Yu"streams.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\baseclasses.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=..\..\..\..\lib\strmiids.lib /nologo /out:"$(OUTDIR)\STRMBASE.lib" /nodefaultlib 
LIB32_OBJS= \
	"$(INTDIR)\amextra.obj" \
	"$(INTDIR)\amfilter.obj" \
	"$(INTDIR)\amvideo.obj" \
	"$(INTDIR)\combase.obj" \
	"$(INTDIR)\cprop.obj" \
	"$(INTDIR)\ctlutil.obj" \
	"$(INTDIR)\ddmm.obj" \
	"$(INTDIR)\dllentry.obj" \
	"$(INTDIR)\dllsetup.obj" \
	"$(INTDIR)\mtype.obj" \
	"$(INTDIR)\outputq.obj" \
	"$(INTDIR)\pstream.obj" \
	"$(INTDIR)\pullpin.obj" \
	"$(INTDIR)\refclock.obj" \
	"$(INTDIR)\renbase.obj" \
	"$(INTDIR)\schedule.obj" \
	"$(INTDIR)\seekpt.obj" \
	"$(INTDIR)\source.obj" \
	"$(INTDIR)\strmctl.obj" \
	"$(INTDIR)\sysclock.obj" \
	"$(INTDIR)\transfrm.obj" \
	"$(INTDIR)\transip.obj" \
	"$(INTDIR)\videoctl.obj" \
	"$(INTDIR)\vtrans.obj" \
	"$(INTDIR)\winctrl.obj" \
	"$(INTDIR)\winutil.obj" \
	"$(INTDIR)\wxdebug.obj" \
	"$(INTDIR)\wxlist.obj" \
	"$(INTDIR)\wxutil.obj"

"$(OUTDIR)\STRMBASE.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "BaseClasses - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\strmbasd.lib"


CLEAN :
	-@erase "$(INTDIR)\amextra.obj"
	-@erase "$(INTDIR)\amfilter.obj"
	-@erase "$(INTDIR)\amvideo.obj"
	-@erase "$(INTDIR)\baseclasses.pch"
	-@erase "$(INTDIR)\combase.obj"
	-@erase "$(INTDIR)\cprop.obj"
	-@erase "$(INTDIR)\ctlutil.obj"
	-@erase "$(INTDIR)\ddmm.obj"
	-@erase "$(INTDIR)\dllentry.obj"
	-@erase "$(INTDIR)\dllsetup.obj"
	-@erase "$(INTDIR)\mtype.obj"
	-@erase "$(INTDIR)\outputq.obj"
	-@erase "$(INTDIR)\pstream.obj"
	-@erase "$(INTDIR)\pullpin.obj"
	-@erase "$(INTDIR)\refclock.obj"
	-@erase "$(INTDIR)\renbase.obj"
	-@erase "$(INTDIR)\schedule.obj"
	-@erase "$(INTDIR)\seekpt.obj"
	-@erase "$(INTDIR)\source.obj"
	-@erase "$(INTDIR)\strmctl.obj"
	-@erase "$(INTDIR)\sysclock.obj"
	-@erase "$(INTDIR)\transfrm.obj"
	-@erase "$(INTDIR)\transip.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\videoctl.obj"
	-@erase "$(INTDIR)\vtrans.obj"
	-@erase "$(INTDIR)\winctrl.obj"
	-@erase "$(INTDIR)\winutil.obj"
	-@erase "$(INTDIR)\wxdebug.obj"
	-@erase "$(INTDIR)\wxlist.obj"
	-@erase "$(INTDIR)\wxutil.obj"
	-@erase "$(OUTDIR)\strmbasd.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /Gz /MTd /W3 /Gm /Gi /GX /Zi /Od /I "." /I "..\..\..\..\include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WIN32_DCOM" /D "DEBUG" /Fp"$(INTDIR)\baseclasses.pch" /Yu"streams.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\baseclasses.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=..\..\..\..\lib\strmiids.lib /nologo /out:"$(OUTDIR)\strmbasd.lib" /nodefaultlib 
LIB32_OBJS= \
	"$(INTDIR)\amextra.obj" \
	"$(INTDIR)\amfilter.obj" \
	"$(INTDIR)\amvideo.obj" \
	"$(INTDIR)\combase.obj" \
	"$(INTDIR)\cprop.obj" \
	"$(INTDIR)\ctlutil.obj" \
	"$(INTDIR)\ddmm.obj" \
	"$(INTDIR)\dllentry.obj" \
	"$(INTDIR)\dllsetup.obj" \
	"$(INTDIR)\mtype.obj" \
	"$(INTDIR)\outputq.obj" \
	"$(INTDIR)\pstream.obj" \
	"$(INTDIR)\pullpin.obj" \
	"$(INTDIR)\refclock.obj" \
	"$(INTDIR)\renbase.obj" \
	"$(INTDIR)\schedule.obj" \
	"$(INTDIR)\seekpt.obj" \
	"$(INTDIR)\source.obj" \
	"$(INTDIR)\strmctl.obj" \
	"$(INTDIR)\sysclock.obj" \
	"$(INTDIR)\transfrm.obj" \
	"$(INTDIR)\transip.obj" \
	"$(INTDIR)\videoctl.obj" \
	"$(INTDIR)\vtrans.obj" \
	"$(INTDIR)\winctrl.obj" \
	"$(INTDIR)\winutil.obj" \
	"$(INTDIR)\wxdebug.obj" \
	"$(INTDIR)\wxlist.obj" \
	"$(INTDIR)\wxutil.obj"

"$(OUTDIR)\strmbasd.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("baseclasses.dep")
!INCLUDE "baseclasses.dep"
!ELSE 
!MESSAGE Warning: cannot find "baseclasses.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "BaseClasses - Win32 Release" || "$(CFG)" == "BaseClasses - Win32 Debug"
SOURCE=.\amextra.cpp

"$(INTDIR)\amextra.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\amfilter.cpp

"$(INTDIR)\amfilter.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\amvideo.cpp

"$(INTDIR)\amvideo.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\combase.cpp

"$(INTDIR)\combase.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\cprop.cpp

"$(INTDIR)\cprop.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\ctlutil.cpp

"$(INTDIR)\ctlutil.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\ddmm.cpp

"$(INTDIR)\ddmm.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\dllentry.cpp

!IF  "$(CFG)" == "BaseClasses - Win32 Release"

CPP_SWITCHES=/nologo /MT /W3 /Gi /GX /O2 /I "." /I "..\..\..\..\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WIN32_DCOM" /Fp"$(INTDIR)\baseclasses.pch" /Yc"streams.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\dllentry.obj"	"$(INTDIR)\baseclasses.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "BaseClasses - Win32 Debug"

CPP_SWITCHES=/nologo /Gz /MTd /W3 /Gm /Gi /GX /Zi /Od /I "." /I "..\..\..\..\include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WIN32_DCOM" /D "DEBUG" /Fp"$(INTDIR)\baseclasses.pch" /Yc"streams.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\dllentry.obj"	"$(INTDIR)\baseclasses.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\dllsetup.cpp

"$(INTDIR)\dllsetup.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\mtype.cpp

"$(INTDIR)\mtype.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\outputq.cpp

"$(INTDIR)\outputq.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\pstream.cpp

"$(INTDIR)\pstream.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\pullpin.cpp

"$(INTDIR)\pullpin.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\refclock.cpp

"$(INTDIR)\refclock.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\renbase.cpp

"$(INTDIR)\renbase.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\schedule.cpp

"$(INTDIR)\schedule.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\seekpt.cpp

"$(INTDIR)\seekpt.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\source.cpp

"$(INTDIR)\source.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\strmctl.cpp

"$(INTDIR)\strmctl.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\sysclock.cpp

"$(INTDIR)\sysclock.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\transfrm.cpp

"$(INTDIR)\transfrm.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\transip.cpp

"$(INTDIR)\transip.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\videoctl.cpp

"$(INTDIR)\videoctl.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\vtrans.cpp

"$(INTDIR)\vtrans.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\winctrl.cpp

"$(INTDIR)\winctrl.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\winutil.cpp

"$(INTDIR)\winutil.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\wxdebug.cpp

"$(INTDIR)\wxdebug.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\wxlist.cpp

"$(INTDIR)\wxlist.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"


SOURCE=.\wxutil.cpp

"$(INTDIR)\wxutil.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\baseclasses.pch"



!ENDIF 

