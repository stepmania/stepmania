#pragma once
/*
-----------------------------------------------------------------------------
 Class: RageMovieTexture

 Desc: Based on the NeHe tutorial 35

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageTexture.h"

struct AviRenderer;	// defined in RageMovieTexture.cpp

//-----------------------------------------------------------------------------
// RageMovieTexture Class Declarations
//-----------------------------------------------------------------------------
class RageMovieTexture : public RageTexture
{
public:
	RageMovieTexture( CString sFilePath, RageTexturePrefs prefs );
	virtual ~RageMovieTexture();
	virtual void Update( float fDeltaTime );
	virtual void Invalidate() { m_uGLTextureID = 0; }
	virtual void Reload( RageTexturePrefs prefs );

	virtual void Play();
	virtual void Pause();
	virtual void Stop();
	virtual void SetPosition( float fSeconds );
	virtual bool IsPlaying() const;
	void SetLooping(bool looping);

protected:
	void Create();	// called by constructor and Reload

	virtual unsigned int GetGLTextureID();

	unsigned int	m_uGLTextureID;
	struct AviRenderer* pAviRenderer;
};
