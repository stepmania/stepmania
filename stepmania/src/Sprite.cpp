#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: Sprite

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include <math.h>
#include <assert.h>

#include "Sprite.h"
#include "RageTextureManager.h"
#include "IniFile.h"
#include "RageLog.h"
#include "RageException.h"
#include "PrefsManager.h"
#include "RageDisplay.h"
#include "GameConstantsAndTypes.h"
#include "SDL_utils.h"
#include "StepMania.h"

Sprite::Sprite()
{
	m_pTexture = NULL;
	m_bDrawIfTextureNull = false;
	m_iCurState = 0;
	m_fSecsIntoState = 0.0;
	m_bUsingCustomTexCoords = false;
	
	m_fRememberedClipWidth = -1;
	m_fRememberedClipHeight = -1;

	m_fTexCoordVelocityX = 0;
	m_fTexCoordVelocityY = 0;
}


Sprite::~Sprite()
{
	UnloadTexture();
}

bool Sprite::LoadBG( RageTextureID ID )
{
	ID.iMipMaps = 1;
	ID.bDither = true;
	return Load(ID);
}

bool Sprite::Load( RageTextureID ID )
{
	if( ID.filename == "" ) return true;
	if( ID.filename.Right(7) == ".sprite" )
		return LoadFromSpriteFile( ID );
	else 
		return LoadFromTexture( ID );
};


// Sprite file has the format:
//
// [Sprite]
// Texture=Textures\Logo.bmp
// Frame0000=0
// Delay0000=1.0
// Frame0001=3
// Delay0000=2.0
// BaseRotationXDegrees=0
// BaseRotationYDegrees=0
// BaseRotationZDegrees=0
// BaseZoomX=1
// BaseZoomY=1
// BaseZoomZ=1
bool Sprite::LoadFromSpriteFile( RageTextureID ID )
{
	LOG->Trace( ssprintf("Sprite::LoadFromSpriteFile(%s)", ID.filename.c_str()) );

	//Init();

	m_sSpritePath = ID.filename;


	// Split for the directory.  We'll need it below
	CString sFontDir, sFontFileName, sFontExtension;
	splitrelpath( m_sSpritePath, sFontDir, sFontFileName, sFontExtension );


	// read sprite file
	IniFile ini;
	ini.SetPath( m_sSpritePath );
	if( !ini.ReadFile() )
		RageException::Throw( "Error opening Sprite file '%s'.", m_sSpritePath.c_str() );

	CString sTextureFile;
	ini.GetValue( "Sprite", "Texture", sTextureFile );
	if( sTextureFile == ""  )
		RageException::Throw( "Error reading value 'Texture' from %s.", m_sSpritePath.c_str() );

	ID.filename = sFontDir + sTextureFile;	// save the path of the real texture
	{
		vector<CString> asElementPaths;
		GetDirListing( ID.filename + "*", asElementPaths, false, true );
		if(asElementPaths.size() == 0)
			RageException::Throw( "The sprite file '%s' points to a texture '%s' which doesn't exist.", m_sSpritePath.c_str(), ID.filename.c_str() );
		if(asElementPaths.size() > 1)
		{
			CString message = ssprintf( 
				"There is more than one file that matches "
				"'%s/%s'.  Please remove all but one of these matches.",
				ID.filename.c_str() );

			RageException::Throw( message ); 
		}
		ID.filename = asElementPaths[0];
	}

	// Load the texture
	LoadFromTexture( ID );

	// Read in frames and delays from the sprite file, 
	// overwriting the states that LoadFromTexture created.
	// If the .sprite file doesn't define any states, leave
	// frames and delays created during LoadFromTexture().
	for( int i=0; true; i++ )
	{
		CString sFrameKey = ssprintf( "Frame%04d", i );
		CString sDelayKey = ssprintf( "Delay%04d", i );
		State newState;

		if( !ini.GetValueI( "Sprite", sFrameKey, newState.iFrameIndex ) )
			break;
		if( newState.iFrameIndex >= m_pTexture->GetNumFrames() )
			RageException::Throw( "In '%s', %s is %d, but the texture %s only has %d frames.",
				m_sSpritePath.c_str(), sFrameKey.c_str(), newState.iFrameIndex, ID.filename.c_str(), m_pTexture->GetNumFrames() );

		if( !ini.GetValueF( "Sprite", sDelayKey, newState.fDelay ) )
			break;

		if( i == 0 )	// the ini file defines at least one frame
			m_States.clear();	// clear before adding

		m_States.push_back( newState );
	}

	float f;
	if( ini.GetValueF( "Sprite", "BaseRotationXDegrees", f ) )	Actor::SetBaseRotationX( f );
	if( ini.GetValueF( "Sprite", "BaseRotationYDegrees", f ) )	Actor::SetBaseRotationY( f );
	if( ini.GetValueF( "Sprite", "BaseRotationZDegrees", f ) )	Actor::SetBaseRotationZ( f );
	if( ini.GetValueF( "Sprite", "BaseZoomX", f ) )				Actor::SetBaseZoomX( f );
	if( ini.GetValueF( "Sprite", "BaseZoomY", f ) )				Actor::SetBaseZoomY( f );
	if( ini.GetValueF( "Sprite", "BaseZoomZ", f ) )				Actor::SetBaseZoomZ( f );


	return true;
}

