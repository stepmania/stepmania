#ifndef RAGEMOVIETEXTURE_H
#define RAGEMOVIETEXTURE_H
/*
-----------------------------------------------------------------------------
 Class: RageMovieTexture

 Desc: Based on the DShowTextures example in the DX8 SDK.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

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
class RageMovieTexture : public RageTexture
{
public:
	RageMovieTexture( const CString &sFilePath, RageTexturePrefs prefs );
	virtual ~RageMovieTexture();
	void Update(float fDeltaTime);

	virtual void Reload( RageTexturePrefs prefs );

	virtual void Play();
	virtual void Pause();
	virtual void Stop();
	virtual void SetPosition( float fSeconds );
	virtual bool IsAMovie() const { return true; };
	virtual bool IsPlaying() const;
	void SetLooping(bool looping=true) { m_bLoop = looping; }

//	LPDIRECT3DTEXTURE8 GetBackBuffer() { return m_pd3dTexture[!m_iIndexFrontBuffer]; }
//	void Flip() { m_iIndexFrontBuffer = !m_iIndexFrontBuffer; }

	void NewData(char *buffer);

protected:
//	LPDIRECT3DTEXTURE8  m_pd3dTexture[2];	// double buffered
	int m_iIndexFrontBuffer;	// index of the buffer that should be rendered from - either 0 or 1

	char *buffer;
	bool buffer_changed;
	RageMutex buffer_mutex;

	void Create();

	bool CreateTexture();
	bool PlayMovie();
	void CheckMovieStatus();

	CString m_FilePath;

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
