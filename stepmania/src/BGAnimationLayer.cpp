#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: BGAnimation

 Desc: Particles used initially for background effects

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "BGAnimationLayer.h"
#include "PrefsManager.h"
#include "GameState.h"
#include "IniFile.h"
#include "RageMath.h"
#include <math.h>


inline float GetOffScreenLeft(  Actor* pActor ) { return SCREEN_LEFT  - pActor->GetZoomedWidth()/2; }
inline float GetOffScreenRight( Actor* pActor ) { return SCREEN_RIGHT + pActor->GetZoomedWidth()/2; }
inline float GetOffScreenTop(   Actor* pActor ) { return SCREEN_TOP   - pActor->GetZoomedHeight()/2; }
inline float GetOffScreenBottom(Actor* pActor ) { return SCREEN_BOTTOM+ pActor->GetZoomedHeight()/2; }

inline bool IsOffScreenLeft(  Actor* pActor ) { return pActor->GetX() < GetOffScreenLeft(pActor); }
inline bool IsOffScreenRight( Actor* pActor ) { return pActor->GetX() > GetOffScreenRight(pActor); }
inline bool IsOffScreenTop(   Actor* pActor ) { return pActor->GetY() < GetOffScreenTop(pActor); }
inline bool IsOffScreenBottom(Actor* pActor ) { return pActor->GetY() > GetOffScreenBottom(pActor); }

// guard rail is the area that keeps particles from going off screen
inline float GetGuardRailLeft(  Actor* pActor ) { return SCREEN_LEFT  + pActor->GetZoomedWidth()/2; }
inline float GetGuardRailRight( Actor* pActor ) { return SCREEN_RIGHT - pActor->GetZoomedWidth()/2; }
inline float GetGuardRailTop(   Actor* pActor ) { return SCREEN_TOP   + pActor->GetZoomedHeight()/2; }
inline float GetGuardRailBottom(Actor* pActor ) { return SCREEN_BOTTOM- pActor->GetZoomedHeight()/2; }

inline bool HitGuardRailLeft(  Actor* pActor ) { return pActor->GetX() < GetGuardRailLeft(pActor); }
inline bool HitGuardRailRight( Actor* pActor ) { return pActor->GetX() > GetGuardRailRight(pActor); }
inline bool HitGuardRailTop(   Actor* pActor ) { return pActor->GetY() < GetGuardRailTop(pActor); }
inline bool HitGuardRailBottom(Actor* pActor ) { return pActor->GetY() > GetGuardRailBottom(pActor); }


const float PARTICLE_VELOCITY = 300;

const float SPIRAL_MAX_ZOOM = 2;
const float SPIRAL_MIN_ZOOM = 0.3f;



BGAnimationLayer::BGAnimationLayer()
{
	m_iNumSprites = 0;
	m_bCycleColor = false;
	m_bCycleAlpha = false;
	m_Effect = EFFECT_STRETCH_STILL;	
}

