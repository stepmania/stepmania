#include "stdafx.h"
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


Sprite::Sprite()
{
	m_pTexture = NULL;
	m_bDrawIfTextureNull = false;
	m_iNumStates = 0;
	m_iCurState = 0;
	m_bIsAnimating = true;
	m_fSecsIntoState = 0.0;
	m_bUsingCustomTexCoords = false;
}


Sprite::~Sprite()
{
	UnloadTexture();
}



// Sprite file has the format:
//
// [Sprite]
// Texture=Textures\Logo.bmp
// Frame0000=0
// Delay0000=1.0
// Frame0001=3
// Delay0000=2.0
bool Sprite::LoadFromSpriteFile( RageTextureID ID )
{
	LOG->Trace( ssprintf("Sprite::LoadFromSpriteFile(%s)", ID.filename.GetString()) );

	//Init();

	m_sSpritePath = ID.filename;


	// Split for the directory.  We'll need it below
	CString sFontDir, sFontFileName, sFontExtension;
	splitrelpath( m_sSpritePath, sFontDir, sFontFileName, sFontExtension );





	// read sprite file
	IniFile ini;
	ini.SetPath( m_sSpritePath );
	if( !ini.ReadFile() )
		RageException::Throw( "Error opening Sprite file '%s'.", m_sSpritePath.GetString() );

	CString sTextureFile;
	ini.GetValue( "Sprite", "Texture", sTextureFile );
	if( sTextureFile == ""  )
		RageException::Throw( "Error reading value 'Texture' from %s.", m_sSpritePath.GetString() );

	ID.filename = sFontDir + sTextureFile;	// save the path of the real texture

	if( !DoesFileExist(ID.filename) )
		RageException::Throw( "The sprite file '%s' points to a texture '%s' which doesn't exist.", m_sSpritePath.GetString(), ID.filename.GetString() );

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
				m_sSpritePath.GetString(), sFrameKey.GetString(), m_iStateToFrame[i], ID.filename.GetString(), m_pTexture->GetNumFrames() );
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


	return true;
}

void Sprite::UnloadTexture()
{
	if( m_pTexture != NULL )			// If there was a previous bitmap...
		TEXTUREMAN->UnloadTexture( m_pTexture );	// Unload it.
	m_pTexture = NULL;
}

