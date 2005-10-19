#ifndef RAGE_MOVIE_TEXTURE_DSHOW_H
#define RAGE_MOVIE_TEXTURE_DSHOW_H

#include "MovieTexture.h"

/* Don't know why we need this for the headers ... */
typedef char TCHAR, *PTCHAR;

/* Prevent these from using Dbg stuff, which we don't link in. */
#ifdef DEBUG
#undef DEBUG
#undef _DEBUG
#define GIVE_BACK_DEBUG
#endif

#include <atlbase.h>

#ifdef GIVE_BACK_DEBUG
#undef GIVE_BACK_DEBUG
#define _DEBUG
#define DEBUG
#endif

#include "baseclasses/streams.h"

#include "RageDisplay.h"
#include "RageTexture.h"
#include "RageThreads.h"

//-----------------------------------------------------------------------------
// RageMovieTexture Class Declarations
//-----------------------------------------------------------------------------
class MovieTexture_DShow : public RageMovieTexture
{
public:
	MovieTexture_DShow( RageTextureID ID );
	virtual ~MovieTexture_DShow();
	CString Init();

	/* only called by RageTextureManager::InvalidateTextures */
	void Invalidate() { m_uTexHandle = 0; }
	void Update(float fDeltaTime);

	virtual void Reload();

	virtual void Play();
	virtual void Pause();
	virtual void SetPosition( float fSeconds );
	virtual void SetPlaybackRate( float fRate );

	void SetLooping( bool bLooping=true ) { m_bLoop = looping; }

	void NewData( const char *pBuffer );

private:
	const char *buffer;
	RageSemaphore buffer_lock, buffer_finished;

	CString Create();

	void CreateTexture();
	void SkipUpdates();
	void StopSkippingUpdates();
	void CheckFrame();
	CString GetActiveFilterList();

	unsigned GetTexHandle() const { return m_uTexHandle; }
	unsigned m_uTexHandle;

	CComPtr<IGraphBuilder>  m_pGB;          // GraphBuilder
	bool					m_bLoop;
	bool					m_bPlaying;
};
#define USE_MOVIE_TEXTURE_DSHOW

#endif

/*
 * (c) 2001-2004 Chris Danford, Glenn Maynard
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
