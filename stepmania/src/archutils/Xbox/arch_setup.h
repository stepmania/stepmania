#ifndef ARCH_SETUP_XBOX_H
#define ARCH_SETUP_XBOX_H

#include <xtl.h>
#include <xgraphics.h>
#include <stdio.h>

#if defined(_XDBG)
#include <xbdm.h>
#define IsDebuggerPresent() DmIsDebuggerPresent()
#endif

// Xbox base path
#define SYS_BASE_PATH "D:\\"

#define SUPPORT_D3D

/* Stubs: */
inline HRESULT CoInitialize( LPVOID pvReserved ) { return S_OK; }
inline void CoUninitialize() { }

extern "C" int SM_main( int argc, char *argv[] );
#define main(x,y) SM_main(x,y)

#endif
