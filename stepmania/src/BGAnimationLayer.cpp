#include "global.h"
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
#include "SDL_utils.h"
#include <math.h>
#include "RageTimer.h"
#include "RageLog.h"


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


const float PARTICLE_SPEED = 300;

const float SPIRAL_MAX_ZOOM = 2;
const float SPIRAL_MIN_ZOOM = 0.3f;



BGAnimationLayer::BGAnimationLayer()
{
	Init();
}

void BGAnimationLayer::Init()
{
//	m_bCycleColor = false;
//	m_bCycleAlpha = false;
//	m_Effect = EFFECT_STRETCH_STILL;

	for( int i=0; i<MAX_SPRITES; i++ )
		m_vParticleVelocity[i] = RageVector3( 0, 0, 0 );

	m_Type = TYPE_SPRITE;

	m_fStretchTexCoordVelocityX = 0;
	m_fStretchTexCoordVelocityY = 0;
	m_bRewindMovie = false;
	m_fZoomMin = 1;
	m_fZoomMax = 1;
	m_fVelocityXMin = 10;
	m_fVelocityXMax = 10;
	m_fVelocityYMin = 0;
	m_fVelocityYMax = 0;
	m_fVelocityZMin = 0;
	m_fVelocityZMax = 0;
	m_fOverrideSpeed = 0;
	m_iNumParticles = 10;
	m_bParticlesBounce = false;
	m_iNumTilesWide = -1;
	m_iNumTilesHigh = -1;
	m_fTilesStartX = 0;
	m_fTilesStartY = 0;
	m_fTilesSpacingX = -1;
	m_fTilesSpacingY = -1;
	m_fTileVelocityX = 0;
	m_fTileVelocityY = 0;


	/* Why doesn't this use the existing tweening mechanism? I can't make
	 * sense of what this code is doing; all I can tell is that it's duplicating
	 * stuff we already have. -glenn */
	/*
	m_PosX = m_PosY = 0;
	m_Zoom = 0;
	m_Rot = 0;
	m_ShowTime = 0;
	m_HideTime = 0;
	m_TweenStartTime = 0;
	m_TweenX = m_TweenY = 0.0;
	m_TweenSpeed = 0;
	m_TweenState = 0;
	m_TweenPassedX = m_TweenPassedY = 0;
	*/
}

/* Static background layers are simple, uncomposited background images with nothing
 * behind them.  Since they have nothing behind them, they have no need for alpha,
 * so turn that off. */