void Sprite::UnloadTexture()
{
	if( m_pTexture != NULL )			// If there was a previous bitmap...
		TEXTUREMAN->UnloadTexture( m_pTexture );	// Unload it.
	m_pTexture = NULL;
}

void Sprite::EnableAnimation( bool bEnable )
{ 
	Actor::EnableAnimation( bEnable ); 
	if(m_pTexture) 
		bEnable ? m_pTexture->Play() : m_pTexture->Pause(); 
}

bool Sprite::LoadFromTexture( RageTextureID ID )
{
	if( !m_pTexture || !(m_pTexture->GetID() == ID) )
	{
		/* Load the texture if it's not already loaded.  We still need
		 * to do the rest, even if it's the same texture, since we need
		 * to reset Sprite::m_size, etc. */
		UnloadTexture();
		m_pTexture = TEXTUREMAN->LoadTexture( ID );
		ASSERT( m_pTexture->GetTextureWidth() >= 0 );
		ASSERT( m_pTexture->GetTextureHeight() >= 0 );
	}

	ASSERT( m_pTexture != NULL );

	// the size of the sprite is the size of the image before it was scaled
	Sprite::m_size.x = (float)m_pTexture->GetSourceFrameWidth();
	Sprite::m_size.y = (float)m_pTexture->GetSourceFrameHeight();		

	// Assume the frames of this animation play in sequential order with 0.2 second delay.
	m_States.clear();
	for( int i=0; i<m_pTexture->GetNumFrames(); i++ )
	{
		State newState = { i, 0.1f };
		m_States.push_back( newState );
	}

	// apply clipping (if any)
	if( m_fRememberedClipWidth != -1 && m_fRememberedClipHeight != -1 )
		ScaleToClipped( m_fRememberedClipWidth, m_fRememberedClipHeight );

	return true;
}



void Sprite::Update( float fDelta )
{
	Actor::Update( fDelta );	// do tweening

	if( !m_bIsAnimating )
	    return;

	if( !m_pTexture )	// no texture, nothing to animate
	    return;

	//
	// update animation frame
	//
	m_fSecsIntoState += fDelta;

	while( m_fSecsIntoState > m_States[m_iCurState].fDelay )	// it's time to switch frames
	{
		// increment frame and reset the counter
		m_fSecsIntoState -= m_States[m_iCurState].fDelay;		// leave the left over time for the next frame
		m_iCurState = (m_iCurState+1) % m_States.size();
	}

	//
	// update scrolling
	//
	if( m_fTexCoordVelocityX != 0 || m_fTexCoordVelocityY != 0 )
	{
		float fTexCoords[8];
		Sprite::GetActiveTextureCoords( fTexCoords );
 
		// top left, bottom left, bottom right, top right
		fTexCoords[0] += fDelta*m_fTexCoordVelocityX;
		fTexCoords[1] += fDelta*m_fTexCoordVelocityY; 
		fTexCoords[2] += fDelta*m_fTexCoordVelocityX;
		fTexCoords[3] += fDelta*m_fTexCoordVelocityY;
		fTexCoords[4] += fDelta*m_fTexCoordVelocityX;
		fTexCoords[5] += fDelta*m_fTexCoordVelocityY;
		fTexCoords[6] += fDelta*m_fTexCoordVelocityX;
		fTexCoords[7] += fDelta*m_fTexCoordVelocityY;

		Sprite::SetCustomTextureCoords( fTexCoords );
	}
}

