#ifndef ARCH_H
#define ARCH_H

#include "RageTextureID.h"

// Put renderers switch here, makes things cleaner
#if defined(_WINDOWS)
#define SUPPORT_OPENGL
#define SUPPORT_D3D
#elif defined(_XBOX)
#define SUPPORT_D3D
#else
#define SUPPORT_OPENGL
#endif

/* Include this file if you need to create an instance of a driver object.  */
class ArchHooks;
ArchHooks *MakeArchHooks();

class DialogDriver;
DialogDriver *MakeDialogDriver();

class InputHandler;
void MakeInputHandlers(CString drivers, vector<InputHandler *> &Add);

class LightsDriver;
void MakeLightsDrivers(CString drivers, vector<LightsDriver *> &Add);

class LoadingWindow;
LoadingWindow *MakeLoadingWindow();

#if defined(SUPPORT_OPENGL)
class LowLevelWindow;
LowLevelWindow *MakeLowLevelWindow();
#endif

class MemoryCardDriver;
MemoryCardDriver *MakeMemoryCardDriver();

class RageMovieTexture;
RageMovieTexture *MakeRageMovieTexture(RageTextureID ID);

class RageSoundDriver;
RageSoundDriver *MakeRageSoundDriver(CString drivers);

#endif

/*
 * (c) 2002-2005 Glenn Maynard, Ben Anderson
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
