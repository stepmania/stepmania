#ifndef RAGE_TEXTURE_H
#define RAGE_TEXTURE_H
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

/* A unique texture is identified by a RageTextureID.  (Loading the
 * same file with two different dither settings is considered two
 * different textures, for example.) */
struct RageTextureID
{
	CString filename;
	int iMaxSize;
	int iMipMaps;
	int iAlphaBits;
	int iColorDepth;
	bool bDither;
	bool bStretch;

	/* Define an ordering so this can be used in a set<>.  This defines a partial
	 * ordering; use equal() to see if they're equal. */
	bool operator< (const RageTextureID &rhs) const	{ return filename < rhs.filename; }
	bool equal(const RageTextureID &rhs) const;

	void Init();

	RageTextureID() { Init(); }
	RageTextureID(const CString &fn) { Init(); filename=fn; }
};

//-----------------------------------------------------------------------------
// RageTexture Class Declarations
//-----------------------------------------------------------------------------
class RageTexture
{
public:
	RageTexture( RageTextureID file );
	virtual ~RageTexture() = 0;
	virtual void Update( float fDeltaTime ) {}
	virtual void Reload( RageTextureID ID ) { m_ID = m_ActualID = ID; }
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
	int   GetNumFrames() const { return m_iFramesWide*m_iFramesHigh; }
	const CString &GetFilePath() const { return GetID().filename; }

	int		m_iRefCount;
	int		m_iTimeOfLastUnload;

	/* The ID that we were asked to load: */
	const RageTextureID &GetID() const { return m_ID; }

	/* The ID that we actually got: */
	const RageTextureID &GetActualID() const { return m_ActualID; }

	static void GetFrameDimensionsFromFileName( CString sPath, int* puFramesWide, int* puFramesHigh );
	static int GetFrameCountFromFileName( CString sPath );

private:
	/* The file we were asked to load.  (This is never changed.) */
	RageTextureID m_ID;

protected:
	/* We might change settings when loading (due to hints, hardware
	 * limitations, etc).  The data actually loaded is here: */
	RageTextureID m_ActualID;

	int		m_iSourceWidth,		m_iSourceHeight;	// dimensions of the original image loaded from disk
	int		m_iTextureWidth,	m_iTextureHeight;	// dimensions of the texture in memory
	int		m_iImageWidth,		m_iImageHeight;		// dimensions of the image in the texture
	int		m_iFramesWide,		m_iFramesHigh;		// The number of frames of animation in each row and column of this texture
	vector<RectF>	m_TextureCoordRects;	// size = m_iFramesWide * m_iFramesHigh

	virtual void CreateFrameRects();
};

#endif
