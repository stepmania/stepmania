#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: LifeMeterBar

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "LifeMeterBar.h"
#include "PrefsManager.h"
#include "RageException.h"
#include "RageTimer.h"
#include "GameState.h"


//
// Important!!!!  Do not use these macros during gameplay.  They return very slowly.  Cache them in a member.
//
#define METER_WIDTH			THEME->GetMetricI("LifeMeterBar","MeterWidth")
#define METER_HEIGHT		THEME->GetMetricI("LifeMeterBar","MeterHeight")
#define DANGER_THRESHOLD	THEME->GetMetricF("LifeMeterBar","DangerThreshold")
#define NUM_CHAMBERS		THEME->GetMetricI("LifeMeterBar","NumChambers")
#define NUM_STRIPS			THEME->GetMetricI("LifeMeterBar","NumStrips")

int		g_iMeterWidth;
int		g_iMeterHeight;
float	g_fDangerThreshold;
int		g_iNumChambers;
int		g_iNumStrips;


const float FAIL_THRESHOLD = 0;


class LifeMeterStream : public Actor
{
public:
	LifeMeterStream()
	{
		g_iMeterWidth = METER_WIDTH;
		g_iMeterHeight = METER_HEIGHT;
		g_fDangerThreshold = DANGER_THRESHOLD;
		g_iNumChambers = NUM_CHAMBERS;
		g_iNumStrips = NUM_STRIPS;


		bool bExtra = GAMESTATE->IsExtraStage()||GAMESTATE->IsExtraStage2();

		m_quadMask.SetDiffuse( D3DXCOLOR(0,0,0,0) );
		m_quadMask.SetZ( -1 );

		CString sGraphicPath;
		
		sGraphicPath = ssprintf("gameplay %slifemeter stream normal", bExtra?"extra ":"");
		m_sprStreamNormal.Load( THEME->GetPathTo("Graphics", sGraphicPath) );

		sGraphicPath = ssprintf("gameplay %slifemeter stream hot", bExtra?"extra ":"");
		m_sprStreamHot.Load( THEME->GetPathTo("Graphics", sGraphicPath) );

		sGraphicPath = ssprintf("gameplay %slifemeter bar", bExtra?"extra ":"");
		m_sprFrame.Load( THEME->GetPathTo("Graphics", sGraphicPath) );
	}

	Sprite		m_sprStreamNormal;
	Sprite		m_sprStreamHot;
	Sprite		m_sprFrame;
	Quad		m_quadMask;

	PlayerNumber m_PlayerNumber;
	float m_fPercent;
	float m_fHotAlpha;

	void GetChamberIndexAndOverslow( float fPercent, int& iChamberOut, float& fChamberOverflowPercentOut )
	{
		iChamberOut = (int)(fPercent*g_iNumChambers);
		fChamberOverflowPercentOut = fPercent*g_iNumChambers - iChamberOut;
	}

	float GetChamberLeftPercent( int iChamber )
	{
		return (iChamber+0) / (float)g_iNumChambers;
	}

	float GetChamberRightPercent( int iChamber )
	{
		return (iChamber+1) / (float)g_iNumChambers;
	}

	float GetRightEdgePercent( int iChamber, float fChamberOverflowPercent )
	{
		if( (iChamber%2) == 0 )
			return (iChamber+fChamberOverflowPercent) / (float)g_iNumChambers;
		else
			return (iChamber+1) / (float)g_iNumChambers;
	}

	float GetHeightPercent( int iChamber, float fChamberOverflowPercent )
	{
		if( (iChamber%2) == 1 )
			return 1-fChamberOverflowPercent;
		else
			return 0;
	}

	void DrawPrimitives()
	{
		DISPLAY->EnableZBuffer();

		if( GAMESTATE->IsPlayerEnabled(m_PlayerNumber) )
		{
			DrawMask( m_fPercent );		// this is the "right endcap" to the life
			
			const float fChamberWidthInPercent = 1.0f/g_iNumChambers;
			float fPercentBetweenStrips = 1.0f/g_iNumStrips;
			// round this so that the chamber overflows align
			if( g_iNumChambers > 10 )
				fPercentBetweenStrips = froundf( fPercentBetweenStrips, fChamberWidthInPercent );

			float fPercentOffset = fmodf( GAMESTATE->m_fSongBeat/4+1000, fPercentBetweenStrips );
			ASSERT( fPercentOffset >= 0  &&  fPercentOffset <= fPercentBetweenStrips );

			for( float f=fPercentOffset+1; f>=0; f-=fPercentBetweenStrips )
			{
				DrawMask( f );
				DrawStrip( f );
			}

		}

		DISPLAY->DisableZBuffer();

		m_sprFrame.Draw();

	}