bool Sprite::LoadFromTexture( RageTextureID ID )
{
	UnloadTexture();

	m_pTexture = TEXTUREMAN->LoadTexture( ID );
	ASSERT( m_pTexture != NULL );

	// the size of the sprite is the size of the image before it was scaled
	Sprite::m_size.x = (float)m_pTexture->GetSourceFrameWidth();
	Sprite::m_size.y = (float)m_pTexture->GetSourceFrameHeight();		

	// Assume the frames of this animation play in sequential order with 0.2 second delay.
	for( int i=0; i<m_pTexture->GetNumFrames(); i++ )
	{
		m_iStateToFrame[i] = i;
		m_fDelay[i] = 0.1f;
		m_iNumStates = i+1;
	}
		
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


void Sprite::DrawPrimitives()
{
	if( m_pTexture == NULL  &&  !m_bDrawIfTextureNull )
		return;

	if( m_pTexture  &&  m_pTexture->IsAMovie()  &&  m_pTexture->IsPlaying() )
		::Sleep( PREFSMAN->m_iMovieDecodeMS );	// let the movie decode a frame

	// use m_temp_* variables to draw the object
	RectF quadVerticies;

	switch( m_HorizAlign )
	{
	case align_top:		quadVerticies.left = 0;				quadVerticies.right = m_size.x;		break;
	case align_middle:	quadVerticies.left = -m_size.x/2;	quadVerticies.right = m_size.x/2;	break;
	case align_bottom:	quadVerticies.left = -m_size.x;		quadVerticies.right = 0;			break;
	default:		ASSERT( false );
	}

	switch( m_VertAlign )
	{
	case align_bottom:	quadVerticies.top = 0;				quadVerticies.bottom = m_size.y;	break;
	case align_middle:	quadVerticies.top = -m_size.y/2;	quadVerticies.bottom = m_size.y/2;	break;
	case align_top:		quadVerticies.top = -m_size.y;		quadVerticies.bottom = 0;			break;
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
		if( m_bUsingCustomTexCoords ) 
		{
			v[0].t = RageVector2( m_CustomTexCoords[2],	m_CustomTexCoords[3] );	// top left
			v[1].t = RageVector2( m_CustomTexCoords[0], m_CustomTexCoords[1] );	// bottom left
			v[2].t = RageVector2( m_CustomTexCoords[4],	m_CustomTexCoords[5] );	// bottom right
			v[3].t = RageVector2( m_CustomTexCoords[6],	m_CustomTexCoords[7] );	// top right

			DISPLAY->EnableTextureWrapping();
		} 
		else 
		{
			unsigned int uFrameNo = m_iStateToFrame[m_iCurState];
			const RectF *pTexCoordRect = m_pTexture->GetTextureCoordRect( uFrameNo );

			v[0].t = RageVector2( pTexCoordRect->left,	pTexCoordRect->top );		// top left
			v[1].t = RageVector2( pTexCoordRect->left,	pTexCoordRect->bottom );	// bottom left
			v[2].t = RageVector2( pTexCoordRect->right,	pTexCoordRect->bottom );	// bottom right
			v[3].t = RageVector2( pTexCoordRect->right,	pTexCoordRect->top );		// top right

			// if the texture has more than one frame, we're going to get border mess from the 
			// neighboring frame, so don't bother turning wrapping off.
			if( m_pTexture->GetNumFrames() == 1 )	
				DISPLAY->DisableTextureWrapping();
		}
	}

	DISPLAY->SetTextureModeModulate();
	if( m_bBlendAdd )
		DISPLAY->SetBlendModeAdd();
	else
		DISPLAY->SetBlendModeNormal();

	/* Draw if we're not fully transparent or the zbuffer is enabled (which ignores
	 * alpha). */
	if( m_temp.diffuse[0].a != 0 || DISPLAY->ZBufferEnabled())
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

		//////////////////////
		// render the diffuse pass
		//////////////////////
		v[0].c = m_temp.diffuse[2];	// bottom left
		v[1].c = m_temp.diffuse[0];	// top left
		v[2].c = m_temp.diffuse[3];	// bottom right
		v[3].c = m_temp.diffuse[1];	// top right
		DISPLAY->DrawQuad( v );
	}

	//////////////////////
	// render the glow pass
	//////////////////////
	if( m_temp.glow.a != 0 )
	{
		DISPLAY->SetTextureModeGlow();
		v[0].c = v[1].c = v[2].c = v[3].c = m_temp.glow;
		DISPLAY->DrawQuad( v );
	}
}


void Sprite::SetState( int iNewState )
{
	ASSERT( iNewState >= 0  &&  iNewState < m_iNumStates );
	CLAMP(iNewState, 0, m_iNumStates-1);
	m_iCurState = iNewState;
	m_fSecsIntoState = 0.0; 
}

void Sprite::SetCustomTextureRect( const RectF &new_texcoord_frect ) 
{ 
	m_bUsingCustomTexCoords = true;
	m_CustomTexCoords[0] = new_texcoord_frect.left;		m_CustomTexCoords[1] = new_texcoord_frect.bottom;	// bottom left
	m_CustomTexCoords[2] = new_texcoord_frect.left;		m_CustomTexCoords[3] = new_texcoord_frect.top;		// top left
	m_CustomTexCoords[4] = new_texcoord_frect.right;	m_CustomTexCoords[5] = new_texcoord_frect.bottom;	// bottom right
	m_CustomTexCoords[6] = new_texcoord_frect.right;	m_CustomTexCoords[7] = new_texcoord_frect.top;		// top right

}

void Sprite::SetCustomTextureCoords( float fTexCoords[8] ) // order: bottom left, top left, bottom right, top right
{ 
	m_bUsingCustomTexCoords = true;
	for( int i=0; i<8; i++ )
		m_CustomTexCoords[i] = fTexCoords[i]; 
}

void Sprite::GetCustomTextureCoords( float fTexCoordsOut[8] ) // order: bottom left, top left, bottom right, top right
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

void Sprite::StopUsingCustomCoords()
{
	m_bUsingCustomTexCoords = false;
}

