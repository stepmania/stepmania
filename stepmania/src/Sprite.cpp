#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: Sprite.cpp

 Desc: A bitmap actor that animates and moves around.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


#include "Sprite.h"
#include "RageTextureManager.h"
#include "GameInfo.h"
#include "IniFile.h"
#include <assert.h>
#include <math.h>


Sprite::Sprite()
{
	Init();
}

void Sprite::Init()
{
	Actor::Init();

	m_pTexture = NULL;
	m_uNumStates = 0;
	m_uCurState = 0;
	m_bIsAnimating = TRUE;
	m_fSecsIntoState = 0.0;
	m_bUsingCustomTexCoords = false;
	m_Effect =  no_effect ;
	m_fPercentBetweenColors = 0.0f;
	m_bTweeningTowardEndColor = true;
	m_fDeltaPercentPerSecond = 1.0f;
	m_fWagRadians =  0.2f;
	m_fWagPeriod =  2.0f;
	m_fWagTimer =  0.0f;
	m_fSpinSpeed =  2.0f;
	m_fVibrationDistance =  5.0f;
	m_bVisibleThisFrame =  FALSE;

	if( GAMEINFO )
		m_bHasShadow = GAMEINFO->m_GameOptions.m_bShadows;
	else
		m_bHasShadow = true;


	m_bBlendAdd = false;
}

Sprite::~Sprite()
{
//	RageLog( "Sprite Destructor" );

	TM->UnloadTexture( m_sTexturePath ); 
}


bool Sprite::LoadFromTexture( CString sTexturePath )
{
	RageLog( ssprintf("Sprite::LoadFromTexture(%s)", sTexturePath) );

	Init();
	return LoadTexture( sTexturePath );
}

// Sprite file has the format:
//
// [Sprite]
// Texture=Textures\Logo.bmp
// Frame0000=0
// Delay0000=1.0
// Frame0001=3
// Delay0000=2.0
bool Sprite::LoadFromSpriteFile( CString sSpritePath )
{
	RageLog( ssprintf("Sprite::LoadFromSpriteFile(%s)", sSpritePath) );

	Init();

	m_sSpritePath = sSpritePath;

	// read sprite file
	IniFile ini;
	ini.SetPath( m_sSpritePath );
	if( !ini.ReadFile() )
		RageError( ssprintf("Error opening Sprite file '%s'.", m_sSpritePath) );

	CString sTexturePath = ini.GetValue( "Sprite", "Texture" );
	if( sTexturePath == "" )
		RageError( ssprintf("Error reading  value 'Texture' from %s.", m_sSpritePath) );

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
		
		m_uFrame[i] = ini.GetValueI( "Sprite", sFrameKey );
		if( m_uFrame[i] >= m_pTexture->GetNumFrames() )
			RageError( ssprintf("In '%s', %s is %d, but the texture %s only has %d frames.",
						m_sSpritePath, sFrameKey, m_uFrame[i], sTexturePath, m_pTexture->GetNumFrames()) );
		m_fDelay[i] = (float)ini.GetValueF( "Sprite", sDelayKey );

		if( m_uFrame[i] == 0  &&  m_fDelay[i] > -0.00001f  &&  m_fDelay[i] < 0.00001f )	// both values are empty
			break;

		m_uNumStates = i+1;
	}

	if( m_uNumStates == 0 )
		RageError( ssprintf("Failed to find at least one state in %s.", m_sSpritePath) );

	return TRUE;
}

bool Sprite::LoadTexture( CString sTexturePath )
{
	if( m_sTexturePath != "" )			// If there was a previous bitmap...
		TM->UnloadTexture( m_sTexturePath );	// Unload it.


	m_sTexturePath = sTexturePath;

	m_pTexture = TM->LoadTexture( m_sTexturePath );
	assert( m_pTexture != NULL );

	// the size of the sprite is the size of the image before it was scaled
	SetWidth( (float)m_pTexture->GetSourceFrameWidth() );
	SetHeight( (float)m_pTexture->GetSourceFrameHeight() );		

	// Assume the frames of this animation play in sequential order with 0.2 second delay.
	for( UINT i=0; i<m_pTexture->GetNumFrames(); i++ )
	{
		m_uFrame[i] = i;
		m_fDelay[i] = 0.1f;
		m_uNumStates = i+1;
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

		if( m_fSecsIntoState > m_fDelay[m_uCurState] )		// it's time to switch frames
		{
			// increment frame and reset the counter
			m_fSecsIntoState -= m_fDelay[m_uCurState];		// leave the left over time for the next frame
			m_uCurState ++;
			if( m_uCurState >= m_uNumStates )
				m_uCurState = 0;
		}
	}




}


