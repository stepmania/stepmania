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
	m_HorizAlign = align_center;
	m_VertAlign = align_middle;


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

	//Init();
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

	//Init();

	m_sSpritePath = sSpritePath;


	// Split for the directory.  We'll need it below
	CString sFontDir, sFontFileName, sFontExtension;
	splitrelpath( m_sSpritePath, sFontDir, sFontFileName, sFontExtension );





	// read sprite file
	IniFile ini;
	ini.SetPath( m_sSpritePath );
	if( !ini.ReadFile() )
		RageError( ssprintf("Error opening Sprite file '%s'.", m_sSpritePath) );

	CString sTextureFile = ini.GetValue( "Sprite", "Texture" );
	if( sTextureFile == "" )
		RageError( ssprintf("Error reading  value 'Texture' from %s.", m_sSpritePath) );

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
	{
		m_uNumStates = 1;
		m_uFrame[0] = 0;
		m_fDelay[0] = 10;
	}


	return true;
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


void Sprite::RenderPrimitives()
{

	if( m_pTexture == NULL )
		return;
	
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
			colorDiffuse[i] = m_bTweeningTowardEndColor ? m_effect_colorDiffuse1 : m_effect_colorDiffuse2;
		}
		break;
	case camelion:
		{
		for(int i=0; i<4; i++)
			colorDiffuse[i] = m_effect_colorDiffuse1*m_fPercentBetweenColors + m_effect_colorDiffuse2*(1.0f-m_fPercentBetweenColors);
		}
		break;
	case glowing:
		colorAdd = m_effect_colorAdd1*m_fPercentBetweenColors + m_effect_colorAdd2*(1.0f-m_fPercentBetweenColors);
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


	
	FRECT quadVerticies;

	switch( m_HorizAlign )
	{
	case align_top:		quadVerticies.left = 0;				quadVerticies.right = m_size.x;		break;
	case align_middle:	quadVerticies.left = -m_size.x/2;	quadVerticies.right = m_size.x/2;	break;
	case align_bottom:	quadVerticies.left = -m_size.x;		quadVerticies.right = 0;			break;
	default:		ASSERT( true );
	}

	switch( m_VertAlign )
	{
	case align_bottom:	quadVerticies.top = 0;				quadVerticies.bottom = m_size.y;	break;
	case align_middle:	quadVerticies.top = -m_size.y/2;	quadVerticies.bottom = m_size.y/2;	break;
	case align_top:		quadVerticies.top = -m_size.y;		quadVerticies.bottom = 0;			break;
	default:		ASSERT( true );
	}


	LPDIRECT3DVERTEXBUFFER8 pVB = SCREEN->GetVertexBuffer();
	CUSTOMVERTEX* v;
	pVB->Lock( 0, 0, (BYTE**)&v, 0 );

	// shift by one pixel because sprites are not aligned (why?!?)
	v[0].p = D3DXVECTOR3( quadVerticies.left+1,		quadVerticies.bottom,	0 );	// bottom left
	v[1].p = D3DXVECTOR3( quadVerticies.left+1,		quadVerticies.top,		0 );	// top left
	v[2].p = D3DXVECTOR3( quadVerticies.right+1,	quadVerticies.bottom,	0 );	// bottom right
	v[3].p = D3DXVECTOR3( quadVerticies.right+1,	quadVerticies.top,		0 );	// top right


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
	LPDIRECT3DDEVICE8 pd3dDevice = SCREEN->GetDevice();
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
			SCREEN->PushMatrix();
			SCREEN->Translate( 5, 5, 0 );	// shift by 5 units

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

			SCREEN->PopMatrix();
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