	void DrawStrip( float fRightEdgePercent )
	{
		RECT rect;

		const float fChamberWidthInPercent = 1.0f/g_iNumChambers;
		const float fStripWidthInPercent = 1.0f/g_iNumStrips;
		
		const float fCorrectedRightEdgePercent = fRightEdgePercent + fChamberWidthInPercent;
		const float fCorrectedStripWidthInPercent = fStripWidthInPercent + 2*fChamberWidthInPercent;
		const float fCorrectedLeftEdgePercent = fCorrectedRightEdgePercent - fCorrectedStripWidthInPercent;


		// set size of streams
		rect.left	= LONG(-g_iMeterWidth/2 + g_iMeterWidth*max(0,fCorrectedLeftEdgePercent));
		rect.top	= LONG(-g_iMeterHeight/2);
		rect.right	= LONG(-g_iMeterWidth/2 + g_iMeterWidth*min(1,fCorrectedRightEdgePercent));
		rect.bottom	= LONG(+g_iMeterHeight/2);

		ASSERT( rect.left <= g_iMeterWidth/2  &&  rect.right <= g_iMeterWidth/2 );  

		float fPercentCroppedFromLeft = max( 0, -fCorrectedLeftEdgePercent );
		float fPercentCroppedFromRight = max( 0, fCorrectedRightEdgePercent-1 );


		m_sprStreamNormal.StretchTo( &rect );
		m_sprStreamHot.StretchTo( &rect );


		// set custom texture coords
//		float fPrecentOffset = fRightEdgePercent;

		FRECT frectCustomTexCoords(
			fPercentCroppedFromLeft,
			0,
			1-fPercentCroppedFromRight,
			1);

		m_sprStreamNormal.SetCustomTextureRect( frectCustomTexCoords );
		m_sprStreamHot.SetCustomTextureRect( frectCustomTexCoords );

		m_sprStreamHot.SetDiffuse( D3DXCOLOR(1,1,1,m_fHotAlpha) );

		m_sprStreamNormal.Draw();
		m_sprStreamHot.Draw();
	}

	void DrawMask( float fPercent )
	{
		RECT rect;

		int iChamber;
		float fChamberOverflowPercent;
		GetChamberIndexAndOverslow( fPercent, iChamber, fChamberOverflowPercent );
		float fRightPercent = GetRightEdgePercent( iChamber, fChamberOverflowPercent );
		float fHeightPercent = GetHeightPercent( iChamber, fChamberOverflowPercent );
		float fChamberLeftPercent = GetChamberLeftPercent( iChamber );
		float fChamberRightPercent = GetChamberRightPercent( iChamber );

		// draw mask for vertical chambers
		rect.left	= LONG(-g_iMeterWidth/2 + fChamberLeftPercent*g_iMeterWidth-1);
		rect.top	= LONG(-g_iMeterHeight/2);
		rect.right	= LONG(-g_iMeterWidth/2 + fChamberRightPercent*g_iMeterWidth+1);
		rect.bottom	= LONG(-g_iMeterHeight/2 + fHeightPercent*g_iMeterHeight);

		rect.left  = MIN( rect.left,  + g_iMeterWidth/2 );
		rect.right = MIN( rect.right, + g_iMeterWidth/2 );

		m_quadMask.StretchTo( &rect );
		m_quadMask.Draw();

		// draw mask for horizontal chambers
		rect.left	= (LONG)(-g_iMeterWidth/2 + fRightPercent*g_iMeterWidth); 
		rect.top	= -g_iMeterHeight/2;
		rect.right	= +g_iMeterWidth/2;
		rect.bottom	= +g_iMeterHeight/2;

		rect.left  = MIN( rect.left,  + g_iMeterWidth/2 );
		rect.right = MIN( rect.right, + g_iMeterWidth/2 );

		m_quadMask.StretchTo( &rect );
		m_quadMask.Draw();
		
	}
};


