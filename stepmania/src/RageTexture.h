#pragma once
/*
-----------------------------------------------------------------------------
 Class: RageTexture

 Desc: Abstract class for a texture and metadata.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "RageTypes.h"


const int MAX_TEXTURE_FRAMES = 256;


struct RageTexturePrefs
{
	bool bForceReload;
	int iMaxSize;
	int iMipMaps;
	int iAlphaBits;
	bool bDither;
	bool bStretch;

	RageTexturePrefs()
	{
		bForceReload = false;
		iMaxSize = 2048;
		iMipMaps = 4;
		iAlphaBits = 4;
		bDither = false;
		bStretch = false;
	}
};

//-----------------------------------------------------------------------------
// RageTexture Class Declarations
//-----------------------------------------------------------------------------
class RageTexture
{
public:
	RageTexture( CString sFilePath, RageTexturePrefs prefs );
	virtual ~RageTexture() = 0;
	virtual void Reload( RageTexturePrefs prefs ) = 0;
	virtual void Invalidate() { }	/* only called by RageTextureManager::InvalidateTextures */
	virtual unsigned int GetGLTextureID() = 0;	// accessed by RageDisplay

	// movie texture/animated texture stuff
	virtual void Play() {}
	virtual void Stop() {}
	virtual void Pause() {}
	virtual void SetPosition( float fSeconds ) {}
	virtual bool IsAMovie() const { return false; }
	virtual bool IsPlaying() const { return false; }
	void SetLooping(bool looping) { }

	int GetSourceWidth() const	{return m_iSourceWidth;}
	int GetSourceHeight() const {return m_iSourceHeight;}
	int GetTextureWidth() const {return m_iTextureWidth;}
	int GetTextureHeight() const{return m_iTextureHeight;}
	int GetImageWidth() const	{return m_iImageWidth;}
	int GetImageHeight() const	{return m_iImageHeight;}

	int GetFramesWide() const	{return m_iFramesWide;}
	int GetFramesHigh() const	{return m_iFramesHigh;}

	int GetSourceFrameWidth() const		{return GetSourceWidth()	/	GetFramesWide();}
	int GetSourceFrameHeight() const	{return GetSourceHeight()	/	GetFramesHigh();}
	int GetTextureFrameWidth() const	{return GetTextureWidth()	/	GetFramesWide();}
	int GetTextureFrameHeight() const	{return GetTextureHeight()	/	GetFramesHigh();}
	int GetImageFrameWidth() const		{return GetImageWidth()		/	GetFramesWide();}
	int GetImageFrameHeight() const		{return GetImageHeight()	/	GetFramesHigh();}
	
	const RectF *GetTextureCoordRect( int frameNo ) const;
	int   GetNumFrames() const {return m_iFramesWide*m_iFramesHigh;}
	CString GetFilePath() const {return m_sFilePath;}

	int		m_iRefCount;
	int		m_iTimeOfLastUnload;


protected:

	virtual void CreateFrameRects();
	virtual void GetFrameDimensionsFromFileName( CString sPath, int* puFramesWide, int* puFramesHigh ) const;

	CString	m_sFilePath;
	RageTexturePrefs m_prefs;

	int		m_iSourceWidth,		m_iSourceHeight;	// dimensions of the original image loaded from disk
	int		m_iTextureWidth,	m_iTextureHeight;	// dimensions of the texture in memory
	int		m_iImageWidth,		m_iImageHeight;		// dimensions of the image in the texture
	int		m_iFramesWide,		m_iFramesHigh;		// The number of frames of animation in each row and column of this texture
	CArray<RectF,RectF>	m_TextureCoordRects;	// size = m_iFramesWide * m_iFramesHigh
};