static void TexCoordsFromArray(RageSpriteVertex *v, const float *f)
{
	v[0].t = RageVector2( f[0], f[1] );	// top left
	v[1].t = RageVector2( f[2],	f[3] );	// bottom left
	v[2].t = RageVector2( f[4],	f[5] );	// bottom right
	v[3].t = RageVector2( f[6],	f[7] );	// top right
}

void TexCoordArrayFromRect(float fImageCoords[8], const RectF &rect)
{
	fImageCoords[0] = rect.left;	fImageCoords[1] = rect.top;		// top left
	fImageCoords[2] = rect.left;	fImageCoords[3] = rect.bottom;	// bottom left
	fImageCoords[4] = rect.right;	fImageCoords[5] = rect.bottom;	// bottom right
	fImageCoords[6] = rect.right;	fImageCoords[7] = rect.top;		// top right
}

void Sprite::DrawPrimitives()
{
	if( m_pTexture == NULL  &&  !m_bDrawIfTextureNull )
		return;

	// bail if cropped all the way 
    if( m_temp.crop.left + m_temp.crop.right > 1  || 
		m_temp.crop.top + m_temp.crop.bottom > 1 ) 
		return; 


	// This is causing terrible movie performance on an Athlon 1.3+Voodoo3.
	// With the default delay of 2ms, the frame rate of demonstration dives
	// from 110fps to 84. SDL_Delay must behave differently than Win32's
	// Sleep(). Leave this commented for now, and uncomment if we hear
	// about problems with dropped movie frames. -Chris
//	if( m_pTexture && m_pTexture->IsAMovie() && m_pTexture->IsPlaying() )
//		SDL_Delay( PREFSMAN->m_iMovieDecodeMS );         // let the movie decode a frame

	// use m_temp_* variables to draw the object
	RectF quadVerticies;

	switch( m_HorizAlign )
	{
	case align_left:	quadVerticies.left = 0;				quadVerticies.right = m_size.x;		break;
	case align_center:	quadVerticies.left = -m_size.x/2;	quadVerticies.right = m_size.x/2;	break;
	case align_right:	quadVerticies.left = -m_size.x;		quadVerticies.right = 0;			break;
	default:		ASSERT( false );
	}

	switch( m_VertAlign )
	{
	// FIXME:  Top and bottom are flipped, but changing them now breaks a lot
	// in our themes.  -Chris
	case align_top:		quadVerticies.top = 0;				quadVerticies.bottom = m_size.y;	break;
	case align_middle:	quadVerticies.top = -m_size.y/2;	quadVerticies.bottom = m_size.y/2;	break;
	case align_bottom:	quadVerticies.top = -m_size.y;		quadVerticies.bottom = 0;			break;
	default:		ASSERT(0);
	}


	RectF croppedQuadVerticies = quadVerticies; 
#define IF_CROP_POS(side,opp_side) if(m_temp.crop.side>0) croppedQuadVerticies.side = SCALE( m_temp.crop.side, 0.f, 1.f, quadVerticies.side, quadVerticies.opp_side ); 
	IF_CROP_POS( left, right ); 
	IF_CROP_POS( top, bottom ); 
	IF_CROP_POS( right, left ); 
	IF_CROP_POS( bottom, top ); 



	static RageSpriteVertex v[4];
	v[0].p = RageVector3( croppedQuadVerticies.left,	croppedQuadVerticies.top,		0 );	// top left
	v[1].p = RageVector3( croppedQuadVerticies.left,	croppedQuadVerticies.bottom,	0 );	// bottom left
	v[2].p = RageVector3( croppedQuadVerticies.right,	croppedQuadVerticies.bottom,	0 );	// bottom right
	v[3].p = RageVector3( croppedQuadVerticies.right,	croppedQuadVerticies.top,		0 );	// top right

	DISPLAY->SetTexture( m_pTexture );

	// Must call this after setting the texture or else texture 
	// parameters have no effect.
	Actor::SetRenderStates();	// set Actor-specified render states

	if( m_pTexture )
	{
		float f[8];
		GetActiveTextureCoords(f);
		TexCoordsFromArray(v, f);


		RageVector2 texCoords[4] = {
			RageVector2( f[0], f[1] ),	// top left
			RageVector2( f[2],	f[3] ),	// bottom left
			RageVector2( f[4],	f[5] ),	// bottom right
			RageVector2( f[6],	f[7] ) 	// top right
		};
	

		if( m_temp.crop.left>0 )
		{
			v[0].t.x = SCALE( m_temp.crop.left, 0.f, 1.f, texCoords[0].x, texCoords[3].x );
			v[1].t.x = SCALE( m_temp.crop.left, 0.f, 1.f, texCoords[1].x, texCoords[2].x );
		}
		if( m_temp.crop.right>0 )
		{
			v[2].t.x = SCALE( m_temp.crop.right, 0.f, 1.f, texCoords[2].x, texCoords[1].x );
			v[3].t.x = SCALE( m_temp.crop.right, 0.f, 1.f, texCoords[3].x, texCoords[0].x );
		}
		if( m_temp.crop.top>0 )
		{
			v[0].t.y = SCALE( m_temp.crop.top, 0.f, 1.f, texCoords[0].y, texCoords[1].y );
			v[3].t.y = SCALE( m_temp.crop.top, 0.f, 1.f, texCoords[3].y, texCoords[2].y );
		}
		if( m_temp.crop.bottom>0 )
		{
			v[1].t.y = SCALE( m_temp.crop.bottom, 0.f, 1.f, texCoords[1].y, texCoords[0].y );
			v[2].t.y = SCALE( m_temp.crop.bottom, 0.f, 1.f, texCoords[2].y, texCoords[3].y );
		}
	}

	DISPLAY->SetTextureModeModulate();

	/* Draw if we're not fully transparent */
	if( m_temp.diffuse[0].a > 0 || 
		m_temp.diffuse[1].a > 0 ||
		m_temp.diffuse[2].a > 0 ||
		m_temp.diffuse[3].a > 0 )
	{
		//////////////////////
		// render the shadow
		//////////////////////
		if( m_bShadow )
		{
			DISPLAY->PushMatrix();
			DISPLAY->TranslateWorld( m_fShadowLength, m_fShadowLength, 0 );	// shift by 5 units
			v[0].c = v[1].c = v[2].c = v[3].c = RageColor(0,0,0,0.5f*m_temp.diffuse[0].a);	// semi-transparent black
			DISPLAY->DrawQuad( v );
			DISPLAY->PopMatrix();
		}

		//////////////////////
		// render the diffuse pass
		//////////////////////
		v[0].c = m_temp.diffuse[0];	// top left
		v[1].c = m_temp.diffuse[2];	// bottom left
		v[2].c = m_temp.diffuse[3];	// bottom right
		v[3].c = m_temp.diffuse[1];	// top right
		DISPLAY->DrawQuad( v );
//		glEnable(GL_BLEND);
	}

	//////////////////////
	// render the glow pass
	//////////////////////
	if( m_temp.glow.a > 0.0001f )
	{
		DISPLAY->SetTextureModeGlow(m_temp.glowmode);
		v[0].c = v[1].c = v[2].c = v[3].c = m_temp.glow;
		DISPLAY->DrawQuad( v );
	}
}


