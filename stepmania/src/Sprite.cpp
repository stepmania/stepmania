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
	m_bUsingCustomTexCoordRect = FALSE ;
	m_Effect =  no_effect ;
	m_fPercentBetweenColors = 0.0f ;
	m_bTweeningTowardEndColor = TRUE ;
	m_fDeltaPercentPerSecond = 1.0 ;
	m_fWagRadians =  0.2f ;
	m_fWagPeriod =  2.0f ;
	m_fWagTimer =  0.0f ;
	m_fSpinSpeed =  2.0f ;
	m_fVibrationDistance =  5.0f ;
	m_bVisibleThisFrame =  FALSE;
}

Sprite::~Sprite()
{
//	RageLog( "Sprite Destructor" );

	TM->UnloadTexture( m_sTexturePath ); 
}


BOOL Sprite::LoadFromTexture( CString sTexturePath )
{
	RageLog( ssprintf("Sprite::LoadFromTexture(%s)", sTexturePath) );

	Init();
	return LoadTexture( sTexturePath );
}

// Sprite file has the format:
//
// [Sprite]
// Texture=Textures\Logo 1x1.bmp
// Frame0000=0
// Delay0000=1.0
// Frame0001=3
// Delay0000=2.0
BOOL Sprite::LoadFromSpriteFile( CString sSpritePath )
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

BOOL Sprite::LoadTexture( CString sTexturePath )
{
	if( m_sTexturePath != "" )			// If there was a previous bitmap...
		TM->UnloadTexture( m_sTexturePath );	// Unload it.


	m_sTexturePath = sTexturePath;

	m_pTexture = TM->LoadTexture( m_sTexturePath );
	assert( m_pTexture != NULL );

	// the size of the sprite is the size of the image before it was scaled
	SetWidth( (FLOAT)m_pTexture->GetSourceFrameWidth() );
	SetHeight( (FLOAT)m_pTexture->GetSourceFrameHeight() );		

	// Assume the frames of this animation play in sequential order with 0.2 second delay.
	for( UINT i=0; i<m_pTexture->GetNumFrames(); i++ )
	{
		m_uFrame[i] = i;
		m_fDelay[i] = 0.1f;
		m_uNumStates = i+1;
	}
		
	return TRUE;
}


void Sprite::PrintDebugInfo()
{
//	Actor::PrintDebugInfo();

	RageLog( "Sprite::PrintDebugInfo()" );
	RageLog( "m_uNumStates: %u, m_uCurState: %u, m_fSecsIntoState: %f", 
		      m_uNumStates, m_uCurState, m_fSecsIntoState );
}


void Sprite::Update( const FLOAT &fDeltaTime )
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

	// update SpriteEffect
	switch( m_Effect )
	{
	case no_effect:
		break;
	case blinking:
	case camelion:
	case glowing:
		if( m_bTweeningTowardEndColor ) {
			m_fPercentBetweenColors += m_fDeltaPercentPerSecond * fDeltaTime;
			if( m_fPercentBetweenColors > 1.0f ) {
				m_fPercentBetweenColors = 1.0f;
				m_bTweeningTowardEndColor = FALSE;
			}
		}
		else {		// !m_bTweeningTowardEndColor
			m_fPercentBetweenColors -= m_fDeltaPercentPerSecond * fDeltaTime;
			if( m_fPercentBetweenColors < 0.0f ) {
				m_fPercentBetweenColors = 0.0f;
				m_bTweeningTowardEndColor = TRUE;
			}
		}
	case wagging:
		m_fWagTimer += fDeltaTime;
		if( m_fWagTimer > m_fWagPeriod )
			m_fWagTimer -= m_fWagPeriod;
		break;
	case spinning:
		FLOAT rotation;
		rotation = GetRotation();
		rotation += m_fSpinSpeed * fDeltaTime;
		if( rotation > 2.0f * D3DX_PI )
			rotation -= 2.0f * D3DX_PI;
		else if( rotation < 0.0f )
			rotation += 2.0f * D3DX_PI;
		SetRotation( rotation );
		break;
	case vibrating:
		break;
	case flickering:
		break;
	}


}