void BGAnimationLayer::LoadFromStaticGraphic( CString sPath )
{
	m_iNumSprites = 1;
	m_Sprites[0].Load( sPath );
	m_Sprites[0].StretchTo( RectI(SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );
}

void BGAnimationLayer::LoadFromMovie( CString sMoviePath, bool bLoop, bool bRewind )
{
	m_iNumSprites = 1;
	m_Sprites[0].Load( sMoviePath );
	m_Sprites[0].StretchTo( RectI(SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );
	m_Sprites[0].GetTexture()->Play();
	::Sleep( 50 );	// decode a frame so we don't see a black flash at the beginning
	m_Sprites[0].GetTexture()->Pause();
	m_bRewindMovie = bRewind;
	if( !bLoop )
		m_Sprites[0].GetTexture()->SetLooping(false);
}

void BGAnimationLayer::LoadFromVisualization( CString sMoviePath )
{
	m_iNumSprites = 1;
	m_Sprites[0].Load( sMoviePath );
	m_Sprites[0].StretchTo( RectI(SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );
	m_Sprites[0].SetBlendModeAdd();
}

void BGAnimationLayer::LoadFromAniLayerFile( CString sPath, CString sSongBGPath )
{
	sPath.MakeLower();

	if( sPath.Find("usesongbg") != -1 )
	{
		LoadFromStaticGraphic( sSongBGPath );
		return;		// this will ignore other effects in the file name
	}

	const CString EFFECT_STRING[NUM_EFFECTS] = {
		"center",
		"stretchstill",
		"stretchscrollleft",
		"stretchscrollright",
		"stretchscrollup",
		"stretchscrolldown",
		"stretchwater",
		"stretchbubble",
		"stretchtwist",
		"stretchspin",
		"particlesspiralout",
		"particlesspiralin",
		"particlesfloatup",
		"particlesfloatdown",
		"particlesfloatleft",
		"particlesfloatright",
		"particlesbounce",
		"tilestill",
		"tilescrollleft",
		"tilescrollright",
		"tilescrollup",
		"tilescrolldown",
		"tileflipx",
		"tileflipy",
		"tilepulse",
		"stretchscrollhorizontal"
	};

	for( int i=0; i<NUM_EFFECTS; i++ )
	{
		if( sPath.Find(EFFECT_STRING[i]) != -1 )
		{
			m_Effect = (Effect)i;
			goto found_effect;
		}
	}
	// If we get here, we didn't find an effect string in the file name.  Use the defualt effect (StretchStill)
	
found_effect:

	//////////////////////
	// init
	//////////////////////
	switch( m_Effect )
	{
	case EFFECT_CENTER:
		m_iNumSprites = 1;
		m_Sprites[0].Load( sPath );
		m_Sprites[0].SetXY( CENTER_X, CENTER_Y );
		break;
	case EFFECT_STRETCH_STILL:
		m_iNumSprites = 1;
		m_Sprites[0].Load( sPath );
		m_Sprites[0].StretchTo( RectI(SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );
		break;
	case EFFECT_STRETCH_SCROLL_LEFT:
	case EFFECT_STRETCH_SCROLL_RIGHT:
	case EFFECT_STRETCH_SCROLL_UP:
	case EFFECT_STRETCH_SCROLL_DOWN:
	case EFFECT_STRETCH_WATER:
	case EFFECT_STRETCH_BUBBLE:
	case EFFECT_STRETCH_TWIST:
		{
			m_iNumSprites = 1;
			RageTexturePrefs prefs;
			prefs.bStretch = true;
			m_Sprites[0].Load( sPath, prefs );
			m_Sprites[0].StretchTo( RectI(SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );
			m_Sprites[0].SetCustomTextureRect( RectF(0,0,1,1) );

			switch( m_Effect )
			{
			case EFFECT_STRETCH_SCROLL_LEFT:	m_vTexCoordVelocity = RageVector2(+0.5f,0);	break;
			case EFFECT_STRETCH_SCROLL_RIGHT:	m_vTexCoordVelocity = RageVector2(-0.5f,0);	break;
			case EFFECT_STRETCH_SCROLL_UP:		m_vTexCoordVelocity = RageVector2(0,+0.5f);	break;
			case EFFECT_STRETCH_SCROLL_DOWN:	m_vTexCoordVelocity = RageVector2(0,-0.5f);	break;
			case EFFECT_STRETCH_WATER:
			case EFFECT_STRETCH_BUBBLE:
			case EFFECT_STRETCH_TWIST:
				m_vTexCoordVelocity = RageVector2(-0.0f,0);	
				break;
			default:
				ASSERT(0);
			}
		}
		break;
	case EFFECT_STRETCH_SCROLL_H:
		{
			m_iNumSprites = 1;
			RageTexturePrefs prefs;
			prefs.bStretch = true;
			m_Sprites[0].Load( sPath, prefs );
			m_Sprites[0].StretchTo( RectI(SCREEN_LEFT,0,SCREEN_RIGHT,m_Sprites[0].GetUnzoomedHeight()) );
			m_Sprites[0].SetCustomTextureRect( RectF(0,0,1,1) );
			m_vTexCoordVelocity = RageVector2(+0.5f,0);
			m_Sprites[0].SetY(m_fStretchScrollH_Y);
		}
		break;
	case EFFECT_STRETCH_SPIN:
		m_iNumSprites = 1;
		m_Sprites[0].Load( sPath );
		m_Sprites[0].ScaleToCover( RectI(SCREEN_LEFT-200,SCREEN_TOP-200,SCREEN_RIGHT+200,SCREEN_BOTTOM+200) );
		m_fRotationalVelocity = 1;
		break;
	case EFFECT_PARTICLES_SPIRAL_OUT:
	case EFFECT_PARTICLES_SPIRAL_IN:
		{
			m_Sprites[0].Load( sPath );
			int iSpriteArea = int( m_Sprites[0].GetUnzoomedWidth()*m_Sprites[0].GetUnzoomedHeight() );
			int iMaxArea = SCREEN_WIDTH*SCREEN_HEIGHT;
			m_iNumSprites = min( iMaxArea / iSpriteArea,  MAX_SPRITES );
			for( int i=0; i<m_iNumSprites; i++ )
			{
				m_Sprites[i].Load( sPath );
				m_Sprites[i].SetZoom( randomf(0.2f,2) );
				m_Sprites[i].SetRotation( randomf(0,PI*2) );
			}
		}
		break;
	case EFFECT_PARTICLES_FLOAT_UP:
	case EFFECT_PARTICLES_FLOAT_DOWN:
	case EFFECT_PARTICLES_FLOAT_LEFT:
	case EFFECT_PARTICLES_FLOAT_RIGHT:
	case EFFECT_PARTICLES_BOUNCE:
		{
			m_Sprites[0].Load( sPath );
			int iSpriteArea = int( m_Sprites[0].GetUnzoomedWidth()*m_Sprites[0].GetUnzoomedHeight() );
			int iMaxArea = SCREEN_WIDTH*SCREEN_HEIGHT;
			m_iNumSprites = min( iMaxArea / iSpriteArea,  MAX_SPRITES );
			for( int i=0; i<m_iNumSprites; i++ )
			{
				m_Sprites[i].Load( sPath );
				m_Sprites[i].SetZoom( 0.7f + 0.6f*i/(float)m_iNumSprites );
				m_Sprites[i].SetX( randomf( GetGuardRailLeft(&m_Sprites[i]), GetGuardRailRight(&m_Sprites[i]) ) );
				m_Sprites[i].SetY( randomf( GetGuardRailTop(&m_Sprites[i]), GetGuardRailBottom(&m_Sprites[i]) ) );

				if( m_Effect == EFFECT_PARTICLES_BOUNCE )
				{
					m_Sprites[i].SetZoom( 1 );
					m_vHeadings[i] = RageVector2( randomf(), randomf() );
					RageVec2Normalize( &m_vHeadings[i], &m_vHeadings[i] );
				}
			}
		}
		break;
	case EFFECT_TILE_STILL:
	case EFFECT_TILE_SCROLL_LEFT:
	case EFFECT_TILE_SCROLL_RIGHT:
	case EFFECT_TILE_SCROLL_UP:
	case EFFECT_TILE_SCROLL_DOWN:
	case EFFECT_TILE_FLIP_X:
	case EFFECT_TILE_FLIP_Y:
	case EFFECT_TILE_PULSE:
		{
			m_Sprites[0].Load( sPath );
			int iNumTilesWide = 1+int(SCREEN_WIDTH /m_Sprites[0].GetUnzoomedWidth());
			int iNumTilesHigh = 1+int(SCREEN_HEIGHT/m_Sprites[0].GetUnzoomedHeight());
			if( m_Effect == EFFECT_TILE_SCROLL_LEFT ||
				m_Effect == EFFECT_TILE_SCROLL_RIGHT ) {
				iNumTilesWide++;
			}
			if( m_Effect == EFFECT_TILE_SCROLL_UP ||
				m_Effect == EFFECT_TILE_SCROLL_DOWN ) {
				iNumTilesHigh++;
			}

			iNumTilesWide = min( iNumTilesWide, MAX_TILES_WIDE );
			iNumTilesHigh = min( iNumTilesHigh, MAX_TILES_HIGH );

			m_iNumSprites = min( iNumTilesWide * iNumTilesHigh,  MAX_SPRITES );

			for( int x=0; x<iNumTilesWide; x++ )
			{
				for( int y=0; y<iNumTilesHigh; y++ )
				{
					int i = x+y*iNumTilesWide;
					m_Sprites[i].Load( sPath );
					m_Sprites[i].SetX( (x+0.5f)*m_Sprites[i].GetUnzoomedWidth() );
					m_Sprites[i].SetY( (y+0.5f)*m_Sprites[i].GetUnzoomedHeight() );
				}
			}
		}
		break;
	default:
		ASSERT(0);
	}


	m_bCycleColor  = sPath.Find("cyclecolor") != -1;
	m_bCycleAlpha  = sPath.Find("cyclealpha") != -1;

	if( sPath.Find("startonrandomframe") != -1 )
		for( int i=0; i<m_iNumSprites; i++ )
			m_Sprites[i].SetState( rand()%m_Sprites[i].GetNumStates() );

	if( sPath.Find("dontanimate") != -1 )
		for( int i=0; i<m_iNumSprites; i++ )
			m_Sprites[i].StopAnimating();

	if( sPath.Find("add") != -1 )
		for( int i=0; i<m_iNumSprites; i++ )
			m_Sprites[i].SetBlendModeAdd();

	CString sDir, sFName, sExt;
	splitrelpath( sPath, sDir, sFName, sExt );
	CString sIniPath = sDir+"\\"+sFName+".ini";
	IniFile ini;
	ini.SetPath( sIniPath );
	if( ini.ReadFile() )
	{
		ini.GetValueF( "BGAnimationLayer", "TexCoordVelocityX", m_vTexCoordVelocity.x );
		ini.GetValueF( "BGAnimationLayer", "TexCoordVelocityY", m_vTexCoordVelocity.y );
		ini.GetValueF( "BGAnimationLayer", "RotationalVelocity", m_fRotationalVelocity );
		ini.GetValueF( "BGAnimationLayer", "SetY", m_fStretchScrollH_Y );
	}
}

void BGAnimationLayer::Update( float fDeltaTime  )
{
	int i;

	const float fSongBeat = GAMESTATE->m_fSongBeat;
	if( m_bCycleColor )
	{
		for( int i=0; i<m_iNumSprites; i++ )
		{
			RageColor color = RageColor(
				cosf( fSongBeat+i ) * 0.5f + 0.5f,
				cosf( fSongBeat+i + PI * 2.0f / 3.0f ) * 0.5f + 0.5f,
				cosf( fSongBeat+i + PI * 4.0f / 3.0f) * 0.5f + 0.5f,
				1.0f
				);
			m_Sprites[i].SetDiffuse( color );
		}
	}
	if( m_bCycleAlpha )
	{
		for( int i=0; i<m_iNumSprites; i++ )
		{
			RageColor color = m_Sprites[i].GetDiffuse();
			color.a = cosf( fSongBeat/2 ) * 0.5f + 0.5f;
			m_Sprites[i].SetDiffuse( color );
		}
	}

	
	
	for( i=0; i<m_iNumSprites; i++ )
	{
		m_Sprites[i].Update( fDeltaTime );
	}

	if(m_Effect == EFFECT_STRETCH_SCROLL_H)
		m_Sprites[0].SetY(m_fStretchScrollH_Y);

	switch( m_Effect )
	{
	case EFFECT_CENTER:
	case EFFECT_STRETCH_STILL:
		break;
	case EFFECT_STRETCH_SCROLL_LEFT:
	case EFFECT_STRETCH_SCROLL_RIGHT:
	case EFFECT_STRETCH_SCROLL_UP:
	case EFFECT_STRETCH_SCROLL_DOWN:
	case EFFECT_STRETCH_SCROLL_H:
		float fTexCoords[8];
		m_Sprites[0].GetCustomTextureCoords( fTexCoords );

		for( i=0; i<8; i+=2 )
		{
			fTexCoords[i  ] += fDeltaTime*m_vTexCoordVelocity.x;
			fTexCoords[i+1] += fDeltaTime*m_vTexCoordVelocity.y;
		}

		m_Sprites[0].SetCustomTextureCoords( fTexCoords );
		
		break;
	case EFFECT_STRETCH_SPIN:
		m_Sprites[0].SetRotation( m_Sprites[0].GetRotation() + fDeltaTime*m_fRotationalVelocity );
	case EFFECT_STRETCH_WATER:
	case EFFECT_STRETCH_BUBBLE:
	case EFFECT_STRETCH_TWIST:
		break;
	case EFFECT_PARTICLES_SPIRAL_OUT:
		for( i=0; i<m_iNumSprites; i++ )
		{
			m_Sprites[i].SetZoom( m_Sprites[i].GetZoom() + fDeltaTime );
			if( m_Sprites[i].GetZoom() > SPIRAL_MAX_ZOOM )
				m_Sprites[i].SetZoom( SPIRAL_MIN_ZOOM );

			m_Sprites[i].SetRotation( m_Sprites[i].GetRotation() + fDeltaTime );

			float fRadius = (m_Sprites[i].GetZoom()-SPIRAL_MIN_ZOOM);
			fRadius *= fRadius;
			fRadius *= 200;
			m_Sprites[i].SetX( CENTER_X + cosf(m_Sprites[i].GetRotation())*fRadius );
			m_Sprites[i].SetY( CENTER_Y + sinf(m_Sprites[i].GetRotation())*fRadius );
		}
		break;
	case EFFECT_PARTICLES_SPIRAL_IN:
		for( i=0; i<m_iNumSprites; i++ )
		{
			m_Sprites[i].SetZoom( m_Sprites[i].GetZoom() - fDeltaTime );
			if( m_Sprites[i].GetZoom() < SPIRAL_MIN_ZOOM )
				m_Sprites[i].SetZoom( SPIRAL_MAX_ZOOM );

			m_Sprites[i].SetRotation( m_Sprites[i].GetRotation() - fDeltaTime );

			float fRadius = (m_Sprites[i].GetZoom()-SPIRAL_MIN_ZOOM);
			fRadius *= fRadius;
			fRadius *= 200;
			m_Sprites[i].SetX( CENTER_X + cosf(m_Sprites[i].GetRotation())*fRadius );
			m_Sprites[i].SetY( CENTER_Y + sinf(m_Sprites[i].GetRotation())*fRadius );
		}
		break;
	case EFFECT_PARTICLES_FLOAT_UP:
		for( i=0; i<m_iNumSprites; i++ )
		{
			m_Sprites[i].SetY( m_Sprites[i].GetY() - fDeltaTime * PARTICLE_VELOCITY * m_Sprites[i].GetZoom() );
			if( IsOffScreenTop(&m_Sprites[i]) )
				m_Sprites[i].SetY( GetOffScreenBottom(&m_Sprites[i]) );
		}
		break;
	case EFFECT_PARTICLES_FLOAT_DOWN:
		for( i=0; i<m_iNumSprites; i++ )
		{
			m_Sprites[i].SetY( m_Sprites[i].GetY() + fDeltaTime * PARTICLE_VELOCITY * m_Sprites[i].GetZoom() );
			if( IsOffScreenBottom(&m_Sprites[i]) )
				m_Sprites[i].SetY( GetOffScreenTop(&m_Sprites[i]) );
		}
		break;
	case EFFECT_PARTICLES_FLOAT_LEFT:
		for( i=0; i<m_iNumSprites; i++ )
		{
			m_Sprites[i].SetX( m_Sprites[i].GetX() - fDeltaTime * PARTICLE_VELOCITY * m_Sprites[i].GetZoom() );
			if( IsOffScreenLeft(&m_Sprites[i]) )
				m_Sprites[i].SetX( GetOffScreenRight(&m_Sprites[i]) );
		}
		break;
	case EFFECT_PARTICLES_FLOAT_RIGHT:
		for( i=0; i<m_iNumSprites; i++ )
		{
			m_Sprites[i].SetX( m_Sprites[i].GetX() + fDeltaTime * PARTICLE_VELOCITY * m_Sprites[i].GetZoom() );
			if( IsOffScreenRight(&m_Sprites[i]) )
				m_Sprites[i].SetX( GetOffScreenLeft(&m_Sprites[i]) );
		}
		break;
	case EFFECT_PARTICLES_BOUNCE:
		for( i=0; i<m_iNumSprites; i++ )
		{
			m_Sprites[i].SetX( m_Sprites[i].GetX() + fDeltaTime * PARTICLE_VELOCITY * m_vHeadings[i].x );
			m_Sprites[i].SetY( m_Sprites[i].GetY() + fDeltaTime * PARTICLE_VELOCITY * m_vHeadings[i].y );
			if( HitGuardRailLeft(&m_Sprites[i]) )	m_vHeadings[i].x *= -1;
			if( HitGuardRailRight(&m_Sprites[i]) )	m_vHeadings[i].x *= -1;
			if( HitGuardRailTop(&m_Sprites[i]) )	m_vHeadings[i].y *= -1;
			if( HitGuardRailBottom(&m_Sprites[i]) )	m_vHeadings[i].y *= -1;
		}
		break;
	case EFFECT_TILE_STILL:
		break;
	case EFFECT_TILE_SCROLL_LEFT:
		for( i=0; i<m_iNumSprites; i++ )
		{
			m_Sprites[i].SetX( m_Sprites[i].GetX() - fDeltaTime * PARTICLE_VELOCITY );
			if( IsOffScreenLeft(&m_Sprites[i]) )
				m_Sprites[i].SetX( m_Sprites[i].GetX()-GetOffScreenLeft(&m_Sprites[i]) + GetOffScreenRight(&m_Sprites[i]) );
		}
		break;
	case EFFECT_TILE_SCROLL_RIGHT:
		for( i=0; i<m_iNumSprites; i++ )
		{
			m_Sprites[i].SetX( m_Sprites[i].GetX() + fDeltaTime * PARTICLE_VELOCITY );
			if( IsOffScreenRight(&m_Sprites[i]) )
				m_Sprites[i].SetX( m_Sprites[i].GetX()-GetOffScreenRight(&m_Sprites[i]) + GetOffScreenLeft(&m_Sprites[i]) );
		}
		break;
	case EFFECT_TILE_SCROLL_UP:
		for( i=0; i<m_iNumSprites; i++ )
		{
			m_Sprites[i].SetY( m_Sprites[i].GetY() - fDeltaTime * PARTICLE_VELOCITY );
			if( IsOffScreenTop(&m_Sprites[i]) )
				m_Sprites[i].SetY( m_Sprites[i].GetY()-GetOffScreenTop(&m_Sprites[i]) + GetOffScreenBottom(&m_Sprites[i]) );
		}
		break;
	case EFFECT_TILE_SCROLL_DOWN:
		for( i=0; i<m_iNumSprites; i++ )
		{
			m_Sprites[i].SetY( m_Sprites[i].GetY() + fDeltaTime * PARTICLE_VELOCITY );
			if( IsOffScreenBottom(&m_Sprites[i]) )
				m_Sprites[i].SetY( m_Sprites[i].GetY()-GetOffScreenBottom(&m_Sprites[i]) + GetOffScreenTop(&m_Sprites[i]) );
		}
		break;
	case EFFECT_TILE_FLIP_X:
		for( i=0; i<m_iNumSprites; i++ )
		{
			m_Sprites[i].SetRotationX( m_Sprites[i].GetRotationX() + fDeltaTime * PI );
		}
		break;
	case EFFECT_TILE_FLIP_Y:
		for( i=0; i<m_iNumSprites; i++ )
		{
			m_Sprites[i].SetRotationY( m_Sprites[i].GetRotationY() + fDeltaTime * PI );
		}
		break;
	case EFFECT_TILE_PULSE:
		for( i=0; i<m_iNumSprites; i++ )
		{
			m_Sprites[i].SetZoom( sinf( fSongBeat*PI/2 ) );
		}
		break;
	default:
		ASSERT(0);
	}
}

void BGAnimationLayer::Draw()
{
	for( int i=0; i<m_iNumSprites; i++ )
	{
		m_Sprites[i].Draw();
	}
}

void BGAnimationLayer::GainingFocus()
{
	if( m_bRewindMovie )
		m_Sprites[0].GetTexture()->SetPosition( 0 );
	// if movie texture, pause and play movie so we don't waste CPU cycles decoding frames that won't be shown
	m_Sprites[0].GetTexture()->Play();
}

void BGAnimationLayer::LosingFocus()
{
	m_Sprites[0].GetTexture()->Pause();
}