void Sprite::SetState( int iNewState )
{
	// This assert will likely trigger if the "missing" theme element graphic 
	// is loaded in place of a multi-frame sprite.  We want to know about these
	// problems in debug builds, but they're not fatal.
	DEBUG_ASSERT( iNewState >= 0  &&  iNewState < (int)m_States.size() );

	CLAMP(iNewState, 0, (int)m_States.size()-1);
	m_iCurState = iNewState;
	m_fSecsIntoState = 0.0; 
}

void Sprite::SetCustomTextureRect( const RectF &new_texcoord_frect ) 
{ 
	m_bUsingCustomTexCoords = true;
	m_bTextureWrapping = true;
	TexCoordArrayFromRect(m_CustomTexCoords, new_texcoord_frect);
}

void Sprite::SetCustomTextureCoords( float fTexCoords[8] ) // order: top left, bottom left, bottom right, top right
{ 
	m_bUsingCustomTexCoords = true;
	m_bTextureWrapping = true;
	for( int i=0; i<8; i++ )
		m_CustomTexCoords[i] = fTexCoords[i]; 
}

void Sprite::SetCustomImageRect( RectF rectImageCoords )
{
	// Convert to a rectangle in texture coordinate space.
	rectImageCoords.left	*= m_pTexture->GetImageWidth()	/ (float)m_pTexture->GetTextureWidth();
	rectImageCoords.right	*= m_pTexture->GetImageWidth()	/ (float)m_pTexture->GetTextureWidth();
	rectImageCoords.top		*= m_pTexture->GetImageHeight()	/ (float)m_pTexture->GetTextureHeight(); 
	rectImageCoords.bottom	*= m_pTexture->GetImageHeight()	/ (float)m_pTexture->GetTextureHeight(); 

	SetCustomTextureRect( rectImageCoords );
}