LifeMeterBar::LifeMeterBar()
{
	m_pStream = new LifeMeterStream;

	switch( GAMESTATE->m_SongOptions.m_DrainType )
	{
	case SongOptions::DRAIN_NORMAL:			m_fLifePercentage = 0.5f;	break;
	case SongOptions::DRAIN_NO_RECOVER:		m_fLifePercentage = 1.0f;	break;
	case SongOptions::DRAIN_SUDDEN_DEATH:	m_fLifePercentage = 1.0f;	break;
	default:	ASSERT(0);
	}

	m_fTrailingLifePercentage = 0;
	m_fLifeVelocity = 0;
	m_fHotAlpha = 0;
	m_bFailedEarlier = false;

	m_quadBlackBackground.SetDiffuse( D3DXCOLOR(0,0,0,1) );
	m_quadBlackBackground.SetZoomX( (float)g_iMeterWidth );
	m_quadBlackBackground.SetZoomY( (float)g_iMeterHeight );

	this->AddChild( &m_quadBlackBackground );
	this->AddChild( m_pStream );

	AfterLifeChanged();
}

LifeMeterBar::~LifeMeterBar()
{
	delete m_pStream;
}

void LifeMeterBar::Load( PlayerNumber pn )
{
	LifeMeter::Load( pn );

	m_pStream->m_PlayerNumber = pn;

	if( pn == PLAYER_2 )
		m_pStream->SetZoomX( -1 );
}

void LifeMeterBar::ChangeLife( TapNoteScore score )
{
	float fDeltaLife=0.f;
	switch( GAMESTATE->m_SongOptions.m_DrainType )
	{
	case SongOptions::DRAIN_NORMAL:
		switch( score )
		{
		case TNS_PERFECT:	fDeltaLife = +0.008f;	break;
		case TNS_GREAT:		fDeltaLife = +0.004f;	break;
		case TNS_GOOD:		fDeltaLife = +0.000f;	break;
		case TNS_BOO:		fDeltaLife = -0.040f;	break;
		case TNS_MISS:		fDeltaLife = -0.080f;	break;
		}
		if( IsHot()  &&  score < TNS_GOOD )
			fDeltaLife = -0.10f;		// make it take a while to get back to "doing great"
		break;
	case SongOptions::DRAIN_NO_RECOVER:
		switch( score )
		{
		case TNS_PERFECT:	fDeltaLife = +0.000f;	break;
		case TNS_GREAT:		fDeltaLife = +0.000f;	break;
		case TNS_GOOD:		fDeltaLife = +0.000f;	break;
		case TNS_BOO:		fDeltaLife = -0.040f;	break;
		case TNS_MISS:		fDeltaLife = -0.080f;	break;
		}
		break;
	case SongOptions::DRAIN_SUDDEN_DEATH:
		switch( score )
		{
		case TNS_PERFECT:	fDeltaLife = +0;	break;
		case TNS_GREAT:		fDeltaLife = +0;	break;
		case TNS_GOOD:		fDeltaLife = -1.0;	break;
		case TNS_BOO:		fDeltaLife = -1.0;	break;
		case TNS_MISS:		fDeltaLife = -1.0;	break;
		}
		break;
	default:
		ASSERT(0);
	}

	if( fDeltaLife > 0 )
		fDeltaLife *= PREFSMAN->m_fLifeDifficultyScale;
	else
		fDeltaLife /= PREFSMAN->m_fLifeDifficultyScale;

	m_fLifePercentage += fDeltaLife;
	CLAMP( m_fLifePercentage, 0, 1 );

	if( m_fLifePercentage <= FAIL_THRESHOLD )
		m_bFailedEarlier = true;

	m_fLifeVelocity += fDeltaLife;
}

void LifeMeterBar::ChangeLife( HoldNoteScore score, TapNoteScore tscore )
{

}

void LifeMeterBar::AfterLifeChanged()
{

}

bool LifeMeterBar::IsHot() 
{ 
	return m_fLifePercentage >= 1; 
}

bool LifeMeterBar::IsInDanger() 
{ 
	return m_fLifePercentage < g_fDangerThreshold; 
}

bool LifeMeterBar::IsFailing() 
{ 
	return m_fLifePercentage <= 0; 
}

bool LifeMeterBar::FailedEarlier() 
{ 
	return m_bFailedEarlier; 
}

