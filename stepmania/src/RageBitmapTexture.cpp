#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: RageBitmapTexture

 Desc: Holder for a static texture with metadata.  Can load just about any image format.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/

#include "RageBitmapTexture.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageException.h"
#include "RageTextureManager.h"

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_endian.h"
#include "SDL_rotozoom.h"
#include "SDL_utils.h"
#include "SDL_dither.h"
#include "SDL_opengl.h"


//-----------------------------------------------------------------------------
// RageBitmapTexture constructor
//-----------------------------------------------------------------------------
RageBitmapTexture::RageBitmapTexture( const CString &sFilePath ) : RageTexture( sFilePath )
{
//	LOG->Trace( "RageBitmapTexture::RageBitmapTexture()" );

	m_uGLTextureID = 0;
	Load( sFilePath );
}

RageBitmapTexture::~RageBitmapTexture()
{
	if(m_uGLTextureID)
		glDeleteTextures(1, &m_uGLTextureID);
}

int power_of_two(int input)
{
    int value = 1;
	while ( value < input ) value <<= 1;
	return value;
}

void RageBitmapTexture::Load( const CString &sFilePath )
{
	m_sFilePath = sFilePath;	// save file path

	int iMaxSize = DISPLAY->GetMaxTextureSize();

	SDL_Surface *img;

	/* Load the image into an SDL surface. */
	img = IMG_Load(sFilePath);
	if(!img)
		throw RageException("Could not load graphic '%s'", sFilePath.GetString());

	/* Save information about the source.  Unless something else changes this
	 * later, the image inside the texture is the same size as the source. */
	m_iImageWidth = m_iSourceWidth = img->w;
	m_iImageHeight = m_iSourceHeight = img->h;

	/* Texture dimensions need to be a power of two; jump to the next. */
	m_iTextureWidth = power_of_two(img->w);
	m_iTextureHeight = power_of_two(img->h);


	/* Convert the data to the destination format.  Hmm.  We could just
	 * convert the format, leaving the resolution alone (simplifying
	 * ConvertSDLSurface), and then load the texture a little more
	 * intelligently.  If we do that with OpenGL, is the rest
	 * of the texture (that we didn't fill) guaranteed to be black?
	 * We don't want anything else to be linearly filtered in on the
	 * edge of the texture ...
	 */
	ConvertSDLSurface(img, m_iTextureWidth, m_iTextureHeight, 32,
		0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000 );

	if(!m_uGLTextureID)
		glGenTextures(1, &m_uGLTextureID);

	DISPLAY->SetTexture(this);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, img->pitch / img->format->BytesPerPixel);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img->w, img->h, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, img->pixels);
	glFlush();

	SDL_FreeSurface(img);

	CreateFrameRects();
}

void RageBitmapTexture::Reload()
{
	DISPLAY->SetTexture(0);

	if(m_uGLTextureID) {
		glDeleteTextures(1, &m_uGLTextureID);
		m_uGLTextureID = 0;
	}

	Load( m_sFilePath );
}
