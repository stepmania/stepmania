#ifndef RAGEMOVIETEXTURE_H
#define RAGEMOVIETEXTURE_H
/*
-----------------------------------------------------------------------------
 Class: RageMovieTexture

 Desc: Based on the DShowTextures example in the DX8 SDK.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/
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
	/* only called by RageTextureManager::InvalidateTextures */
	void Invalidate() { m_uGLTextureID = 0; }
	void Update(float fDeltaTime);

	virtual void Reload();

	virtual void Play();
	virtual void Pause();
	virtual void Stop();
	virtual void SetPosition( float fSeconds );
	virtual bool IsPlaying() const;
	void SetLooping(bool looping=true) { m_bLoop = looping; }

	void NewData(char *buffer);

protected:
	char *buffer;
	bool buffer_changed;
	RageMutex buffer_mutex;

	void Create();

	void CreateTexture();
	bool PlayMovie();
	void CheckMovieStatus();

	virtual unsigned int GetGLTextureID();
	unsigned int m_uGLTextureID;

	//-----------------------------------------------------------------------------
	// DirectShow pointers
	//-----------------------------------------------------------------------------
	CComPtr<IGraphBuilder>  m_pGB;          // GraphBuilder
	bool					m_bLoop;
	bool					m_bPlaying;
};

#endif