void BGAnimationLayer::LoadFromStaticGraphic( CString sPath )
{
	Init();
	m_iNumSprites = 1;
	RageTextureID ID(sPath);
	ID.iAlphaBits = 0;
	m_Sprites[0].LoadBG( ID );
	m_Sprites[0].StretchTo( RectI(SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );
}

void BGAnimationLayer::LoadFromMovie( CString sMoviePath, bool bLoop, bool bRewind )
{
	Init();
	m_iNumSprites = 1;
	m_Sprites[0].LoadBG( sMoviePath );
	m_Sprites[0].StretchTo( RectI(SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );
	m_Sprites[0].GetTexture()->Play();
	SDL_Delay( 50 );	// decode a frame so we don't see a black flash at the beginning
	m_Sprites[0].GetTexture()->Pause();
	m_bRewindMovie = bRewind;
	if( !bLoop )
		m_Sprites[0].GetTexture()->SetLooping(false);
}

void BGAnimationLayer::LoadFromVisualization( CString sMoviePath )
{
	Init();
	m_iNumSprites = 1;
	m_Sprites[0].LoadBG( sMoviePath );
	m_Sprites[0].StretchTo( RectI(SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );
	m_Sprites[0].EnableAdditiveBlend( true );
}


void BGAnimationLayer::LoadFromAniLayerFile( CString sPath, CString sSongBGPath )
{
	Init();
	CString lcPath = sPath;
	lcPath.MakeLower();

	if( lcPath.Find("usesongbg") != -1 )
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
	};

	Effect effect = EFFECT_CENTER;

	for( int i=0; i<NUM_EFFECTS; i++ )
		if( lcPath.Find(EFFECT_STRING[i]) != -1 )
			effect = (Effect)i;

	switch( effect )
	{
	case EFFECT_CENTER:
		m_Type = TYPE_SPRITE;
		m_iNumSprites = 1;
		m_Sprites[0].Load( sPath );
		m_Sprites[0].SetXY( CENTER_X, CENTER_Y );
		break;
	case EFFECT_STRETCH_STILL:
	case EFFECT_STRETCH_SCROLL_LEFT:
	case EFFECT_STRETCH_SCROLL_RIGHT:
	case EFFECT_STRETCH_SCROLL_UP:
	case EFFECT_STRETCH_SCROLL_DOWN:
	case EFFECT_STRETCH_WATER:
	case EFFECT_STRETCH_BUBBLE:
	case EFFECT_STRETCH_TWIST:
		{
			m_Type = TYPE_STRETCH;
			m_iNumSprites = 1;
			RageTextureID ID(sPath);
			ID.bStretch = true;
			m_Sprites[0].LoadBG( ID );
			m_Sprites[0].StretchTo( RectI(SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );
			m_Sprites[0].SetCustomTextureRect( RectF(0,0,1,1) );

			switch( effect )
			{
			case EFFECT_STRETCH_SCROLL_LEFT:	m_fStretchTexCoordVelocityX = +0.5f; m_fStretchTexCoordVelocityY = 0;	break;
			case EFFECT_STRETCH_SCROLL_RIGHT:	m_fStretchTexCoordVelocityX = -0.5f; m_fStretchTexCoordVelocityY = 0;	break;
			case EFFECT_STRETCH_SCROLL_UP:		m_fStretchTexCoordVelocityX = 0; m_fStretchTexCoordVelocityY = +0.5f;	break;
			case EFFECT_STRETCH_SCROLL_DOWN:	m_fStretchTexCoordVelocityX = 0; m_fStretchTexCoordVelocityY = -0.5f;	break;
				break;
			}
		}
		break;
	case EFFECT_STRETCH_SPIN:
		{
			m_Type = TYPE_STRETCH;
			m_iNumSprites = 1;
			m_Sprites[0].LoadBG( sPath );
			m_Sprites[0].ScaleToCover( RectI(SCREEN_LEFT-200,SCREEN_TOP-200,SCREEN_RIGHT+200,SCREEN_BOTTOM+200) );
			m_Sprites[0].SetEffectSpin( RageVector3(0,0,1) );
		}
		break;
	case EFFECT_PARTICLES_SPIRAL_OUT:
	case EFFECT_PARTICLES_SPIRAL_IN:
/*		{
			m_Type = TYPE_PARTICLES;
			m_Sprites[0].Load( sPath );
			int iSpriteArea = int( m_Sprites[0].GetUnzoomedWidth()*m_Sprites[0].GetUnzoomedHeight() );
			int iMaxArea = SCREEN_WIDTH*SCREEN_HEIGHT;
			m_iNumSprites = m_iNumParticles = iMaxArea / iSpriteArea;
			m_iNumSprites = m_iNumParticles = min( m_iNumSprites, MAX_SPRITES );
			for( unsigned i=0; i<m_iNumSprites; i++ )
			{
				m_Sprites[i].Load( sPath );
				m_Sprites[i].SetZoom( randomf(0.2f,2) );
				m_Sprites[i].SetRotationZ( randomf(0,PI*2) );
			}
		}
		break;
*/	case EFFECT_PARTICLES_FLOAT_UP:
	case EFFECT_PARTICLES_FLOAT_DOWN:
	case EFFECT_PARTICLES_FLOAT_LEFT:
	case EFFECT_PARTICLES_FLOAT_RIGHT:
	case EFFECT_PARTICLES_BOUNCE:
		{
			m_Type = TYPE_PARTICLES;
			m_Sprites[0].Load( sPath );
			int iSpriteArea = int( m_Sprites[0].GetUnzoomedWidth()*m_Sprites[0].GetUnzoomedHeight() );
			int iMaxArea = SCREEN_WIDTH*SCREEN_HEIGHT;
			m_iNumSprites = m_iNumParticles = iMaxArea / iSpriteArea;
			m_iNumSprites = m_iNumParticles = min( m_iNumSprites, unsigned(MAX_SPRITES) );
			for( unsigned i=0; i<m_iNumSprites; i++ )
			{
				m_Sprites[i].Load( sPath );
				m_Sprites[i].SetZoom( 0.7f + 0.6f*i/(float)m_iNumSprites );
				m_Sprites[i].SetX( randomf( GetGuardRailLeft(&m_Sprites[i]), GetGuardRailRight(&m_Sprites[i]) ) );
				m_Sprites[i].SetY( randomf( GetGuardRailTop(&m_Sprites[i]), GetGuardRailBottom(&m_Sprites[i]) ) );

				switch( effect )
				{
				case EFFECT_PARTICLES_FLOAT_UP:
				case EFFECT_PARTICLES_SPIRAL_OUT:
					m_vParticleVelocity[i] = RageVector3( 0, -PARTICLE_SPEED*m_Sprites[i].GetZoom(), 0 );
					break;
				case EFFECT_PARTICLES_FLOAT_DOWN:
				case EFFECT_PARTICLES_SPIRAL_IN:
					m_vParticleVelocity[i] = RageVector3( 0, PARTICLE_SPEED*m_Sprites[i].GetZoom(), 0 );
					break;
				case EFFECT_PARTICLES_FLOAT_LEFT:
					m_vParticleVelocity[i] = RageVector3( -PARTICLE_SPEED*m_Sprites[i].GetZoom(), 0, 0 );
					break;
				case EFFECT_PARTICLES_FLOAT_RIGHT:
					m_vParticleVelocity[i] = RageVector3( +PARTICLE_SPEED*m_Sprites[i].GetZoom(), 0, 0 );
					break;
				case EFFECT_PARTICLES_BOUNCE:
					m_bParticlesBounce = true;
					m_Sprites[i].SetZoom( 1 );
					m_vParticleVelocity[i] = RageVector3( randomf(), randomf(), 0 );
					RageVec3Normalize( &m_vParticleVelocity[i], &m_vParticleVelocity[i] );
					break;
				default:
					ASSERT(0);
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
			m_Type = TYPE_TILES;
			RageTextureID ID(sPath);
			ID.bStretch = true;
			m_Sprites[0].Load( ID );
			m_iNumTilesWide = 2+int(SCREEN_WIDTH /m_Sprites[0].GetUnzoomedWidth());
			m_iNumTilesWide = min( m_iNumTilesWide, MAX_TILES_WIDE );
			m_iNumTilesHigh = 2+int(SCREEN_HEIGHT/m_Sprites[0].GetUnzoomedHeight());
			m_iNumTilesHigh = min( m_iNumTilesHigh, MAX_TILES_HIGH );
			m_iNumSprites = m_iNumTilesWide * m_iNumTilesHigh;
			m_fTilesStartX = m_Sprites[0].GetUnzoomedWidth() / 2;
			m_fTilesStartY = m_Sprites[0].GetUnzoomedHeight() / 2;
			m_fTilesSpacingX = m_Sprites[0].GetUnzoomedWidth();
			m_fTilesSpacingY = m_Sprites[0].GetUnzoomedHeight();
			// HACK:  fix cracks in tiles
			m_fTilesSpacingX -= 1;
			m_fTilesSpacingY -= 1;
			for( int x=0; x<m_iNumTilesWide; x++ )
			{
				for( int y=0; y<m_iNumTilesHigh; y++ )
				{
					int i = y*m_iNumTilesWide + x;
					m_Sprites[i].Load( ID );

					switch( effect )
					{
					case EFFECT_TILE_STILL:
						break;
					case EFFECT_TILE_SCROLL_LEFT:
						m_fTileVelocityX = -PARTICLE_SPEED;
						break;
					case EFFECT_TILE_SCROLL_RIGHT:
						m_fTileVelocityX = +PARTICLE_SPEED;
						break;
					case EFFECT_TILE_SCROLL_UP:
						m_fTileVelocityY = -PARTICLE_SPEED;
						break;
					case EFFECT_TILE_SCROLL_DOWN:
						m_fTileVelocityY = +PARTICLE_SPEED;
						break;
					case EFFECT_TILE_FLIP_X:
						m_Sprites[i].SetEffectSpin( RageVector3(2,0,0) );
						break;
					case EFFECT_TILE_FLIP_Y:
						m_Sprites[i].SetEffectSpin( RageVector3(0,2,0) );
						break;
					case EFFECT_TILE_PULSE:
						m_Sprites[i].SetEffectPulse( 1, 0.3f, 1.f );
						break;
					default:
						ASSERT(0);
					}
				}
			}
		}
		break;
	default:
		ASSERT(0);
	}


	sPath.MakeLower();

	if( sPath.Find("cyclecolor") != -1 )
		for( unsigned i=0; i<m_iNumSprites; i++ )
			m_Sprites[i].SetEffectRainbow( 5 );

	if( sPath.Find("cyclealpha") != -1 )
		for( unsigned i=0; i<m_iNumSprites; i++ )
			m_Sprites[i].SetEffectDiffuseShift( 2, RageColor(1,1,1,1), RageColor(1,1,1,0) );

	if( sPath.Find("startonrandomframe") != -1 )
		for( unsigned i=0; i<m_iNumSprites; i++ )
			m_Sprites[i].SetState( rand()%m_Sprites[i].GetNumStates() );

	if( sPath.Find("dontanimate") != -1 )
		for( unsigned i=0; i<m_iNumSprites; i++ )
			m_Sprites[i].StopAnimating();

	if( sPath.Find("add") != -1 )
		for( unsigned i=0; i<m_iNumSprites; i++ )
			m_Sprites[i].EnableAdditiveBlend( true );

	/*
	CString sDir, sFName, sExt;
	splitrelpath( sPath, sDir, sFName, sExt );
	CString sIniPath = sDir+"/"+sFName+".ini";
	IniFile ini;
	ini.SetPath( sIniPath );
	if( ini.ReadFile() )
	{
		ini.GetValueF( "BGAnimationLayer", "SetXpos", m_PosX );
		ini.GetValueF( "BGAnimationLayer", "SetYpos", m_PosY );
		ini.GetValueF( "BGAnimationLayer", "SetZoom", m_Zoom );
		ini.GetValueF( "BGAnimationLayer", "SetRot", m_Rot );
		ini.GetValueF( "BGAnimationLayer", "TweenStartTime", m_TweenStartTime );
		ini.GetValueF( "BGAnimationLayer", "TweenX", m_TweenX );
		ini.GetValueF( "BGAnimationLayer", "TweenY", m_TweenY );
		ini.GetValueF( "BGAnimationLayer", "TweenSpeed", m_TweenSpeed );
		ini.GetValueF( "BGAnimationLayer", "ShowTime", m_ShowTime );
		ini.GetValueF( "BGAnimationLayer", "HideTime", m_HideTime );
		ini.GetValueF( "BGAnimationLayer", "TexCoordVelocityX", m_vTexCoordVelocity.x );
		ini.GetValueF( "BGAnimationLayer", "TexCoordVelocityY", m_vTexCoordVelocity.y );
		ini.GetValueF( "BGAnimationLayer", "RotationalVelocity", m_fRotationalVelocity );
		ini.GetValueF( "BGAnimationLayer", "SetY", m_fStretchScrollH_Y );
	}

	if(m_ShowTime != 0) // they don't want to show until a certain point... hide it all
	{
		m_Sprites[0].SetDiffuse(RageColor(0,0,0,0));
	}
	if(m_PosX != 0)
	{
		m_Sprites[0].SetX(m_PosX);
	}
	if(m_PosY != 0)
	{
		m_Sprites[0].SetY(m_PosY);
	}
	if(m_Zoom != 0)
	{
		m_Sprites[0].SetZoom(m_Zoom);
	}
	if(m_Rot != 0)
	{
		m_Sprites[0].SetRotationZ(m_Rot);
	}
	*/
}


void BGAnimationLayer::LoadFromIni( CString sAniDir, CString sLayer, CString sSongBGPath )
{
	Init();
	if( sAniDir.Right(1) != "/" )
		sAniDir += "/";

	ASSERT( IsADirectory(sAniDir) );

	CString sPathToIni = sAniDir + "BGAnimation.ini";

	IniFile ini(sPathToIni);
	ini.ReadFile();

	CString sFile;
	ini.GetValue( sLayer, "File", sFile );

	bool bUseSongBG = false;
	ini.GetValueB( sLayer, "UseSongBG", bUseSongBG );
	if( bUseSongBG )
		sFile = sSongBGPath;

	if( sFile == "" )
		RageException::Throw( "In the ini file for BGAnimation '%s', '%s' is missing a the line 'File='.", sAniDir.GetString(), sLayer.GetString() );

	CString sPath = sAniDir+sFile;
	if( !DoesFileExist(sPath) )
		RageException::Throw( "In the ini file for BGAnimation '%s', the specified File '%s' does not exist.", sAniDir.GetString(), sFile.GetString() );

	ini.GetValueI( sLayer, "Type", (int&)m_Type );
	ini.GetValue ( sLayer, "Command", m_sCommand );
	ini.GetValueF( sLayer, "StretchTexCoordVelocityX", m_fStretchTexCoordVelocityX );
	ini.GetValueF( sLayer, "StretchTexCoordVelocityY", m_fStretchTexCoordVelocityY );
	ini.GetValueB( sLayer, "RewindMovie", m_bRewindMovie );
	ini.GetValueF( sLayer, "ZoomMin", m_fZoomMin );
	ini.GetValueF( sLayer, "ZoomMax", m_fZoomMax );
	ini.GetValueF( sLayer, "VelocityXMin", m_fVelocityXMin );
	ini.GetValueF( sLayer, "VelocityXMax", m_fVelocityXMax );
	ini.GetValueF( sLayer, "VelocityYMin", m_fVelocityYMin );
	ini.GetValueF( sLayer, "VelocityYMax", m_fVelocityYMax );
	ini.GetValueF( sLayer, "VelocityZMin", m_fVelocityZMin );
	ini.GetValueF( sLayer, "VelocityZMax", m_fVelocityZMax );
	ini.GetValueF( sLayer, "OverrideSpeed", m_fOverrideSpeed );
	ini.GetValueI( sLayer, "NumParticles", m_iNumParticles );
	ini.GetValueB( sLayer, "ParticlesBounce", m_bParticlesBounce );
//	ini.GetValueI( sLayer, "NumTilesWide", m_iNumTilesWide );	// infer from spacing (or else the Update logic breaks)
//	ini.GetValueI( sLayer, "NumTilesHigh", m_iNumTilesHigh );	// infer from spacing (or else the Update logic breaks)
	ini.GetValueF( sLayer, "TilesStartX", m_fTilesStartX );
	ini.GetValueF( sLayer, "TilesStartY", m_fTilesStartY );
	ini.GetValueF( sLayer, "TilesSpacingX", m_fTilesSpacingX );
	ini.GetValueF( sLayer, "TilesSpacingY", m_fTilesSpacingY );
	ini.GetValueF( sLayer, "TileVelocityX", m_fTileVelocityX );
	ini.GetValueF( sLayer, "TileVelocityY", m_fTileVelocityY );


	switch( m_Type )
	{
	case TYPE_SPRITE:
		m_iNumSprites = 1;
		m_Sprites[0].Load( sPath );
		m_Sprites[0].SetXY( CENTER_X, CENTER_Y );
		break;
	case TYPE_STRETCH:
		{
			m_iNumSprites = 1;
			RageTextureID ID(sPath);
			ID.bStretch = true;
			m_Sprites[0].LoadBG( ID );
			m_Sprites[0].StretchTo( RectI(SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM) );
			m_Sprites[0].SetCustomTextureRect( RectF(0,0,1,1) );
		}
		break;
	case TYPE_PARTICLES:
		{
			m_iNumSprites = m_iNumParticles;
			for( unsigned i=0; i<m_iNumSprites; i++ )
			{
				m_Sprites[i].Load( sPath );
				m_Sprites[i].SetXY( randomf(SCREEN_LEFT,SCREEN_RIGHT), randomf(SCREEN_TOP,SCREEN_BOTTOM) );
				m_Sprites[i].SetZoom( randomf(m_fZoomMin,m_fZoomMax) );
				m_vParticleVelocity[i] = RageVector3( 
					randomf(m_fVelocityXMin,m_fVelocityXMax),
					randomf(m_fVelocityYMin,m_fVelocityYMax),
					randomf(m_fVelocityZMin,m_fVelocityZMax) );
				if( m_fOverrideSpeed != 0 )
				{
					RageVec3Normalize( &m_vParticleVelocity[i], &m_vParticleVelocity[i] );
					m_vParticleVelocity[i] *= m_fOverrideSpeed;
				}
			}
		}
		break;
	case TYPE_TILES:
		{
			m_Sprites[0].Load( sPath );
			if( m_fTilesSpacingX == -1 )
			{
				m_fTilesSpacingX = m_Sprites[0].GetUnzoomedWidth();
				m_fTilesSpacingX -= 1;
			}
			if( m_fTilesSpacingY == -1 )
			{
				m_fTilesSpacingY = m_Sprites[0].GetUnzoomedHeight();
				m_fTilesSpacingY -= 1;
			}
			m_iNumTilesWide = 2+(int)(SCREEN_WIDTH /m_fTilesSpacingX);
			m_iNumTilesHigh = 2+(int)(SCREEN_HEIGHT/m_fTilesSpacingY);
			m_iNumSprites = m_iNumTilesWide * m_iNumTilesHigh;
			for( unsigned i=0; i<m_iNumSprites; i++ )
			{
				m_Sprites[i].Load( sPath );
				//m_Sprites[i].SetXY( );	// let Update set X and Y
				m_Sprites[i].SetZoom( randomf(m_fZoomMin,m_fZoomMax) );
			}
		}
		break;
	default:
		ASSERT(0);
	}

	bool bStartOnRandomFrame = false;
	ini.GetValueB( sLayer, "StartOnRandomFrame", bStartOnRandomFrame );
	if( bStartOnRandomFrame )
	{
		for( unsigned i=0; i<m_iNumSprites; i++ )
			m_Sprites[i].SetState( rand()%m_Sprites[i].GetNumStates() );
	}

	if( m_sCommand != "" )
	{
		for( unsigned i=0; i<m_iNumSprites; i++ )
			m_Sprites[i].Command( m_sCommand );
	}
}

void BGAnimationLayer::Update( float fDeltaTime )
{
	const float fSongBeat = GAMESTATE->m_fSongBeat;
	
	unsigned i;
	for( i=0; i<m_iNumSprites; i++ )
		m_Sprites[i].Update( fDeltaTime );


	switch( m_Type )
	{
	case TYPE_SPRITE:
		break;
	case TYPE_STRETCH:
		{
			float fTexCoords[8];
			m_Sprites[0].GetCustomTextureCoords( fTexCoords );

			for( i=0; i<8; i+=2 )
			{
				fTexCoords[i  ] += fDeltaTime*m_fStretchTexCoordVelocityX;
				fTexCoords[i+1] += fDeltaTime*m_fStretchTexCoordVelocityY;
			}

			m_Sprites[0].SetCustomTextureCoords( fTexCoords );
		}
		break;
/*	case EFFECT_PARTICLES_SPIRAL_OUT:
		for( i=0; i<m_iNumSprites; i++ )
		{
			m_Sprites[i].SetZoom( m_Sprites[i].GetZoom() + fDeltaTime );
			if( m_Sprites[i].GetZoom() > SPIRAL_MAX_ZOOM )
				m_Sprites[i].SetZoom( SPIRAL_MIN_ZOOM );

			m_Sprites[i].SetRotationZ( m_Sprites[i].GetRotationZ() + fDeltaTime );

			float fRadius = (m_Sprites[i].GetZoom()-SPIRAL_MIN_ZOOM);
			fRadius *= fRadius;
			fRadius *= 200;
			m_Sprites[i].SetX( CENTER_X + cosf(m_Sprites[i].GetRotationZ())*fRadius );
			m_Sprites[i].SetY( CENTER_Y + sinf(m_Sprites[i].GetRotationZ())*fRadius );
		}
		break;
	case EFFECT_PARTICLES_SPIRAL_IN:
		for( i=0; i<m_iNumSprites; i++ )
		{
			m_Sprites[i].SetZoom( m_Sprites[i].GetZoom() - fDeltaTime );
			if( m_Sprites[i].GetZoom() < SPIRAL_MIN_ZOOM )
				m_Sprites[i].SetZoom( SPIRAL_MAX_ZOOM );

			m_Sprites[i].SetRotationZ( m_Sprites[i].GetRotationZ() - fDeltaTime );

			float fRadius = (m_Sprites[i].GetZoom()-SPIRAL_MIN_ZOOM);
			fRadius *= fRadius;
			fRadius *= 200;
			m_Sprites[i].SetX( CENTER_X + cosf(m_Sprites[i].GetRotationZ())*fRadius );
			m_Sprites[i].SetY( CENTER_Y + sinf(m_Sprites[i].GetRotationZ())*fRadius );
		}
		break;
*/
	case TYPE_PARTICLES:
		for( i=0; i<m_iNumSprites; i++ )
		{
			m_Sprites[i].SetX( m_Sprites[i].GetX() + fDeltaTime*m_vParticleVelocity[i].x  );
			m_Sprites[i].SetY( m_Sprites[i].GetY() + fDeltaTime*m_vParticleVelocity[i].y  );
			m_Sprites[i].SetZ( m_Sprites[i].GetZ() + fDeltaTime*m_vParticleVelocity[i].z  );
			if( m_bParticlesBounce )
			{
				if( HitGuardRailLeft(&m_Sprites[i]) )	
				{
					m_vParticleVelocity[i].x *= -1;
					m_Sprites[i].SetX( GetGuardRailLeft(&m_Sprites[i]) );
				}
				if( HitGuardRailRight(&m_Sprites[i]) )	
				{
					m_vParticleVelocity[i].x *= -1;
					m_Sprites[i].SetX( GetGuardRailRight(&m_Sprites[i]) );
				}
				if( HitGuardRailTop(&m_Sprites[i]) )	
				{
					m_vParticleVelocity[i].y *= -1;
					m_Sprites[i].SetY( GetGuardRailTop(&m_Sprites[i]) );
				}
				if( HitGuardRailBottom(&m_Sprites[i]) )	
				{
					m_vParticleVelocity[i].y *= -1;
					m_Sprites[i].SetY( GetGuardRailBottom(&m_Sprites[i]) );
				}
			}
			else // !m_bParticlesBounce 
			{
				if( m_vParticleVelocity[i].x<0  &&  IsOffScreenLeft(&m_Sprites[i]) )
					m_Sprites[i].SetX( GetOffScreenRight(&m_Sprites[i]) );
				if( m_vParticleVelocity[i].x>0  &&  IsOffScreenRight(&m_Sprites[i]) )
					m_Sprites[i].SetX( GetOffScreenLeft(&m_Sprites[i]) );
				if( m_vParticleVelocity[i].y<0  &&  IsOffScreenTop(&m_Sprites[i]) )
					m_Sprites[i].SetY( GetOffScreenBottom(&m_Sprites[i]) );
				if( m_vParticleVelocity[i].y>0  &&  IsOffScreenBottom(&m_Sprites[i]) )
					m_Sprites[i].SetY( GetOffScreenTop(&m_Sprites[i]) );
			}
		}
		break;
	case TYPE_TILES:
		{
			float fSecs = RageTimer::GetTimeSinceStart();
			float fTotalWidth = m_iNumTilesWide * m_fTilesSpacingX;
			float fTotalHeight = m_iNumTilesHigh * m_fTilesSpacingY;
			
			ASSERT( int(m_iNumSprites) == m_iNumTilesWide * m_iNumTilesHigh );

			for( int x=0; x<m_iNumTilesWide; x++ )
			{
				for( int y=0; y<m_iNumTilesHigh; y++ )
				{
					int i = y*m_iNumTilesWide + x;

					float fX = m_fTilesStartX + m_fTilesSpacingX * x + fSecs * m_fTileVelocityX;
					float fY = m_fTilesStartY + m_fTilesSpacingY * y + fSecs * m_fTileVelocityY;

					fX += m_fTilesSpacingX/2;
					fY += m_fTilesSpacingY/2;

					fX = fmodf( fX, fTotalWidth );
					fY = fmodf( fY, fTotalHeight );

					if( fX < 0 )	fX += fTotalWidth;
					if( fY < 0 )	fY += fTotalHeight;

					fX -= m_fTilesSpacingX/2;
					fY -= m_fTilesSpacingY/2;
					
					m_Sprites[i].SetX( fX );
					m_Sprites[i].SetY( fY );
				}
			}
/*			
		for( i=0; i<m_iNumSprites; i++ )
		{
			m_Sprites[i].SetX( m_Sprites[i].GetX() + fDeltaTime*  );
			m_Sprites[i].SetY( m_Sprites[i].GetY() + fDeltaTime*m_vParticleVelocity[i].y  );
			m_Sprites[i].SetZ( m_Sprites[i].GetZ() + fDeltaTime*m_vParticleVelocity[i].z  );
			if( IsOffScreenLeft(&m_Sprites[i]) )
				m_Sprites[i].SetX( m_Sprites[i].GetX()-GetOffScreenLeft(&m_Sprites[i]) + GetOffScreenRight(&m_Sprites[i]) );
			if( IsOffScreenRight(&m_Sprites[i]) )
				m_Sprites[i].SetX( m_Sprites[i].GetX()-GetOffScreenRight(&m_Sprites[i]) + GetOffScreenLeft(&m_Sprites[i]) );
			if( IsOffScreenTop(&m_Sprites[i]) )
				m_Sprites[i].SetY( m_Sprites[i].GetY()-GetOffScreenTop(&m_Sprites[i]) + GetOffScreenBottom(&m_Sprites[i]) );
			if( IsOffScreenBottom(&m_Sprites[i]) )
				m_Sprites[i].SetY( m_Sprites[i].GetY()-GetOffScreenBottom(&m_Sprites[i]) + GetOffScreenTop(&m_Sprites[i]) );
				*/
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

	/*
	if(m_TweenStartTime != 0 && !(m_TweenStartTime < 0))
	{
		m_TweenStartTime -= fDeltaTime;
		if(m_TweenStartTime <= 0) // if we've gone past the magic point... show the beast....
		{
		//	m_Sprites[0].SetTweenXY( m_TweenX, m_TweenY);
			
			// WHAT WOULD BE NICE HERE:
			// Set the Sprite Tweening To m_TweenX and m_TweenY
			// Going as fast as m_TweenSpeed specifies.
			// however, TWEEN falls over on its face at this point.
			// Lovely.
			// Instead: Manual tweening. Blah.
			m_TweenState = 1;
			if(m_PosX == m_TweenX)
			{
				m_TweenPassedX = 1;
			}
			if(m_PosY == m_TweenY)
			{
				m_TweenPassedY = 1;
			}
		}		
	}

	if(m_TweenState) // A FAR from perfect Tweening Mechanism.
	{
		if(m_TweenPassedY != 1) // Check to see if we still need to Tween Along the Y Axis
		{
			if(m_Sprites[0].GetY() < m_TweenY) // it needs to travel down
			{
				// Speed = Distance / Time....
				// Take away from the current position... the distance it has to travel divided by the time they want it done in...
				m_Sprites[0].SetY(m_Sprites[0].GetY() + ((m_TweenY - m_PosY)/(m_TweenSpeed*60)));

				if(m_Sprites[0].GetY() > m_TweenY) // passed the location we wanna go to?
				{
					m_Sprites[0].SetY(m_TweenY); // set it to the exact location we want
					m_TweenPassedY = 1; // say we passed it.
				}
			}
			else // travelling up
			{
				m_Sprites[0].SetY(m_Sprites[0].GetY() - ((m_TweenY + m_PosY)/(m_TweenSpeed*60)));

				if(m_Sprites[0].GetY() < m_TweenY)
				{
					m_Sprites[0].SetY(m_TweenY);
					m_TweenPassedY = 1;
				}
			}
		}

		if(m_TweenPassedX != 1) // Check to see if we still need to Tween Along the X Axis
		{
			if(m_Sprites[0].GetX() < m_TweenX) // it needs to travel right
			{
				m_Sprites[0].SetX(m_Sprites[0].GetX() + ((m_TweenX - m_PosX)/(m_TweenSpeed*60)));
				if(m_Sprites[0].GetX() > m_TweenX)
				{
					m_Sprites[0].SetX(m_TweenX);
					m_TweenPassedX = 1;
				}
			}
			else // travelling left
			{
				m_Sprites[0].SetX(m_Sprites[0].GetX() - ((m_TweenX + m_PosX)/(m_TweenSpeed*60)));
				if(m_Sprites[0].GetX() < m_TweenX)
				{
					m_Sprites[0].SetX(m_TweenX);
					m_TweenPassedX = 1;
				}
			}
		}

		if(m_TweenPassedY == 1 && m_TweenPassedX == 1) // totally passed both X and Y? Stop tweening.
		{
			m_TweenState = 0;
		}
	}

	if(m_ShowTime != 0 && !(m_ShowTime < 0))
	{
		m_ShowTime -= fDeltaTime;
		if(m_ShowTime <= 0) // if we've gone past the magic point... show the beast....
		{
			m_Sprites[0].SetDiffuse( RageColor(1,1,1,1) );
		}		
	}
	if(m_HideTime != 0 && !(m_HideTime < 0)) // make sure it's not 0 or less than 0...
	{
		m_HideTime -= fDeltaTime;
		if(m_HideTime <= 0) // if we've gone past the magic point... hide the beast....
		{
			m_Sprites[0].SetDiffuse( RageColor(0,0,0,0) );
		}
		
	}
	*/
}

void BGAnimationLayer::Draw()
{
	for( unsigned i=0; i<m_iNumSprites; i++ )
		m_Sprites[i].Draw();
}

void BGAnimationLayer::SetDiffuse( RageColor c )
{
	for(unsigned i=0; i<m_iNumSprites; i++) 
		m_Sprites[i].SetDiffuse(c);
}

void BGAnimationLayer::GainingFocus()
{
	if( m_bRewindMovie )
		m_Sprites[0].GetTexture()->SetPosition( 0 );
	// if movie texture, pause and play movie so we don't waste CPU cycles decoding frames that won't be shown
	m_Sprites[0].GetTexture()->Play();

	for( unsigned i=0; i<m_iNumSprites; i++ )
		m_Sprites[i].Command( m_sCommand );
}

void BGAnimationLayer::LosingFocus()
{
	m_Sprites[0].GetTexture()->Pause();
}
