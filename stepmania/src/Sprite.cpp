#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: Sprite.cpp

 Desc: A bitmap actor that animates and moves around.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


#include "Sprite.h"
#include "RageTextureManager.h"
#include "IniFile.h"
#include <math.h>
#include "RageLog.h"

#include "PrefsManager.h"


Sprite::Sprite()
{
	m_pTexture = NULL;
	m_iNumStates = 0;
	m_iCurState = 0;
	m_bIsAnimating = TRUE;
	m_fSecsIntoState = 0.0;
	m_bUsingCustomTexCoords = false;
}


Sprite::~Sprite()
{
//	LOG->WriteLine( "Sprite Destructor" );

	TEXTUREMAN->UnloadTexture( m_sTexturePath ); 
}


bool Sprite::LoadFromTexture( CString sTexturePath, bool bForceReload, int iMipMaps, int iAlphaBits, bool bDither, bool bStretch )
{
	LOG->WriteLine( ssprintf("Sprite::LoadFromTexture(%s)", sTexturePath) );

	//Init();
	return LoadTexture( sTexturePath, bForceReload, iMipMaps, iAlphaBits, bDither, bStretch );
}

// Sprite file has the format:
//
// [Sprite]
// Texture=Textures\Logo.bmp
// Frame0000=0
// Delay0000=1.0
// Frame0001=3
// Delay0000=2.0
bool Sprite::LoadFromSpriteFile( CString sSpritePath, bool bForceReload, int iMipMaps, int iAlphaBits, bool bDither, bool bStretch )
{
	LOG->WriteLine( ssprintf("Sprite::LoadFromSpriteFile(%s)", sSpritePath) );

	//Init();

	m_sSpritePath = sSpritePath;


	// Split for the directory.  We'll need it below
	CString sFontDir, sFontFileName, sFontExtension;
	splitrelpath( m_sSpritePath, sFontDir, sFontFileName, sFontExtension );





	// read sprite file
	IniFile ini;
	ini.SetPath( m_sSpritePath );
	if( !ini.ReadFile() )
		throw RageException( ssprintf("Error opening Sprite file '%s'.", m_sSpritePath) );

	CString sTextureFile;
	ini.GetValue( "Sprite", "Texture", sTextureFile );
	if( sTextureFile == "" )
		throw RageException( ssprintf("Error reading  value 'Texture' from %s.", m_sSpritePath) );

	CString sTexturePath = sFontDir + sTextureFile;	// save the path of the new texture

	// Load the texture
	if( !LoadTexture( sTexturePath ) )
		return FALSE;


	// Read in frames and delays from the sprite file, 
	// overwriting the states that LoadFromTexture created.
	for( UINT i=0; i<MAX_SPRITE_STATES; i++ )
	{
		CString sStateNo;
		sStateNo.Format( "%u%u%u%u", (i%10000)/1000, (i%1000)/100, (i%100)/10, (i%10) );	// four digit state no

		CString sFrameKey( CString("Frame") + sStateNo );
		CString sDelayKey( CString("Delay") + sStateNo );
		
		m_iStateToFrame[i] = 0;
		if( !ini.GetValueI( "Sprite", sFrameKey, m_iStateToFrame[i] ) )
			break;
		if( m_iStateToFrame[i] >= m_pTexture->GetNumFrames() )
			throw RageException( "In '%s', %s is %d, but the texture %s only has %d frames.",
				m_sSpritePath, sFrameKey, m_iStateToFrame[i], sTexturePath, m_pTexture->GetNumFrames() );
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
		TEXTUREMAN->UnloadTexture( m_sTexturePath );	// Unload it.
	m_pTexture = NULL;
	m_sTexturePath = "";
}

bool Sprite::LoadTexture( CString sTexturePath, bool bForceReload, int iMipMaps, int iAlphaBits, bool bDither, bool bStretch )
{
	if( m_pTexture != NULL )			// If there was a previous bitmap...
		UnloadTexture();


	m_sTexturePath = sTexturePath;

	m_pTexture = TEXTUREMAN->LoadTexture( m_sTexturePath, bForceReload, iMipMaps, iAlphaBits, bDither, bStretch );
	assert( m_pTexture != NULL );

	// the size of the sprite is the size of the image before it was scaled
	SetWidth( (float)m_pTexture->GetSourceFrameWidth() );
	SetHeight( (float)m_pTexture->GetSourceFrameHeight() );		

	// Assume the frames of this animation play in sequential order with 0.2 second delay.
	for( int i=0; i<m_pTexture->GetNumFrames(); i++ )
	{
		m_iStateToFrame[i] = i;
		m_fDelay[i] = 0.1f;
		m_iNumStates = i+1;
	}
		
	return TRUE;
}



void Sprite::Update( float fDeltaTime )
{
	//PrintDebugInfo();

	Actor::Update( fDeltaTime );	// do tweening


	// update animation
	if( m_bIsAnimating )
	{
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




}


void Sprite::DrawPrimitives()
{
	// offset so that pixels are aligned to texels
	if( PREFSMAN->m_bHighDetail )	// 640x480
		DISPLAY->TranslateLocal( -0.5f, -0.5f, 0 );
	else		// 320x240
		DISPLAY->TranslateLocal( -0.5f, -0.5f, 0 );
//		DISPLAY->TranslateLocal( -1, -1, 0 );	// offset so that pixels are aligned to texels

	if( m_pTexture == NULL )
		return;
	
	// use m_temp_* variables to draw the object


	// make the object in logical units centered at the origin


	
	FRECT quadVerticies;

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


	LPDIRECT3DVERTEXBUFFER8 pVB = DISPLAY->GetVertexBuffer();
	RAGEVERTEX* v;
	pVB->Lock( 0, 0, (BYTE**)&v, 0 );

	v[0].p = D3DXVECTOR3( quadVerticies.left,	quadVerticies.bottom,	0 );	// bottom left
	v[1].p = D3DXVECTOR3( quadVerticies.left,	quadVerticies.top,		0 );	// top left
	v[2].p = D3DXVECTOR3( quadVerticies.right,	quadVerticies.bottom,	0 );	// bottom right
	v[3].p = D3DXVECTOR3( quadVerticies.right,	quadVerticies.top,		0 );	// top right


	if( m_bUsingCustomTexCoords ) 
	{
		v[0].t = D3DXVECTOR2( m_CustomTexCoords[0], m_CustomTexCoords[1] );	// bottom left
		v[1].t = D3DXVECTOR2( m_CustomTexCoords[2],	m_CustomTexCoords[3] );	// top left
		v[2].t = D3DXVECTOR2( m_CustomTexCoords[4],	m_CustomTexCoords[5] );	// bottom right
		v[3].t = D3DXVECTOR2( m_CustomTexCoords[6],	m_CustomTexCoords[7] );	// top right
	} 
	else 
	{
		UINT uFrameNo = m_iStateToFrame[m_iCurState];
		FRECT* pTexCoordRect = m_pTexture->GetTextureCoordRect( uFrameNo );

		v[0].t = D3DXVECTOR2( pTexCoordRect->left,	pTexCoordRect->bottom );	// bottom left
		v[1].t = D3DXVECTOR2( pTexCoordRect->left,	pTexCoordRect->top );		// top left
		v[2].t = D3DXVECTOR2( pTexCoordRect->right,	pTexCoordRect->bottom );	// bottom right
		v[3].t = D3DXVECTOR2( pTexCoordRect->right,	pTexCoordRect->top );		// top right
	}


	pVB->Unlock();


	// Set the stage...
	LPDIRECT3DDEVICE8 pd3dDevice = DISPLAY->GetDevice();
	pd3dDevice->SetTexture( 0, m_pTexture->GetD3DTexture() );

	pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  m_bBlendAdd ? D3DBLEND_ONE : D3DBLEND_SRCALPHA );
	pd3dDevice->SetRenderState( D3DRS_DESTBLEND, m_bBlendAdd ? D3DBLEND_ONE : D3DBLEND_INVSRCALPHA );

	pd3dDevice->SetVertexShader( D3DFVF_RAGEVERTEX );
	pd3dDevice->SetStreamSource( 0, pVB, sizeof(RAGEVERTEX) );



	if( m_temp_colorDiffuse[0].a != 0 )
	{
		//////////////////////
		// render the shadow
		//////////////////////
		if( m_bShadow )
		{
			DISPLAY->PushMatrix();
			DISPLAY->TranslateLocal( m_fShadowLength, m_fShadowLength, 0 );	// shift by 5 units

			pVB->Lock( 0, 0, (BYTE**)&v, 0 );

			v[0].color = v[1].color = v[2].color = v[3].color = D3DXCOLOR(0,0,0,0.5f*m_temp_colorDiffuse[0].a);	// semi-transparent black
			
			pVB->Unlock();

			pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
			
			pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

			DISPLAY->PopMatrix();
		}


		//////////////////////
		// render the diffuse pass
		//////////////////////
		pVB->Lock( 0, 0, (BYTE**)&v, 0 );

		v[0].color = m_temp_colorDiffuse[2];	// bottom left
		v[1].color = m_temp_colorDiffuse[0];	// top left
		v[2].color = m_temp_colorDiffuse[3];	// bottom right
		v[3].color = m_temp_colorDiffuse[1];	// top right
		
		pVB->Unlock();

		
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );//bBlendAdd ? D3DTOP_ADD : D3DTOP_MODULATE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );//bBlendAdd ? D3DTOP_ADD : D3DTOP_MODULATE );


		// finally!  Pump those triangles!	
		pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
	}


	//////////////////////
	// render the add pass
	//////////////////////
	if( m_temp_colorAdd.a != 0 )
	{
		pVB->Lock( 0, 0, (BYTE**)&v, 0 );

		v[0].color = v[1].color = v[2].color = v[3].color = m_temp_colorAdd;
		
		pVB->Unlock();

		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
		
		pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
	}
}


void Sprite::SetState( int iNewState )
{
	ASSERT( iNewState >= 0  &&  iNewState < m_iNumStates );
	if( iNewState < 0 )						iNewState = 0;
	else if( iNewState >= m_iNumStates )	iNewState = m_iNumStates-1;
	m_iCurState = iNewState;
	m_fSecsIntoState = 0.0; 
}

void Sprite::SetCustomTextureRect( FRECT new_texcoord_frect ) 
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


void Sprite::SetCustomImageRect( FRECT rectImageCoords )
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