void LifeMeterBar::Update( float fDeltaTime )
{
	LifeMeter::Update( fDeltaTime );

	// HACK:  Tweaking these values is very difficulty.  Update the
	// "physics" many times so that the spring motion appears faster

	for( int i=0; i<10; i++ )
	{

		const float fDelta = m_fLifePercentage - m_fTrailingLifePercentage;

		const float fSpringForce = fDelta * 2.0f;
		m_fLifeVelocity += fSpringForce * fDeltaTime;

		const float fViscousForce = -m_fLifeVelocity * 0.2f;
		m_fLifeVelocity += fViscousForce * fDeltaTime;

		CLAMP( m_fLifeVelocity, -.06f, +.02f );

		m_fTrailingLifePercentage += m_fLifeVelocity * fDeltaTime;
	}

	m_fHotAlpha  += IsHot() ? + fDeltaTime*2 : -fDeltaTime*2;
	CLAMP( m_fHotAlpha, 0, 1 );

	if( IsHot() )
		m_fLifeVelocity = max( 0, m_fLifeVelocity );
}


void LifeMeterBar::DrawPrimitives()
{
	/*
	// set custom texture coords
	static RECT rectSize;
	rectSize.left	= -m_iMeterWidth/2; 
	rectSize.top	= -m_iMeterHeight/2;
	rectSize.right	= -m_iMeterWidth/2 + int(roundf(m_iMeterWidth*m_fTrailingLifePercentage));
	rectSize.bottom	= +m_iMeterHeight/2;

	float fPrecentOffset = TIMER->GetTimeSinceStart();
	fPrecentOffset -= (int)fPrecentOffset;

	FRECT frectCustomTexCoords(
		0 - fPrecentOffset,
		0,
		m_fTrailingLifePercentage - fPrecentOffset,
		1 );

	m_sprStreamNormal.StretchTo( &rectSize );
	m_sprStreamNormal.SetCustomTextureRect( frectCustomTexCoords );
	m_sprStreamHot.StretchTo( &rectSize );
	m_sprStreamHot.SetCustomTextureRect( frectCustomTexCoords );

	m_sprStreamHot.SetDiffuse(    D3DXCOLOR(1,1,1,m_fHotAlpha) );

*/

	m_pStream->m_fPercent = m_fTrailingLifePercentage;
	m_pStream->m_fHotAlpha = m_fHotAlpha;

	float fPercentRed = (m_fTrailingLifePercentage<g_fDangerThreshold) ? sinf( TIMER->GetTimeSinceStart()*D3DX_PI*4 )/2+0.5f : 0;
	m_quadBlackBackground.SetDiffuse( D3DXCOLOR(fPercentRed*0.8f,0,0,1) );

	ActorFrame::DrawPrimitives();
}