void Sprite::SetCustomImageCoords( float fImageCoords[8] )	// order: top left, bottom left, bottom right, top right
{
	// convert image coords to texture coords in place
	for( int i=0; i<8; i+=2 )
	{
		fImageCoords[i+0] *= m_pTexture->GetImageWidth()	/ (float)m_pTexture->GetTextureWidth(); 
		fImageCoords[i+1] *= m_pTexture->GetImageHeight()	/ (float)m_pTexture->GetTextureHeight(); 
	}

	SetCustomTextureCoords( fImageCoords );
}

const RectF *Sprite::GetCurrentTextureCoordRect() const
{
	unsigned int uFrameNo = m_States[m_iCurState].iFrameIndex;
	return m_pTexture->GetTextureCoordRect( uFrameNo );
}


/* If we're using custom coordinates, return them; otherwise return the coordinates
 * for the current state. */
void Sprite::GetActiveTextureCoords(float fTexCoordsOut[8]) const
{
	if(m_bUsingCustomTexCoords) 
	{
		// GetCustomTextureCoords
		for( int i=0; i<8; i++ )
			fTexCoordsOut[i] = m_CustomTexCoords[i]; 
	}
	else
	{
		// GetCurrentTextureCoords
		const RectF *pTexCoordRect = GetCurrentTextureCoordRect();
		TexCoordArrayFromRect(fTexCoordsOut, *pTexCoordRect);
	}
}


void Sprite::StopUsingCustomCoords()
{
	m_bUsingCustomTexCoords = false;
}


