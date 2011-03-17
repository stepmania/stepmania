//------------------------------------------------------------------------------
// File: Streams.h
//
// Desc: DirectShow base classes - defines overall streams architecture.
//
// Copyright (c) 1992-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#ifndef __STREAMS__
#define __STREAMS__

// disable some level-4 warnings, use #pragma warning(enable:###) to re-enable
#pragma warning(disable:4100) // warning C4100: unreferenced formal parameter
#pragma warning(disable:4127) // warning C4127: conditional expression is constant
#pragma warning(disable:4189) // warning C4189: local variable is initialized but not referenced
#pragma warning(disable:4201) // warning C4201: nonstandard extension used : nameless struct/union
#pragma warning(disable:4511) // warning C4511: copy constructor could not be generated
#pragma warning(disable:4512) // warning C4512: assignment operator could not be generated
#pragma warning(disable:4514) // warning C4514: unreferenced inline function has been removed
#pragma warning(disable:4710) // warning C4710: 'function' not inlined

#ifdef  _MSC_VER
#if _MSC_VER>=1100
#define AM_NOVTABLE __declspec(novtable)
#else
#define AM_NOVTABLE
#endif
#endif  // MSC_VER


#include <windows.h>
#include <windowsx.h>
#include <olectl.h>
#include <ddraw.h>
#include <mmsystem.h>

#ifndef NUMELMS
   #define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#endif

///////////////////////////////////////////////////////////////////////////
// The following definitions come from the Platform SDK and are required if
// the applicaiton is being compiled with the headers from Visual C++ 6.0.
///////////////////////////////////////////////////////////////////////////
#ifndef InterlockedExchangePointer
    #define InterlockedExchangePointer(Target, Value) \
   (PVOID)InterlockedExchange((PLONG)(Target), (LONG)(Value))
#endif

#ifndef _WAVEFORMATEXTENSIBLE_
#define _WAVEFORMATEXTENSIBLE_
typedef struct {
    WAVEFORMATEX    Format;
    union {
        WORD wValidBitsPerSample;       /* bits of precision  */
        WORD wSamplesPerBlock;          /* valid if wBitsPerSample==0 */
        WORD wReserved;                 /* If neither applies, set to zero. */
    } Samples;
    DWORD           dwChannelMask;      /* which channels are */
                                        /* present in stream  */
    GUID            SubFormat;
} WAVEFORMATEXTENSIBLE, *PWAVEFORMATEXTENSIBLE;
#endif // !_WAVEFORMATEXTENSIBLE_

#if !defined(WAVE_FORMAT_EXTENSIBLE)
#define  WAVE_FORMAT_EXTENSIBLE                 0xFFFE
#endif // !defined(WAVE_FORMAT_EXTENSIBLE)

#ifndef GetWindowLongPtr
  #define GetWindowLongPtrA   GetWindowLongA
  #define GetWindowLongPtrW   GetWindowLongW
  #ifdef UNICODE
    #define GetWindowLongPtr  GetWindowLongPtrW
  #else
    #define GetWindowLongPtr  GetWindowLongPtrA
  #endif // !UNICODE
#endif // !GetWindowLongPtr

#ifndef SetWindowLongPtr
  #define SetWindowLongPtrA   SetWindowLongA
  #define SetWindowLongPtrW   SetWindowLongW
  #ifdef UNICODE
    #define SetWindowLongPtr  SetWindowLongPtrW
  #else
    #define SetWindowLongPtr  SetWindowLongPtrA
  #endif // !UNICODE
#endif // !SetWindowLongPtr

#ifndef GWLP_WNDPROC
  #define GWLP_WNDPROC        (-4)
#endif
#ifndef GWLP_HINSTANCE
  #define GWLP_HINSTANCE      (-6)
#endif
#ifndef GWLP_HWNDPARENT
  #define GWLP_HWNDPARENT     (-8)
#endif
#ifndef GWLP_USERDATA
  #define GWLP_USERDATA       (-21)
#endif
#ifndef GWLP_ID
  #define GWLP_ID             (-12)
#endif
#ifndef DWLP_MSGRESULT
  #define DWLP_MSGRESULT  0
#endif
#ifndef DWLP_DLGPROC 
  #define DWLP_DLGPROC    DWLP_MSGRESULT + sizeof(LRESULT)
#endif
#ifndef DWLP_USER
  #define DWLP_USER       DWLP_DLGPROC + sizeof(DLGPROC)
#endif
///////////////////////////////////////////////////////////////////////////
// End Platform SDK definitions
///////////////////////////////////////////////////////////////////////////


#pragma warning(disable:4201) // warning C4201: nonstandard extension used : nameless struct/union
#include <strmif.h>     // Generated IDL header file for streams interfaces

//
// Modified by Chris to look in local baseclasses directory instead of include path
//
#include "baseclasses/reftime.h"    // Helper class for REFERENCE_TIME management
#include "baseclasses/wxdebug.h"    // Debug support for logging and ASSERTs
#include "baseclasses/amvideo.h"    // ActiveMovie video interfaces and definitions

//include amaudio.h explicitly if you need it.  it requires the DX SDK.
//#include <amaudio.h>    // ActiveMovie audio interfaces and definitions

#include "baseclasses/wxutil.h"     // General helper classes for threads etc
#include "baseclasses/combase.h"    // Base COM classes to support IUnknown
#include "baseclasses/dllsetup.h"   // Filter registration support functions
#include "baseclasses/measure.h"    // Performance measurement
#include <comlite.h>    // Light weight com function prototypes

#include "baseclasses/cache.h"      // Simple cache container class
#include "baseclasses/wxlist.h"     // Non MFC generic list class
#include "baseclasses/msgthrd.h"    // CMsgThread
#include "baseclasses/mtype.h"      // Helper class for managing media types
#include "baseclasses/fourcc.h"     // conversions between FOURCCs and GUIDs
#include <control.h>    // generated from control.odl
#include "baseclasses/ctlutil.h"    // control interface utility classes
#include <evcode.h>     // event code definitions
#include "baseclasses/amfilter.h"   // Main streams architecture class hierachy
#include "baseclasses/transfrm.h"   // Generic transform filter
#include "baseclasses/transip.h"    // Generic transform-in-place filter
#include <uuids.h>      // declaration of type GUIDs and well-known clsids
#include "baseclasses/source.h" // Generic source filter
#include "baseclasses/outputq.h"    // Output pin queueing
#include <errors.h>     // HRESULT status and error definitions
#include "baseclasses/renbase.h"    // Base class for writing ActiveX renderers
#include "baseclasses/winutil.h"    // Helps with filters that manage windows
#include "baseclasses/winctrl.h"    // Implements the IVideoWindow interface
#include "baseclasses/videoctl.h"   // Specifically video related classes
#include "baseclasses/refclock.h"   // Base clock class
#include "baseclasses/sysclock.h"   // System clock
#include "baseclasses/pstream.h"    // IPersistStream helper class
#include "baseclasses/vtrans.h"     // Video Transform Filter base class
#include "baseclasses/amextra.h"
#include "baseclasses/cprop.h"      // Base property page class
#include "baseclasses/strmctl.h"    // IAMStreamControl support
#include <edevdefs.h>   // External device control interface defines
#include <audevcod.h>   // audio filter device error event codes

#else
    #ifdef DEBUG
    #pragma message("STREAMS.H included TWICE")
    #endif
#endif // __STREAMS__