void Sprite::Draw()
{
	D3DXVECTOR2 pos				= m_pos;
	D3DXVECTOR3 rotation		= m_rotation;
	D3DXVECTOR2 scale			= m_scale;

	// update properties based on SpriteEffects 
	switch( m_Effect )
	{
	case no_effect:
		break;
	case blinking:
		break;
	case camelion:
		break;
	case glowing:
		break;
	case wagging:
		rotation.z = m_fWagRadians * (float)sin( 
			(m_fWagTimer / m_fWagPeriod)	// percent through wag
			* 2.0 * D3DX_PI );
		break;
	case spinning:
		// nothing special needed
		break;
	case vibrating:
		pos.x += m_fVibrationDistance * randomf(-1.0f, 1.0f) * GetZoom();
		pos.y += m_fVibrationDistance * randomf(-1.0f, 1.0f) * GetZoom();
		break;
	case flickering:
		break;
	}


	LPDIRECT3DDEVICE8 pd3dDevice = SCREEN->GetDevice();

	// calculate and apply world transform
	D3DXMATRIX matOriginalWorld, matNewWorld, matTemp;
    pd3dDevice->GetTransform( D3DTS_WORLD, &matOriginalWorld );	// save the original world matrix

	matNewWorld = matOriginalWorld;		// initialize the matrix we're about to build to transform into this Frame's coord space

	D3DXMatrixTranslation( &matTemp, pos.x, pos.y, 0 );	// add in the translation
	matNewWorld = matTemp * matNewWorld;
	D3DXMatrixTranslation( &matTemp, -0.5f, -0.5f, 0 );		// shift to align texels with pixels
	matNewWorld = matTemp * matNewWorld;
	D3DXMatrixScaling( &matTemp, scale.x, scale.y, 1 );	// add in the zoom
	matNewWorld = matTemp * matNewWorld;
	D3DXMatrixRotationYawPitchRoll( &matTemp, rotation.y, rotation.x, rotation.z );	// add in the rotation
	matNewWorld = matTemp * matNewWorld;

    pd3dDevice->SetTransform( D3DTS_WORLD, &matNewWorld );	// transform to local coordinates

	

	if( m_pTexture != NULL )
	{
	
		D3DXCOLOR	colorDiffuse[4];
		for(int i=0; i<4; i++)
			colorDiffuse[i] = m_colorDiffuse[i];
		D3DXCOLOR	colorAdd		= m_colorAdd;

		// update properties based on SpriteEffects 
		switch( m_Effect )
		{
		case no_effect:
			break;
		case blinking:
			{
			for(int i=0; i<4; i++)
				colorDiffuse[i] = m_bTweeningTowardEndColor ? m_start_colorDiffuse[i] : m_end_colorDiffuse[i];
			}
			break;
		case camelion:
			{
			for(int i=0; i<4; i++)
				colorDiffuse[i] = m_start_colorDiffuse[i]*m_fPercentBetweenColors + m_end_colorDiffuse[i]*(1.0f-m_fPercentBetweenColors);
			}
			break;
		case glowing:
			colorAdd = m_start_colorAdd*m_fPercentBetweenColors + m_end_colorAdd*(1.0f-m_fPercentBetweenColors);
			break;
		case wagging:
			break;
		case spinning:
			// nothing special needed
			break;
		case vibrating:
			break;
		case flickering:
			m_bVisibleThisFrame = !m_bVisibleThisFrame;
			if( !m_bVisibleThisFrame )
				for(int i=0; i<4; i++)
					colorDiffuse[i] = D3DXCOLOR(0,0,0,0);		// don't draw the frame
			break;
		}


		// make the object in logical units centered at the origin
		LPDIRECT3DVERTEXBUFFER8 pVB = SCREEN->GetVertexBuffer();
		CUSTOMVERTEX* v;
		pVB->Lock( 0, 0, (BYTE**)&v, 0 );

		float fHalfSizeX = m_size.x/2;
		float fHalfSizeY = m_size.y/2;

		v[0].p = D3DXVECTOR3( -fHalfSizeX,	 fHalfSizeY,	0 );	// bottom left
		v[1].p = D3DXVECTOR3( -fHalfSizeX,	-fHalfSizeY,	0 );	// top left
		v[2].p = D3DXVECTOR3(  fHalfSizeX,	 fHalfSizeY,	0 );	// bottom right
		v[3].p = D3DXVECTOR3(  fHalfSizeX,	-fHalfSizeY,	0 );	// top right


		if( m_bUsingCustomTexCoords ) 
		{
			v[0].tu = m_CustomTexCoords[0];		v[0].tv = m_CustomTexCoords[1];	// bottom left
			v[1].tu = m_CustomTexCoords[2];		v[1].tv = m_CustomTexCoords[3];	// top left
			v[2].tu = m_CustomTexCoords[4];		v[2].tv = m_CustomTexCoords[5];	// bottom right
			v[3].tu = m_CustomTexCoords[6];		v[3].tv = m_CustomTexCoords[7];	// top right
		} 
		else 
		{
			UINT uFrameNo = m_uFrame[m_uCurState];
			FRECT* pTexCoordRect = m_pTexture->GetTextureCoordRect( uFrameNo );

			v[0].tu = pTexCoordRect->left;		v[0].tv = pTexCoordRect->bottom;	// bottom left
			v[1].tu = pTexCoordRect->left;		v[1].tv = pTexCoordRect->top;		// top left
			v[2].tu = pTexCoordRect->right;		v[2].tv = pTexCoordRect->bottom;	// bottom right
			v[3].tu = pTexCoordRect->right;		v[3].tv = pTexCoordRect->top;		// top right
		}



		pVB->Unlock();


		// Set the stage...
		pd3dDevice->SetTexture( 0, m_pTexture->GetD3DTexture() );

		pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
		pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
		//pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  bBlendAdd ? D3DBLEND_ONE : D3DBLEND_SRCALPHA );
		//pd3dDevice->SetRenderState( D3DRS_DESTBLEND, bBlendAdd ? D3DBLEND_ONE : D3DBLEND_INVSRCALPHA );
		pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  m_bBlendAdd ? D3DBLEND_ONE : D3DBLEND_SRCALPHA );
		pd3dDevice->SetRenderState( D3DRS_DESTBLEND, m_bBlendAdd ? D3DBLEND_ONE : D3DBLEND_INVSRCALPHA );

		pd3dDevice->SetVertexShader( D3DFVF_CUSTOMVERTEX );
		pd3dDevice->SetStreamSource( 0, pVB, sizeof(CUSTOMVERTEX) );



		if( colorDiffuse[0].a != 0 )
		{
			//////////////////////
			// render the shadow
			//////////////////////
			if( m_bHasShadow )
			{
				D3DXMATRIX matOriginalWorld, matNewWorld, matTemp;
				pd3dDevice->GetTransform( D3DTS_WORLD, &matOriginalWorld );	// save the original world matrix
				matNewWorld = matOriginalWorld;

				D3DXMatrixTranslation( &matTemp, 5, 5, 0 );	// shift by 5 units
				matNewWorld = matTemp * matNewWorld;
				pd3dDevice->SetTransform( D3DTS_WORLD, &matNewWorld );	// transform to local coordinates

				pVB->Lock( 0, 0, (BYTE**)&v, 0 );

				v[0].color = v[1].color = v[2].color = v[3].color = D3DXCOLOR(0,0,0,0.5f*colorDiffuse[0].a);	// semi-transparent black
				
				pVB->Unlock();

				pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
				pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
				pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
				pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
				pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
				pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
				
				pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

				pd3dDevice->SetTransform( D3DTS_WORLD, &matOriginalWorld );	// restore the original world matrix

			}


			//////////////////////
			// render the diffuse pass
			//////////////////////
			pVB->Lock( 0, 0, (BYTE**)&v, 0 );

			v[0].color = colorDiffuse[2];	// bottom left
			v[1].color = colorDiffuse[0];	// top left
			v[2].color = colorDiffuse[3];	// bottom right
			v[3].color = colorDiffuse[1];	// top right
			
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
		if( colorAdd.a != 0 )
		{
			pVB->Lock( 0, 0, (BYTE**)&v, 0 );

			v[0].color = v[1].color = v[2].color = v[3].color = colorAdd;
			
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
	
	
	pd3dDevice->SetTransform( D3DTS_WORLD, &matOriginalWorld );	// restore the original world matrix

}


void Sprite::SetState( UINT uNewState )
{
	ASSERT( uNewState >= 0  &&  uNewState < m_uNumStates );
	m_uCurState = uNewState;
	m_fSecsIntoState = 0.0; 
}

void Sprite::SetCustomSrcRect( FRECT new_texcoord_frect ) 
{ 
	m_bUsingCustomTexCoords = true;
	m_CustomTexCoords[0] = new_texcoord_frect.left;		m_CustomTexCoords[1] = new_texcoord_frect.bottom;	// bottom left
	m_CustomTexCoords[2] = new_texcoord_frect.left;		m_CustomTexCoords[3] = new_texcoord_frect.top;		// top left
	m_CustomTexCoords[4] = new_texcoord_frect.right;	m_CustomTexCoords[5] = new_texcoord_frect.bottom;	// bottom right
	m_CustomTexCoords[6] = new_texcoord_frect.right;	m_CustomTexCoords[7] = new_texcoord_frect.top;		// top right

}

void Sprite::SetCustomTexCoords( float fTexCoords[8] ) // order: bottom left, top left, bottom right, top right
{ 
	m_bUsingCustomTexCoords = true;
	for( int i=0; i<8; i++ )
		m_CustomTexCoords[i] = fTexCoords[i]; 
}