void Sprite::ScaleToClipped( float fWidth, float fHeight )
{
	m_fRememberedClipWidth = fWidth;
	m_fRememberedClipHeight = fHeight;

	if( !m_pTexture )
		return;

	int iSourceWidth	= m_pTexture->GetSourceWidth();
	int iSourceHeight	= m_pTexture->GetSourceHeight();

	// save the original X&Y.  We're going to resore them later.
	float fOriginalX = GetX();
	float fOriginalY = GetY();

	if( IsDiagonalBanner(iSourceWidth, iSourceHeight) )		// this is a SSR/DWI CroppedSprite
	{
		float fCustomImageCoords[8] = {
			0.02f,	0.78f,	// top left
			0.22f,	0.98f,	// bottom left
			0.98f,	0.22f,	// bottom right
			0.78f,	0.02f,	// top right
		};
		Sprite::SetCustomImageCoords( fCustomImageCoords );

		if( fWidth != -1 && fHeight != -1)
			m_size = RageVector2( fWidth, fHeight );
		else
		{
			/* If no crop size is set, then we're only being used to crop diagonal
			 * banners so they look like regular ones. We don't actually care about
			 * the size of the image, only that it has an aspect ratio of 4:1.  */
			m_size = RageVector2(256, 64);
		}
		SetZoom( 1 );
	}
	else if( m_pTexture->GetID().filename.find( "(was rotated)" ) != m_pTexture->GetID().filename.npos && 
			 fWidth != -1 && fHeight != -1 )
	{
		/* Dumb hack.  Normally, we crop all sprites except for diagonal banners,
		 * which are stretched.  Low-res versions of banners need to do the same
		 * thing as their full resolution counterpart, so the crossfade looks right.
		 * However, low-res diagonal banners are un-rotated, to save space.  BannerCache
		 * drops the above text into the "filename" (which is otherwise unused for
		 * these banners) to tell us this.
		 */
		Sprite::StopUsingCustomCoords();
		m_size = RageVector2( fWidth, fHeight );
		SetZoom( 1 );
	}
	else if( fWidth != -1 && fHeight != -1 )
	{
		// this is probably a background graphic or something not intended to be a CroppedSprite
		Sprite::StopUsingCustomCoords();

		// first find the correct zoom
		Sprite::ScaleToCover( RectI(0, 0,
									(int)fWidth,
									(int)fHeight )
							 );
		// find which dimension is larger
		bool bXDimNeedsToBeCropped = GetZoomedWidth() > fWidth+0.01;
		
		if( bXDimNeedsToBeCropped )	// crop X
		{
			float fPercentageToCutOff = (this->GetZoomedWidth() - fWidth) / this->GetZoomedWidth();
			float fPercentageToCutOffEachSide = fPercentageToCutOff / 2;
			
			// generate a rectangle with new texture coordinates
			RectF fCustomImageRect( 
				fPercentageToCutOffEachSide, 
				0, 
				1 - fPercentageToCutOffEachSide, 
				1 );
			SetCustomImageRect( fCustomImageRect );
		}
		else		// crop Y
		{
			float fPercentageToCutOff = (this->GetZoomedHeight() - fHeight) / this->GetZoomedHeight();
			float fPercentageToCutOffEachSide = fPercentageToCutOff / 2;
			
			// generate a rectangle with new texture coordinates
			RectF fCustomImageRect( 
				0, 
				fPercentageToCutOffEachSide,
				1, 
				1 - fPercentageToCutOffEachSide );
			SetCustomImageRect( fCustomImageRect );
		}
		m_size = RageVector2( fWidth, fHeight );
		SetZoom( 1 );
	}

	// restore original XY
	SetXY( fOriginalX, fOriginalY );
}

bool Sprite::IsDiagonalBanner( int iWidth, int iHeight )
{
	/* A diagonal banner is a square.  Give a couple pixels of leeway. */
	return iWidth >= 100 && abs(iWidth - iHeight) < 2;
}

inline CString GetParam( const CStringArray& sParams, int iIndex, int& iMaxIndexAccessed )
{
	iMaxIndexAccessed = max( iIndex, iMaxIndexAccessed );
	if( iIndex < int(sParams.size()) )
		return sParams[iIndex];
	else
		return "";
}

void Sprite::HandleCommand( const CStringArray &asTokens )
{
	int iMaxIndexAccessed = 0;

#define sParam(i) (GetParam(asTokens,i,iMaxIndexAccessed))
#define fParam(i) ((float)atof(sParam(i)))
#define iParam(i) (atoi(sParam(i)))
#define bParam(i) (iParam(i)!=0)

	const CString& sName = asTokens[0];

	// Commands that go in the tweening queue:
	/* Add left/right/top/bottom for alpha if needed. */
	// Commands that take effect immediately (ignoring the tweening queue):
	if( sName=="customtexturerect" )	SetCustomTextureRect( RectF(fParam(1),fParam(2),fParam(3),fParam(4)) );
	else if( sName=="texcoordvelocity" )	SetTexCoordVelocity( fParam(1),fParam(2) );
	else if( sName=="scaletoclipped" )	ScaleToClipped( fParam(1),fParam(2) );
	else
	{
		Actor::HandleCommand( asTokens );
		return;
	}

	if( iMaxIndexAccessed != (int)asTokens.size()-1 )
	{
		CString sError = ssprintf( "Actor::HandleCommand: Wrong number of parameters in command '%s'.  Expected %d but there are %d.", join(",",asTokens).c_str(), iMaxIndexAccessed+1, (int)asTokens.size() );
		LOG->Warn( sError );
		if( DISPLAY->IsWindowed() )
			HOOKS->MessageBoxOK( sError );
	}
}
