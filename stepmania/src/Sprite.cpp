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

#include "RageDisplayInternal.h"

Sprite::Sprite()
{
	m_pTexture = NULL;
	m_bDrawIfTextureNull = false;
	m_iNumStates = 0;
	m_iCurState = 0;
	m_fSecsIntoState = 0.0;
	m_bUsingCustomTexCoords = false;
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
	for( int i=0; i<MAX_SPRITE_STATES; i++ )
	{
		CString sFrameKey = ssprintf( "Frame%04d", i );
		CString sDelayKey = ssprintf( "Delay%04d", i );
		
		m_iStateToFrame[i] = 0;
		if( !ini.GetValueI( "Sprite", sFrameKey, m_iStateToFrame[i] ) )
			break;
		if( m_iStateToFrame[i] >= m_pTexture->GetNumFrames() )
			RageException::Throw( "In '%s', %s is %d, but the texture %s only has %d frames.",
				m_sSpritePath.c_str(), sFrameKey.c_str(), m_iStateToFrame[i], ID.filename.c_str(), m_pTexture->GetNumFrames() );
		m_fDelay[i] = 0.2f;
		if( !ini.GetValueF( "Sprite", sDelayKey, m_fDelay[i] ) )
			break;

		if( m_iStateToFrame[i] == 0  &&  m_fDelay[i] > -0.00001f  &&  m_fDelay[i] < 0.00001f )	// both values are empty
			break;

		m_iNumStates = i+1;
	}

	if( m_iNumStates == 0 )
	{
		m_iNumStates = 1;
		m_iStateToFrame[0] = 0;
		m_fDelay[0] = 10;
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
	}

	ASSERT( m_pTexture != NULL );

	// the size of the sprite is the size of the image before it was scaled
	Sprite::m_size.x = (float)m_pTexture->GetSourceFrameWidth();
	Sprite::m_size.y = (float)m_pTexture->GetSourceFrameHeight();		

	// Assume the frames of this animation play in sequential order with 0.2 second delay.
	for( int i=0; i<m_pTexture->GetNumFrames(); i++ )
	{
		m_iStateToFrame[i] = i;
		m_fDelay[i] = 0.1f;
	}

	m_iNumStates = m_pTexture->GetNumFrames();
	return true;
}



void Sprite::Update( float fDeltaTime )
{
	Actor::Update( fDeltaTime );	// do tweening

	if( !m_bIsAnimating )
	    return;

	// update animation
	m_fSecsIntoState += fDeltaTime;

	if( m_fSecsIntoState > m_fDelay[m_iCurState] )		// it's time to switch frames
	{
		// increment frame and reset the counter
		m_fSecsIntoState -= m_fDelay[m_iCurState];		// leave the left over time for the next frame
		m_iCurState ++;
		if( m_iCurState >= m_iNumStates )
			m_iCurState = 0;
	}
}