void Sprite::Draw()
{
	if( m_pTexture == NULL )
		return;
	
	FRECT* pTexCoordRect;	// the texture coordinates of the frame we're going to use
	if( m_bUsingCustomTexCoordRect ) {
		pTexCoordRect = &m_CustomTexCoordRect;
	} else {
		UINT uFrameNo = m_uFrame[m_uCurState];
		pTexCoordRect = m_pTexture->GetTextureCoordRect( uFrameNo );
	}

	D3DXCOLOR	color	= m_color;
	D3DXVECTOR2 pos		= m_pos;
	D3DXVECTOR3 rotation = m_rotation;
	D3DXVECTOR2 scale	= m_scale;
	bool bBlendAdd		= false;

	// update properties based on SpriteEffects 
	switch( m_Effect )
	{
	case no_effect:
		break;
	case blinking:
		color = m_bTweeningTowardEndColor ? m_start_color : m_end_color;
		break;
	case camelion:
		color = m_start_color*m_fPercentBetweenColors + m_end_color*(1.0f-m_fPercentBetweenColors);
		break;
	case glowing:
		color = m_start_color*m_fPercentBetweenColors + m_end_color*(1.0f-m_fPercentBetweenColors);
		bBlendAdd = true;
		break;
	case wagging:
		rotation.z = m_fWagRadians * (FLOAT)sin( 
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
		m_bVisibleThisFrame = !m_bVisibleThisFrame;
		if( m_bVisibleThisFrame )
			return;		// don't draw the frame
		break;
	}


	// make a 1x1 rect centered at the origin
	LPDIRECT3DVERTEXBUFFER8 pVB = SCREEN->GetVertexBuffer();
	CUSTOMVERTEX* v;
	pVB->Lock( 0, 0, (BYTE**)&v, 0 );

	v[0].p = D3DXVECTOR3( -0.5f,	 0.5f,		0 );	// bottom left
	v[1].p = D3DXVECTOR3( -0.5f,	-0.5f,		0 );	// top left
	v[2].p = D3DXVECTOR3(  0.5f,	 0.5f,		0 );	// bottom right
	v[3].p = D3DXVECTOR3(  0.5f,	-0.5f,		0 );	// top right

	v[0].tu = pTexCoordRect->left;		v[0].tv = pTexCoordRect->bottom;	// bottom left
	v[1].tu = pTexCoordRect->left;		v[1].tv = pTexCoordRect->top;		// top left
	v[2].tu = pTexCoordRect->right;		v[2].tv = pTexCoordRect->bottom;	// bottom right
	v[3].tu = pTexCoordRect->right;		v[3].tv = pTexCoordRect->top;		// top right

	v[0].color = v[1].color = v[2].color = v[3].color = color;
	
	pVB->Unlock();


	// set texture and alpha properties
	LPDIRECT3DDEVICE8 pd3dDevice = SCREEN->GetDevice();
    pd3dDevice->SetTexture( 0, m_pTexture->GetD3DTexture() );

	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   bBlendAdd ? D3DTOP_ADD : D3DTOP_MODULATE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   bBlendAdd ? D3DTOP_ADD : D3DTOP_MODULATE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
	//pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  bBlendAdd ? D3DBLEND_ONE : D3DBLEND_SRCALPHA );
	//pd3dDevice->SetRenderState( D3DRS_DESTBLEND, bBlendAdd ? D3DBLEND_ONE : D3DBLEND_INVSRCALPHA );
	pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
	pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );


	// calculate transforms
	D3DXMATRIX matWorld, matTemp;
	D3DXMatrixIdentity( &matWorld );		// initialize world
	D3DXMatrixScaling( &matTemp, m_size.x, m_size.y, 1 );	// scale to the native height and width
	matWorld *= matTemp;
	D3DXMatrixScaling( &matTemp, scale.x, scale.y, 1 );	// add in the zoom
	matWorld *= matTemp;
	D3DXMatrixRotationYawPitchRoll( &matTemp, rotation.y, rotation.x, rotation.z );	// add in the rotation
	matWorld *= matTemp;
	D3DXMatrixTranslation( &matTemp, pos.x, pos.y, 0 );	// add in the translation
	matWorld *= matTemp;
	D3DXMatrixTranslation( &matTemp, -0.5f, -0.5f, 0 );		// shift to align texels with pixels
	matWorld *= matTemp;
    pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

	D3DXMATRIX matView;
    D3DXMatrixIdentity( &matView );
	pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

	D3DXMATRIX matProj;
    D3DXMatrixOrthoOffCenterLH( &matProj, 0, 640, 480, 0, -100, 100 );
	pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );


	// finally!  Pump those triangles!	
	pd3dDevice->SetVertexShader( D3DFVF_CUSTOMVERTEX );
	pd3dDevice->SetStreamSource( 0, pVB, sizeof(CUSTOMVERTEX) );
	pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );




/*  WORKS!

	LPDIRECT3DVERTEXBUFFER8 pVB2        = NULL; // Buffer to hold vertices

	// A structure for our custom vertex type
	struct CUSTOMVERTEX
	{
		FLOAT x, y, z;      // The untransformed, 3D position for the vertex
		DWORD color;        // The vertex color
	};

	// Our custom FVF, which describes our custom vertex structure
	#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

    // Initialize three vertices for rendering a triangle
    CUSTOMVERTEX g_Vertices[] =
    {
        { -15.0f,-15.0f, 0.0f, 0xffff0000, },
        {  15.0f,-15.0f, 0.0f, 0xff0000ff, },
        {  00.0f, 15.0f, 0.0f, 0xffffffff, },
    };

    // Create the vertex buffer.
    if( FAILED( pd3dDevice->CreateVertexBuffer( 3*sizeof(CUSTOMVERTEX),
                                                  0, D3DFVF_CUSTOMVERTEX,
                                                  D3DPOOL_DEFAULT, &pVB2 ) ) )
    {
        return;
    }

    // Turn off culling, so we see the front and back of the triangle
    pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    // Turn off D3D lighting, since we are providing our own vertex colors
    pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );


    // Fill the vertex buffer.
    void* pVertices;
    if( FAILED( pVB2->Lock( 0, sizeof(g_Vertices), (BYTE**)&pVertices, 0 ) ) )
        return;
    memcpy( pVertices, g_Vertices, sizeof(g_Vertices) );
    pVB2->Unlock();


    // For our world matrix, we will just rotate the object about the y-axis.
    //D3DXMATRIX matWorld;
    D3DXMatrixRotationY( &matWorld, GetTickCount()/150.0f );
    pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    // Set up our view matrix. A view matrix can be defined given an eye point,
    // a point to lookat, and a direction for which way is up. Here, we set the
    // eye five units back along the z-axis and up three units, look at the
    // origin, and define "up" to be in the y-direction.
    //D3DXMATRIX matView;
    //D3DXMatrixLookAtLH( &matView, &D3DXVECTOR3( 0.0f, 3.0f,-5.0f ),
    //                              &D3DXVECTOR3( 0.0f, 0.0f, 0.0f ),
    //                              &D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) );
    D3DXMatrixIdentity( &matView );
	pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

    // For the projection matrix, we set up a perspective transform (which
    // transforms geometry from 3D view space to 2D viewport space, with
    // a perspective divide making objects smaller in the distance). To build
    // a perpsective transform, we need the field of view (1/4 pi is common),
    // the aspect ratio, and the near and far clipping planes (which define at
    // what distances geometry should be no longer be rendered).
    //D3DXMATRIX matProj;
    //D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, 1.0f, 1.0f, 100.0f );
    //D3DXMatrixOrthoLH( &matProj, 640, -480, -100, 100 );
    D3DXMatrixOrthoOffCenterLH( &matProj, 0, 320, 480, 0, -100, 100 );
//    D3DXMatrixOrthoOffCenterLH( &matProj, -20, 20, 20, -20, -100, 100 );
   
	pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    // Render the vertex buffer contents
    pd3dDevice->SetStreamSource( 0, pVB2, sizeof(CUSTOMVERTEX) );
    pd3dDevice->SetVertexShader( D3DFVF_CUSTOMVERTEX );
    pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 1 );




    if( pVB2 != NULL )
        pVB2->Release();
*/





}


void Sprite::SetState( UINT uNewState )
{
	ASSERT( uNewState >= 0  &&  uNewState < m_uNumStates );
	m_uCurState = uNewState;
	m_fSecsIntoState = 0.0; 
}

void Sprite::SetCustomSrcRect(	FRECT new_texcoord_frect ) 
{ 
	m_bUsingCustomTexCoordRect = true;
	m_CustomTexCoordRect = new_texcoord_frect; 
}

void Sprite::SetEffectNone()
{
	m_Effect = no_effect;
	//m_color = D3DXCOLOR( 1.0,1.0,1.0,1.0 );
}

void Sprite::SetEffectBlinking( FLOAT fDeltaPercentPerSecond, D3DXCOLOR Color, D3DXCOLOR Color2 )
{
	m_Effect = blinking;
	m_start_color = Color;
	m_end_color = Color2;
	//m_fPercentBetweenColors = 0.0;
	//m_bTweeningTowardEndColor = TRUE;
	m_fDeltaPercentPerSecond = fDeltaPercentPerSecond;
}

void Sprite::SetEffectCamelion( FLOAT fDeltaPercentPerSecond, D3DXCOLOR Color, D3DXCOLOR Color2 )
{
	m_Effect = camelion;
	m_start_color = Color;
	m_end_color = Color2;
	//m_fPercentBetweenColors = 0.0;
	//m_bTweeningTowardEndColor = TRUE;
	m_fDeltaPercentPerSecond = fDeltaPercentPerSecond;
}

void Sprite::SetEffectGlowing( FLOAT fDeltaPercentPerSecond, D3DXCOLOR Color, D3DXCOLOR Color2 )
{
	m_Effect = glowing;
	m_start_color = Color;
	m_end_color = Color2;
	//m_fPercentBetweenColors = 0.0;
	//m_bTweeningTowardEndColor = TRUE;
	m_fDeltaPercentPerSecond = fDeltaPercentPerSecond;
}

void Sprite::SetEffectWagging(	FLOAT fWagRadians, FLOAT fWagPeriod )
{
	m_Effect = wagging;
	m_fWagRadians = fWagRadians;
	m_fWagPeriod = fWagPeriod;
}

void Sprite::SetEffectSpinning(	FLOAT fSpinSpeed /*radians per second*/ )
{
	m_Effect = spinning;
	m_fSpinSpeed = fSpinSpeed;
}

void Sprite::SetEffectVibrating( FLOAT fVibrationDistance )
{
	m_Effect = vibrating;
	m_fVibrationDistance = fVibrationDistance;
}

void Sprite::SetEffectFlickering()
{
	m_Effect = flickering;
}