/*

  Chris:  I'm making the coloring of the lifemeter a property 
  of the theme.  That's where it belongs anyway...

  
const D3DXCOLOR COLOR_EZ2NORMAL_1	= D3DXCOLOR(0.7f,0.4f,0,1);
const D3DXCOLOR COLOR_EZ2NORMAL_2	= D3DXCOLOR(0.8f,0.4f,0,1);
const D3DXCOLOR COLOR_EZ2NEARFULL_1	= D3DXCOLOR(0.7f,0.6f,0,1);
const D3DXCOLOR COLOR_EZ2NEARFULL_2	= D3DXCOLOR(0.8f,0.7f,0,1);
const D3DXCOLOR COLOR_EZ2NEARFAIL_1	= D3DXCOLOR(0.9f,0.0f,0,1);
const D3DXCOLOR COLOR_EZ2NEARFAIL_2	= D3DXCOLOR(0.8f,0.1f,0,1);
const D3DXCOLOR COLOR_EZ2FULL_1	= D3DXCOLOR(0.3f,0.9f,0.4f,1);
const D3DXCOLOR COLOR_EZ2FULL_2	= D3DXCOLOR(0.2f,0.7f,0.3f,1);
const D3DXCOLOR COLOR_NORMAL_1	= D3DXCOLOR(1,1,1,1);
const D3DXCOLOR COLOR_NORMAL_2	= D3DXCOLOR(0,1,0,1);
const D3DXCOLOR COLOR_FULL_1	= D3DXCOLOR(1,0,0,1);
const D3DXCOLOR COLOR_FULL_2	= D3DXCOLOR(1,1,0,1);

D3DXCOLOR LifeStream::GetColor( float fPercentIntoSection )
{
	float fPercentColor1 = fabsf( fPercentIntoSection*2 - 1 );
	fPercentColor1 *= fPercentColor1 * fPercentColor1 * fPercentColor1;	// make the color bunch around one side
	if (GAMESTATE->m_CurGame != GAME_EZ2)
	{
		if( m_fLifePercentage == 1 )
			return COLOR_FULL_1 * fPercentColor1 + COLOR_FULL_2 * (1-fPercentColor1);
		else
			return COLOR_NORMAL_1 * fPercentColor1 + COLOR_NORMAL_2 * (1-fPercentColor1);
	}
	else
	{
		if( m_fLifePercentage == 1 )
			return COLOR_EZ2FULL_1 * fPercentColor1 + COLOR_EZ2FULL_2 * (1-fPercentColor1);
		else if ( m_fLifePercentage > 0.60 )
			return COLOR_EZ2NEARFULL_1 * fPercentColor1 + COLOR_EZ2NEARFULL_2 * (1-fPercentColor1);
		else if ( m_fLifePercentage < 0.25 )
			return COLOR_EZ2NEARFAIL_1 * fPercentColor1 + COLOR_EZ2NEARFAIL_2 * (1-fPercentColor1);
		else
			return COLOR_EZ2NORMAL_1 * fPercentColor1 + COLOR_EZ2NORMAL_2 * (1-fPercentColor1);
	}
}

void LifeStream::DrawPrimitives()
{
	// make the object in logical units centered at the origin
	LPDIRECT3DVERTEXBUFFER8 pVB = DISPLAY->GetVertexBuffer();
	RAGEVERTEX* v;
	pVB->Lock( 0, 0, (BYTE**)&v, 0 );

	int iNumV = 0;

	float fPercentIntoSection = (TIMER->GetTimeSinceStart()/0.3f* (m_bIsHot ? 2 : 1) )*LIFE_STREAM_SECTION_WIDTH;
	fPercentIntoSection -= (int)fPercentIntoSection;
	fPercentIntoSection = 1-fPercentIntoSection;
	fPercentIntoSection -= (int)fPercentIntoSection;
	ASSERT( fPercentIntoSection >= 0  &&  fPercentIntoSection < 1 );
	float fX = - 0.5f; 


	while( fX < m_fTrailingLifePercentage-0.5f )
	{
		//
		// draw middle
		//
		v[iNumV++].p = D3DXVECTOR3( fX, -0.5f, 0 );
		v[iNumV++].p = D3DXVECTOR3( fX,  0.5f, 0 );
		
		iNumV -= 2;
		const D3DXCOLOR color = GetColor( fPercentIntoSection ); 
		v[iNumV++].color = color;
		v[iNumV++].color = color;


		if( fPercentIntoSection < 0.5f )
		{
			const float fPercentToAdd = 0.5f-fPercentIntoSection;
			fPercentIntoSection += fPercentToAdd;
			fX += fPercentToAdd*LIFE_STREAM_SECTION_WIDTH;
		}
		else if( fPercentIntoSection < 1.0f )
		{
			const float fPercentToAdd = 1.0f-fPercentIntoSection;
			fPercentIntoSection = 0;
			fX += fPercentToAdd*LIFE_STREAM_SECTION_WIDTH;
		}
		else
			ASSERT( false );
	}

	const float fXToSubtract = fX - (m_fTrailingLifePercentage-0.5f);
	fX -= fXToSubtract;
	fPercentIntoSection -= fXToSubtract/LIFE_STREAM_SECTION_WIDTH;
	if( fPercentIntoSection < 0 )
		fPercentIntoSection += 1;

	//
	// draw right edge
	//
	v[iNumV++].p = D3DXVECTOR3( fX, -0.5f, 0 );
	v[iNumV++].p = D3DXVECTOR3( fX,  0.5f, 0 );
	
	iNumV -= 2;
	const D3DXCOLOR color = GetColor( fPercentIntoSection ); 
	v[iNumV++].color = color;
	v[iNumV++].color = color; 



	pVB->Unlock();


	LPDIRECT3DDEVICE8 pd3dDevice = DISPLAY->GetDevice();
	pd3dDevice->SetTexture( 0, NULL );



	// set texture and alpha properties
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );

	pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
	pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );


	// finally!  Pump those triangles!	
	pd3dDevice->SetVertexShader( D3DFVF_RAGEVERTEX );
	pd3dDevice->SetStreamSource( 0, pVB, sizeof(RAGEVERTEX) );
	pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, iNumV-2 );

}
*/