static void TexCoordsFromArray(RageVertex *v, const float *f)
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

	if( m_pTexture  &&  m_pTexture->IsAMovie()  &&  m_pTexture->IsPlaying() )
		SDL_Delay( PREFSMAN->m_iMovieDecodeMS );	// let the movie decode a frame

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
	case align_top:		quadVerticies.top = 0;				quadVerticies.bottom = m_size.y;	break;
	case align_middle:	quadVerticies.top = -m_size.y/2;	quadVerticies.bottom = m_size.y/2;	break;
	case align_bottom:	quadVerticies.top = -m_size.y;		quadVerticies.bottom = 0;			break;
	default:		ASSERT( false );
	}


	static RageVertex v[4];
	v[0].p = RageVector3( quadVerticies.left,	quadVerticies.top,		0 );	// top left
	v[1].p = RageVector3( quadVerticies.left,	quadVerticies.bottom,	0 );	// bottom left
	v[2].p = RageVector3( quadVerticies.right,	quadVerticies.bottom,	0 );	// bottom right
	v[3].p = RageVector3( quadVerticies.right,	quadVerticies.top,		0 );	// top right

	DISPLAY->SetTexture( m_pTexture );

	if( m_pTexture )
	{
		float TexCoords[8];
		GetActiveTexCoords(TexCoords);
		TexCoordsFromArray(v, TexCoords);

		DISPLAY->EnableTextureWrapping(m_bTextureWrapping);
	}

	DISPLAY->SetTextureModeModulate();
	if( m_bBlendAdd )
		DISPLAY->SetBlendModeAdd();
	else
		DISPLAY->SetBlendModeNormal();

	/* Draw if we're not fully transparent or the zbuffer is enabled (which ignores
	 * alpha). */
	if( m_temp.diffuse[0].a > 0 || 
		m_temp.diffuse[1].a > 0 ||
		m_temp.diffuse[2].a > 0 ||
		m_temp.diffuse[3].a > 0 ||
		DISPLAY->ZBufferEnabled() )
	{
		//////////////////////
		// render the shadow
		//////////////////////
		if( m_bShadow )
		{
			DISPLAY->PushMatrix();
			DISPLAY->TranslateLocal( m_fShadowLength, m_fShadowLength, 0 );	// shift by 5 units
			v[0].c = v[1].c = v[2].c = v[3].c = RageColor(0,0,0,0.5f*m_temp.diffuse[0].a);	// semi-transparent black
			DISPLAY->DrawQuad( v );
			DISPLAY->PopMatrix();
		}

		/* If the texture doesn't have alpha, and we're not changing alpha in diffuse,
		 * don't bother to blend when doing the diffuse pass. */
/* I'm not sure this actually helps anywhere.
		if(m_pTexture && !m_bBlendAdd && m_pTexture->GetActualID().iAlphaBits == 0 &&
			m_temp.diffuse[0].a + m_temp.diffuse[1].a + m_temp.diffuse[2].a + m_temp.diffuse[3].a == 4)
			glDisable(GL_BLEND);
*/
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
	DEBUG_ASSERT( iNewState >= 0  &&  iNewState < m_iNumStates );

	CLAMP(iNewState, 0, m_iNumStates-1);
	m_iCurState = iNewState;
	m_fSecsIntoState = 0.0; 
}

void Sprite::SetCustomTextureRect( const RectF &new_texcoord_frect ) 
{ 
	m_bUsingCustomTexCoords = true;
	m_bTextureWrapping = true;
	TexCoordArrayFromRect(m_CustomTexCoords, new_texcoord_frect);
}

void Sprite::SetCustomTextureCoords( float fTexCoords[8] ) // order: bottom left, top left, bottom right, top right
{ 
	m_bUsingCustomTexCoords = true;
	m_bTextureWrapping = true;
	for( int i=0; i<8; i++ )
		m_CustomTexCoords[i] = fTexCoords[i]; 
}

void Sprite::GetCustomTextureCoords( float fTexCoordsOut[8] ) const // order: bottom left, top left, bottom right, top right
{ 
	for( int i=0; i<8; i++ )
		fTexCoordsOut[i] = m_CustomTexCoords[i]; 
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

void Sprite::SetCustomImageCoords( float fImageCoords[8] )	// order: bottom left, top left, bottom right, top right
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
	unsigned int uFrameNo = m_iStateToFrame[m_iCurState];
	return m_pTexture->GetTextureCoordRect( uFrameNo );
}

void Sprite::GetCurrentTextureCoords(float fImageCoords[8]) const
{
	const RectF *pTexCoordRect = GetCurrentTextureCoordRect();
	TexCoordArrayFromRect(fImageCoords, *pTexCoordRect);
}


/* If we're using custom coordinates, return them; otherwise return the coordinates
 * for the current state. */
void Sprite::GetActiveTexCoords(float fImageCoords[8]) const
{
	if(m_bUsingCustomTexCoords) GetCustomTextureCoords(fImageCoords);
	else GetCurrentTextureCoords(fImageCoords);
}


void Sprite::StopUsingCustomCoords()
{
	m_bUsingCustomTexCoords = false;
}

