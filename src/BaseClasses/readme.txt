DirectShow Sample -- Base Classes
---------------------------------

The Microsoft DirectShow base classes are a set of C++ classes 
and utility functions that you can use to implement DirectShow filters. 
For complete documentation of the base classes, see "DirectShow Base Classes" 
in the DirectShow Reference section of the DirectX SDK documentation.

NOTE: The BaseClasses header file schedule.h has been renamed to dsschedule.h
to prevent conflicts with the Platform SDK <schedule.h>.  Only the refclock.h
header in the BaseClasses directory uses this header, so the impact should
be minimal.

Building for Windows XP
-----------------------
If you want to target Windows XP specifically to use its new features, 
you must set WINVER=0x501 in the BaseClasses project file.  You must also 
install the Windows XP Platform SDK, however, to ensure that you have the 
latest header files.  For example, wxutil.cpp uses the new TIME_KILL_SYNCHRONOUS flag 
only if (WINVER >= 0x501).  This flag is conditionally defined in the Windows XP 
Platform SDK in mmsystem.h, but only if WINVER is also set to 0x501 when compiling.

	#if (WINVER >= 0x501)
	#define TIME_KILL_SYNCHRONOUS	0x0100
	#endif
