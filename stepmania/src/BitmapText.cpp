#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: BitmapText.h

 Desc: A font class that draws characters from a bitmap.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "BitmapText.h"
#include <assert.h>
#include "IniFile.h"



BitmapText::BitmapText()
{
	m_pFont = NULL;
	m_colorTop = D3DXCOLOR(1,1,1,1);
	m_colorBottom = D3DXCOLOR(1,1,1,1);

}

BitmapText::~BitmapText()
{
	delete m_pFont;
}

bool BitmapText::LoadFromFontName( CString sFontName )
{
	RageLog( "BitmapText::LoadFromFontName(%s)", sFontName );
	
	SAFE_DELETE( m_pFont );	// delete old font (if any)

	m_sFontName = sFontName;	// save font name

	m_pFont= new CBitmapFont( SCREEN->GetD3D(), SCREEN->GetDevice() );
	m_pFont->Load( BL_BITMAP, ssprintf("Fonts/%s.png",sFontName) );
	m_pFont->Load( BL_SIZES,  ssprintf("Fonts/%s.dat",sFontName) );

	return TRUE;
}

void BitmapText::Draw()
{
	RECT rect = m_pFont->GetTextRect(m_sText, 0, 0);
	float text_width	= RECTWIDTH( rect );
	float text_height	= RECTHEIGHT( rect );


	LPDIRECT3DDEVICE8 pd3dDevice = SCREEN->GetDevice();

	// calculate transforms
	D3DXMATRIX matWorld, matTemp;
	D3DXMatrixIdentity( &matWorld );		// initialize world
	//D3DXMatrixScaling( &matTemp, m_size.x, m_size.y, 1 );	// scale to the native height and width
	//matWorld *= matTemp;
	D3DXMatrixScaling( &matTemp, m_scale.x, m_scale.y, 1 );	// add in the zoom
	matWorld *= matTemp;
	D3DXMatrixRotationYawPitchRoll( &matTemp, m_rotation.y, m_rotation.x, m_rotation.z );	// add in the rotation
	matWorld *= matTemp;
	D3DXMatrixTranslation( &matTemp, m_pos.x, m_pos.y, 0 );	// add in the translation
	matWorld *= matTemp;
    pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

	D3DXMATRIX matView;
    D3DXMatrixIdentity( &matView );
	pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

	D3DXMATRIX matProj;
    D3DXMatrixOrthoOffCenterLH( &matProj, 0, 640, 480, 0, -100, 100 );
	pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

	m_pFont->BeginDraw();
	m_pFont->DrawTextEx(m_sText, -text_width/2, -text_height/2, m_colorTop, m_colorBottom, 1.0f);
	//m_pFont->DrawTextEx("Cool Effects!", 50, 80, 0xffffff00, 0xffff0000, 2.0f);
	//m_pFont->DrawTextEx("Cool Effects!", 50, 120, 0x1100ffff, 0x8800ffff, 2.0f);
	m_pFont->EndDraw();
	///////////////////////////////////////////////////////////////////////////////

}

