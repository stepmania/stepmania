#include "global.h"
#include "BGAnimationLayer.h"
#include "PrefsManager.h"
#include "GameState.h"
#include "IniFile.h"
#include "RageMath.h"
#include "RageTimer.h"
#include "RageLog.h"
#include "song.h"
#include "ActorCollision.h"
#include "Sprite.h"
#include "RageDisplay.h"
#include "ActorUtil.h"
#include "arch/ArchHooks/ArchHooks.h"
#include "RageTextureManager.h"
#include "RageFile.h"
#include "LuaHelpers.h"


const float PARTICLE_SPEED = 300;

const float SPIRAL_MAX_ZOOM = 2;
const float SPIRAL_MIN_ZOOM = 0.3f;

#define MAX_TILES_WIDE int(SCREEN_WIDTH/32+2)
#define MAX_TILES_HIGH int(SCREEN_HEIGHT/32+2)
#define MAX_SPRITES (MAX_TILES_WIDE*MAX_TILES_HIGH)

#define FullScreenRectF RectF(SCREEN_LEFT,SCREEN_TOP,SCREEN_RIGHT,SCREEN_BOTTOM)


BGAnimationLayer::BGAnimationLayer( bool Generic )
{
	/* If Generic is false, this is a layer in a real BGA--one that was loaded
	 * by simply constructing a BGAnimation.  These normally have a position
	 * of 0,0 (top-left of the screen).  Loaded images are given a default position
	 * of 320x240, centered in the screen.  Additionally, the "On" command will
	 * be run automatically.  Example:
	 *
	 * BGAnimation bga;
	 * bga.Load( path );
	 * this->AddChind( &bga );
	 *
	 * If Generic is true, then we act like any other actor.  We assume we don't
	 * know anything about where we're positioned.  Loaded images are given a
	 * default position of 0x0.  The "On" command is not run; we'll receive that
	 * from the owner through COMMAND().  Example:
	 * 
	 * AutoActor image;
	 * image.Load( path );
	 * ON_COMMAND( image );
	 * this->AddChind( &image );
	 *
	 * TYPE_PARTICLES and TYPE_TILES are currently not supported in this mode.
	 * 
	 */
	m_bGeneric = Generic;

	Init();
}

BGAnimationLayer::~BGAnimationLayer()
{
	Unload();
}

void BGAnimationLayer::Unload()
{
	ActorFrame::DeleteAllChildren();
}

void BGAnimationLayer::Init()
{
	Unload();

	m_fRepeatCommandEverySeconds = -1;
	m_fSecondsUntilNextCommand = 0;
	m_fUpdateRate = 1;
	m_fFOV = -1;	// no change
	m_bLighting = false;

	m_vParticleVelocity.clear();

	m_Type = TYPE_SPRITE;

	m_fTexCoordVelocityX = 0;
	m_fTexCoordVelocityY = 0;
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
}

/* Static background layers are simple, uncomposited background images with nothing
 * behind them.  Since they have nothing behind them, they have no need for alpha,
 * so turn that off. */
void BGAnimationLayer::LoadFromStaticGraphic( const CString& sPath )
{
	Init();
	Sprite* pSprite = new Sprite;
	pSprite->LoadBG( sPath );
	pSprite->StretchTo( FullScreenRectF );
	this->AddChild( pSprite );
}

void BGAnimationLayer::LoadFromMovie( const CString& sMoviePath )
{
	Init();
	Sprite* pSprite = new Sprite;
	pSprite->LoadBG( sMoviePath );
	pSprite->StretchTo( FullScreenRectF );
	pSprite->EnableAnimation( false );
	this->AddChild( pSprite );
}

void BGAnimationLayer::LoadFromVisualization( const CString& sMoviePath )
{
	Init();
	Sprite* pSprite = new Sprite;
	this->AddChild( pSprite );
	pSprite->LoadBG( sMoviePath );
	pSprite->StretchTo( FullScreenRectF );
	pSprite->SetBlendMode( BLEND_ADD );
}


void BGAnimationLayer::LoadFromAniLayerFile( const CString& sPath )
{
	/* Generic BGAs are new.  Animation directories with no INI are old and obsolete. 
	 * Don't combine them. */
	ASSERT( !m_bGeneric );
	Init();
	CString lcPath = sPath;
	lcPath.MakeLower();

	if( lcPath.Find("usesongbg") != -1 )
	{
		const Song* pSong = GAMESTATE->m_pCurSong;
		CString sSongBGPath;
		if( pSong && pSong->HasBackground() )
			sSongBGPath = pSong->GetBackgroundPath();
		else
			sSongBGPath = THEME->GetPathToG("Common fallback background");

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
		{
			m_Type = TYPE_SPRITE;
			Sprite* pSprite = new Sprite;
			this->AddChild( pSprite );
			pSprite->Load( sPath );
			pSprite->SetXY( SCREEN_CENTER_X, SCREEN_CENTER_Y );
		}
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
			m_Type = TYPE_SPRITE;
			Sprite* pSprite = new Sprite;
			this->AddChild( pSprite );
			RageTextureID ID(sPath);
			ID.bStretch = true;
			pSprite->LoadBG( ID );
			pSprite->StretchTo( FullScreenRectF );
			pSprite->SetCustomTextureRect( RectF(0,0,1,1) );

			switch( effect )
			{
			case EFFECT_STRETCH_SCROLL_LEFT:	m_fTexCoordVelocityX = +0.5f; m_fTexCoordVelocityY = 0;	break;
			case EFFECT_STRETCH_SCROLL_RIGHT:	m_fTexCoordVelocityX = -0.5f; m_fTexCoordVelocityY = 0;	break;
			case EFFECT_STRETCH_SCROLL_UP:		m_fTexCoordVelocityX = 0; m_fTexCoordVelocityY = +0.5f;	break;
			case EFFECT_STRETCH_SCROLL_DOWN:	m_fTexCoordVelocityX = 0; m_fTexCoordVelocityY = -0.5f;	break;
				break;
			}
		}
		break;
	case EFFECT_STRETCH_SPIN:
		{
			m_Type = TYPE_SPRITE;
			Sprite* pSprite = new Sprite;
			this->AddChild( pSprite );
			pSprite->LoadBG( sPath );
			const RectF StretchedFullScreenRectF(
				FullScreenRectF.left-200,
				FullScreenRectF.top-200,
				FullScreenRectF.right+200,
				FullScreenRectF.bottom+200 );

			pSprite->ScaleToCover( StretchedFullScreenRectF );
			pSprite->SetEffectSpin( RageVector3(0,0,60) );
		}
		break;
	case EFFECT_PARTICLES_SPIRAL_OUT:
	case EFFECT_PARTICLES_SPIRAL_IN:
	case EFFECT_PARTICLES_FLOAT_UP:
	case EFFECT_PARTICLES_FLOAT_DOWN:
	case EFFECT_PARTICLES_FLOAT_LEFT:
	case EFFECT_PARTICLES_FLOAT_RIGHT:
	case EFFECT_PARTICLES_BOUNCE:
		{
			m_Type = TYPE_PARTICLES;
			Sprite s;
			s.Load( sPath );
			int iSpriteArea = int( s.GetUnzoomedWidth()*s.GetUnzoomedHeight() );
			const int iMaxArea = int(SCREEN_WIDTH*SCREEN_HEIGHT);
			m_iNumParticles = iMaxArea / iSpriteArea;
			m_iNumParticles = min( m_iNumParticles, MAX_SPRITES );

			for( int i=0; i<m_iNumParticles; i++ )
			{
				Sprite* pSprite = new Sprite;
				this->AddChild( pSprite );
				pSprite->Load( sPath );
				pSprite->SetZoom( 0.7f + 0.6f*i/(float)m_iNumParticles );
				pSprite->SetX( randomf( GetGuardRailLeft(pSprite), GetGuardRailRight(pSprite) ) );
				pSprite->SetY( randomf( GetGuardRailTop(pSprite), GetGuardRailBottom(pSprite) ) );

				switch( effect )
				{
				case EFFECT_PARTICLES_FLOAT_UP:
				case EFFECT_PARTICLES_SPIRAL_OUT:
					m_vParticleVelocity.push_back( RageVector3( 0, -PARTICLE_SPEED*pSprite->GetZoom(), 0 ) );
					break;
				case EFFECT_PARTICLES_FLOAT_DOWN:
				case EFFECT_PARTICLES_SPIRAL_IN:
					m_vParticleVelocity.push_back( RageVector3( 0, PARTICLE_SPEED*pSprite->GetZoom(), 0 ) );
					break;
				case EFFECT_PARTICLES_FLOAT_LEFT:
					m_vParticleVelocity.push_back( RageVector3( -PARTICLE_SPEED*pSprite->GetZoom(), 0, 0 ) );
					break;
				case EFFECT_PARTICLES_FLOAT_RIGHT:
					m_vParticleVelocity.push_back( RageVector3( +PARTICLE_SPEED*pSprite->GetZoom(), 0, 0 ) );
					break;
				case EFFECT_PARTICLES_BOUNCE:
					m_bParticlesBounce = true;
					pSprite->SetZoom( 1 );
					m_vParticleVelocity.push_back( RageVector3( randomf(), randomf(), 0 ) );
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
			Sprite s;
			s.Load( ID );
			m_iNumTilesWide = 2+int(SCREEN_WIDTH /s.GetUnzoomedWidth());
			m_iNumTilesWide = min( m_iNumTilesWide, MAX_TILES_WIDE );
			m_iNumTilesHigh = 2+int(SCREEN_HEIGHT/s.GetUnzoomedHeight());
			m_iNumTilesHigh = min( m_iNumTilesHigh, MAX_TILES_HIGH );
			m_fTilesStartX = s.GetUnzoomedWidth() / 2;
			m_fTilesStartY = s.GetUnzoomedHeight() / 2;
			m_fTilesSpacingX = s.GetUnzoomedWidth();
			m_fTilesSpacingY = s.GetUnzoomedHeight();
			for( int x=0; x<m_iNumTilesWide; x++ )
			{
				for( int y=0; y<m_iNumTilesHigh; y++ )
				{
					Sprite* pSprite = new Sprite;
					this->AddChild( pSprite );
					pSprite->Load( ID );
					pSprite->SetTextureWrapping( true );	// gets rid of some "cracks"

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
						pSprite->SetEffectSpin( RageVector3(2,0,0) );
						break;
					case EFFECT_TILE_FLIP_Y:
						pSprite->SetEffectSpin( RageVector3(0,2,0) );
						break;
					case EFFECT_TILE_PULSE:
						pSprite->SetEffectPulse( 1, 0.3f, 1.f );
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


	CString sHint = sPath;
	sHint.MakeLower();

	if( sHint.Find("cyclecolor") != -1 )
		for( unsigned i=0; i<m_SubActors.size(); i++ )
			m_SubActors[i]->SetEffectRainbow( 5 );

	if( sHint.Find("cyclealpha") != -1 )
		for( unsigned i=0; i<m_SubActors.size(); i++ )
			m_SubActors[i]->SetEffectDiffuseShift( 2, RageColor(1,1,1,1), RageColor(1,1,1,0) );

	if( sHint.Find("startonrandomframe") != -1 )
		for( unsigned i=0; i<m_SubActors.size(); i++ )
			m_SubActors[i]->SetState( rand()%m_SubActors[i]->GetNumStates() );

	if( sHint.Find("dontanimate") != -1 )
		for( unsigned i=0; i<m_SubActors.size(); i++ )
			m_SubActors[i]->StopAnimating();

	if( sHint.Find("add") != -1 )
		for( unsigned i=0; i<m_SubActors.size(); i++ )
			m_SubActors[i]->SetBlendMode( BLEND_ADD );
}

void BGAnimationLayer::LoadFromIni( const CString& sAniDir_, const IniKey& layer )
{
	CString sAniDir = sAniDir_;

	Init();
	if( sAniDir.Right(1) != "/" )
		sAniDir += "/";

	DEBUG_ASSERT( IsADirectory(sAniDir) );

	CHECKPOINT_M( ssprintf( "BGAnimationLayer::LoadFromIni \"%s\" %s",
		sAniDir.c_str(), m_bGeneric? "(generic) ":"" ) );

	{
		CString sPlayer;
		if( layer.GetValue("Player", sPlayer) )
			ASSERT_M( 0, "The BGAnimation parameter 'Player' is deprecated.  Please use 'Condition=IsPlayerEnabled(p)'." );
	}

	{
		CString expr;
		if( layer.GetValue("Cond",expr) || layer.GetValue("Condition",expr) )
		{
			if( !Lua::RunExpressionB( expr ) )
				return;
		}
	}

	bool bStretch = false;
	{
		CString type = "sprite";
		layer.GetValue( "Type", type );
		type.MakeLower();

		/* The preferred way of stretching a sprite to fit the screen is "Type=sprite"
		 * and "stretch=1".  "type=1" is for backwards-compatibility. */
		layer.GetValue( "Stretch", bStretch );

		// Check for string match first, then do integer match.
		// "if(atoi(type)==0)" was matching against all string matches.
		// -Chris
		if( stricmp(type,"sprite")==0 )
		{
			m_Type = TYPE_SPRITE;
		}
		else if( stricmp(type,"particles")==0 )
		{
			m_Type = TYPE_PARTICLES;
		}
		else if( stricmp(type,"tiles")==0 )
		{
			m_Type = TYPE_TILES;
		}
		else if( atoi(type) == 1 )
		{
			m_Type = TYPE_SPRITE; 
			bStretch = true; 
		}
		else if( atoi(type) == 2 )
		{
			m_Type = TYPE_PARTICLES; 
		}
		else if( atoi(type) == 3 )
		{
			m_Type = TYPE_TILES; 
		}
		else
		{
			m_Type = TYPE_SPRITE;
		}
	}
#if 0
	{
		for( IniKey::const_iterator i = layer.begin();
			 i != layer.end(); ++i)
		{
			CString KeyName = i->first; /* "OnCommand" */
			KeyName.MakeLower();

			if( KeyName.Right(7) != "command" )
				continue; /* not a command */

			const CString &sData = i->second;
			Commands cmds = ParseCommands( sData );
			CString sCmdName;
			/* Special case: "Command=foo" -> "OnCommand=foo" */
			if( KeyName.size() == 7 )
				sCmdName="on";
			else
				sCmdName = KeyName.Left( KeyName.size()-7 );
			m_mapNameToCommands[sCmdName] = cmds;
		}
	}
#endif

	layer.GetValue( "CommandRepeatSeconds", m_fRepeatCommandEverySeconds );
	m_fSecondsUntilNextCommand = m_fRepeatCommandEverySeconds;
	layer.GetValue( "FOV", m_fFOV );
	layer.GetValue( "Lighting", m_bLighting );
	layer.GetValue( "TexCoordVelocityX", m_fTexCoordVelocityX );
	layer.GetValue( "TexCoordVelocityY", m_fTexCoordVelocityY );
	layer.GetValue( "DrawCond", m_sDrawCond );

	// compat:
	layer.GetValue( "StretchTexCoordVelocityX", m_fTexCoordVelocityX );
	layer.GetValue( "StretchTexCoordVelocityY", m_fTexCoordVelocityY );
	layer.GetValue( "ZoomMin", m_fZoomMin );
	layer.GetValue( "ZoomMax", m_fZoomMax );
	layer.GetValue( "VelocityXMin", m_fVelocityXMin );
	layer.GetValue( "VelocityXMax", m_fVelocityXMax );
	layer.GetValue( "VelocityYMin", m_fVelocityYMin );
	layer.GetValue( "VelocityYMax", m_fVelocityYMax );
	layer.GetValue( "VelocityZMin", m_fVelocityZMin );
	layer.GetValue( "VelocityZMax", m_fVelocityZMax );
	layer.GetValue( "OverrideSpeed", m_fOverrideSpeed );
	layer.GetValue( "NumParticles", m_iNumParticles );
	layer.GetValue( "ParticlesBounce", m_bParticlesBounce );
	layer.GetValue( "TilesStartX", m_fTilesStartX );
	layer.GetValue( "TilesStartY", m_fTilesStartY );
	layer.GetValue( "TilesSpacingX", m_fTilesSpacingX );
	layer.GetValue( "TilesSpacingY", m_fTilesSpacingY );
	layer.GetValue( "TileVelocityX", m_fTileVelocityX );
	layer.GetValue( "TileVelocityY", m_fTileVelocityY );


	bool NeedTextureStretch = false;
	if( m_fTexCoordVelocityX != 0 ||
		m_fTexCoordVelocityY != 0 )
		NeedTextureStretch = true;

	switch( m_Type )
	{
	case TYPE_SPRITE:
		{
			Actor* pActor = LoadFromActorFile( sAniDir, layer );
			this->AddChild( pActor );
			if( !m_bGeneric )
			{
				if( bStretch )
					pActor->StretchTo( FullScreenRectF );
				else
					pActor->SetXY( SCREEN_CENTER_X, SCREEN_CENTER_Y );
			}
		}
		break;
	case TYPE_PARTICLES:
		{
			CString sFile;
			layer.GetValue( "File", sFile );
			FixSlashesInPlace( sFile );
			
			CString sPath = sAniDir+sFile;
			CollapsePath( sPath );


			ASSERT( !m_bGeneric );
			for( int i=0; i<m_iNumParticles; i++ )
			{
				Actor* pActor = MakeActor( sPath );
				this->AddChild( pActor );
				pActor->SetXY( randomf(float(FullScreenRectF.left),float(FullScreenRectF.right)),
							   randomf(float(FullScreenRectF.top),float(FullScreenRectF.bottom)) );
				pActor->SetZoom( randomf(m_fZoomMin,m_fZoomMax) );
				m_vParticleVelocity.push_back( RageVector3( 
					randomf(m_fVelocityXMin,m_fVelocityXMax),
					randomf(m_fVelocityYMin,m_fVelocityYMax),
					randomf(m_fVelocityZMin,m_fVelocityZMax) ) );
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
			CString sFile;
			layer.GetValue( "File", sFile );
			FixSlashesInPlace( sFile );
			
			CString sPath = sAniDir+sFile;
			CollapsePath( sPath );

			ASSERT( !m_bGeneric );
			Sprite s;
			RageTextureID ID(sPath);
			ID.bStretch = true;
			s.Load( ID );
			if( m_fTilesSpacingX == -1 )
				m_fTilesSpacingX = s.GetUnzoomedWidth();
			if( m_fTilesSpacingY == -1 )
				m_fTilesSpacingY = s.GetUnzoomedHeight();
			m_iNumTilesWide = 2+(int)(SCREEN_WIDTH /m_fTilesSpacingX);
			m_iNumTilesHigh = 2+(int)(SCREEN_HEIGHT/m_fTilesSpacingY);
			unsigned NumSprites = m_iNumTilesWide * m_iNumTilesHigh;
			for( unsigned i=0; i<NumSprites; i++ )
			{
				Sprite* pSprite = new Sprite;
				this->AddChild( pSprite );
				pSprite->Load( ID );
				pSprite->SetTextureWrapping( true );		// gets rid of some "cracks"
				pSprite->SetZoom( randomf(m_fZoomMin,m_fZoomMax) );
			}
		}
		break;
	default:
		ASSERT(0);
	}

	bool bStartOnRandomFrame = false;
	layer.GetValue( "StartOnRandomFrame", bStartOnRandomFrame );
	if( bStartOnRandomFrame )
	{
		for( unsigned i=0; i<m_SubActors.size(); i++ )
			m_SubActors[i]->SetState( rand()%m_SubActors[i]->GetNumStates() );
	}

	if( !m_bGeneric )
		PlayCommand( "On" );
}

void BGAnimationLayer::Update( float fDeltaTime )
{
	if( m_fHibernateSecondsLeft > 0 )
		return;

	fDeltaTime *= m_fUpdateRate;

	const float fSongBeat = GAMESTATE->m_fSongBeat;
	
	for( unsigned i=0; i<m_SubActors.size(); i++ )
		m_SubActors[i]->Update( fDeltaTime );


	switch( m_Type )
	{
	case TYPE_SPRITE:
		if( m_fTexCoordVelocityX || m_fTexCoordVelocityY )
		{
			for( unsigned i=0; i<m_SubActors.size(); i++ )
			{
				Sprite *pSprite = (Sprite*)m_SubActors[i];
				pSprite->StretchTexCoords(
					fDeltaTime*m_fTexCoordVelocityX,
					fDeltaTime*m_fTexCoordVelocityY );
			}
		}
		break;
	case EFFECT_PARTICLES_SPIRAL_OUT:
		for( unsigned i=0; i<m_SubActors.size(); i++ )
		{
			m_SubActors[i]->SetZoom( m_SubActors[i]->GetZoom() + fDeltaTime );
			if( m_SubActors[i]->GetZoom() > SPIRAL_MAX_ZOOM )
				m_SubActors[i]->SetZoom( SPIRAL_MIN_ZOOM );

			m_SubActors[i]->SetRotationZ( m_SubActors[i]->GetRotationZ() + fDeltaTime );

			float fRadius = (m_SubActors[i]->GetZoom()-SPIRAL_MIN_ZOOM);
			fRadius *= fRadius;
			fRadius *= 200;
			m_SubActors[i]->SetX( SCREEN_CENTER_X + cosf(m_SubActors[i]->GetRotationZ())*fRadius );
			m_SubActors[i]->SetY( SCREEN_CENTER_Y + sinf(m_SubActors[i]->GetRotationZ())*fRadius );
		}
		break;
	case EFFECT_PARTICLES_SPIRAL_IN:
		for( unsigned i=0; i<m_SubActors.size(); i++ )
		{
			m_SubActors[i]->SetZoom( m_SubActors[i]->GetZoom() - fDeltaTime );
			if( m_SubActors[i]->GetZoom() < SPIRAL_MIN_ZOOM )
				m_SubActors[i]->SetZoom( SPIRAL_MAX_ZOOM );

			m_SubActors[i]->SetRotationZ( m_SubActors[i]->GetRotationZ() - fDeltaTime );

			float fRadius = (m_SubActors[i]->GetZoom()-SPIRAL_MIN_ZOOM);
			fRadius *= fRadius;
			fRadius *= 200;
			m_SubActors[i]->SetX( SCREEN_CENTER_X + cosf(m_SubActors[i]->GetRotationZ())*fRadius );
			m_SubActors[i]->SetY( SCREEN_CENTER_Y + sinf(m_SubActors[i]->GetRotationZ())*fRadius );
		}
		break;

	case TYPE_PARTICLES:
		for( unsigned i=0; i<m_SubActors.size(); i++ )
		{
			Actor* pActor = m_SubActors[i];
			RageVector3 &vel = m_vParticleVelocity[i];

			m_SubActors[i]->SetX( pActor->GetX() + fDeltaTime*vel.x );
			m_SubActors[i]->SetY( pActor->GetY() + fDeltaTime*vel.y );
			pActor->SetZ( pActor->GetZ() + fDeltaTime*vel.z  );
			if( m_bParticlesBounce )
			{
				if( HitGuardRailLeft(pActor) )	
				{
					vel.x *= -1;
					pActor->SetX( GetGuardRailLeft(pActor) );
				}
				if( HitGuardRailRight(pActor) )	
				{
					vel.x *= -1;
					pActor->SetX( GetGuardRailRight(pActor) );
				}
				if( HitGuardRailTop(pActor) )	
				{
					vel.y *= -1;
					pActor->SetY( GetGuardRailTop(pActor) );
				}
				if( HitGuardRailBottom(pActor) )	
				{
					vel.y *= -1;
					pActor->SetY( GetGuardRailBottom(pActor) );
				}
			}
			else // !m_bParticlesBounce 
			{
				if( vel.x<0  &&  IsOffScreenLeft(pActor) )
					pActor->SetX( GetOffScreenRight(pActor) );
				if( vel.x>0  &&  IsOffScreenRight(pActor) )
					pActor->SetX( GetOffScreenLeft(pActor) );
				if( vel.y<0  &&  IsOffScreenTop(pActor) )
					pActor->SetY( GetOffScreenBottom(pActor) );
				if( vel.y>0  &&  IsOffScreenBottom(pActor) )
					pActor->SetY( GetOffScreenTop(pActor) );
			}
		}
		break;
	case TYPE_TILES:
		{
			float fSecs = RageTimer::GetTimeSinceStart();
			float fTotalWidth = m_iNumTilesWide * m_fTilesSpacingX;
			float fTotalHeight = m_iNumTilesHigh * m_fTilesSpacingY;
			
			ASSERT( int(m_SubActors.size()) == m_iNumTilesWide * m_iNumTilesHigh );

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
					
					m_SubActors[i]->SetX( fX );
					m_SubActors[i]->SetY( fY );
				}
			}
		}
		break;
	case EFFECT_TILE_PULSE:
		for( unsigned i=0; i<m_SubActors.size(); i++ )
			m_SubActors[i]->SetZoom( sinf( fSongBeat*PI/2 ) );

		break;
	default:
		ASSERT(0);
	}

	if( m_fRepeatCommandEverySeconds != -1 )	// if repeating
	{
		m_fSecondsUntilNextCommand -= fDeltaTime;
		if( m_fSecondsUntilNextCommand <= 0 )
		{
			PlayCommand( "On" );
			m_fSecondsUntilNextCommand += m_fRepeatCommandEverySeconds;

			/* In case we delayed a long time, don't queue two repeats at once. */
			wrap( m_fSecondsUntilNextCommand, m_fRepeatCommandEverySeconds );
		}
	}
}

bool BGAnimationLayer::EarlyAbortDraw()
{
	if( m_sDrawCond.empty() )
		return false;

	if( !Lua::RunExpressionB( m_sDrawCond ) )
		return true;

	return false;
}

void BGAnimationLayer::DrawPrimitives()
{
	if( m_fFOV != -1 )
	{
		DISPLAY->CameraPushMatrix();
		DISPLAY->LoadMenuPerspective( m_fFOV, SCREEN_CENTER_X, SCREEN_CENTER_Y );
	}

	if( m_bLighting )
	{
		DISPLAY->SetLighting( true );
		DISPLAY->SetLightDirectional( 
			0, 
			RageColor(1,1,1,1), 
			RageColor(1,1,1,1),
			RageColor(1,1,1,1),
			RageVector3(0,0,1) );
	}

	ActorFrame::DrawPrimitives();
	
	if( m_fFOV != -1 )
	{
		DISPLAY->CameraPopMatrix();
	}

	if( m_bLighting )
	{
		DISPLAY->SetLightOff( 0 );
		DISPLAY->SetLighting( false );
	}
}

void BGAnimationLayer::GainFocus( float fRate, bool bRewindMovie, bool bLoop )
{
	m_fUpdateRate = fRate;

	if( !m_SubActors.size() )
		return;

	//
	// The order of these actions is important.
	// At this point, the movie is probably paused (by LoseFocus()).
	// Play the movie, then set the playback rate (which can 
	// potentially pause the movie again).
	//
	// TODO: Don't special case subActor[0].  The movie layer should be set up with
	// a LoseFocusCommand that pauses, and a GainFocusCommand that plays.
	if( bRewindMovie )
		RunCommandOnChildren( ParseCommands("position,0") );
	RunCommandOnChildren( ParseCommands(ssprintf("loop,%i",bLoop)) );
	RunCommandOnChildren( ParseCommands("play") );
	RunCommandOnChildren( ParseCommands(ssprintf("rate,%f",fRate)) );

	if( m_fRepeatCommandEverySeconds == -1 )	// if not repeating
	{
		/* Yuck.  We send OnCommand on load, since that's what's wanted for
		 * most backgrounds.  However, gameplay backgrounds (loaded from Background)
		 * should run OnCommand when they're actually displayed, when GainFocus
		 * gets called.  We've already run OnCommand; abort it so we don't run tweens
		 * twice. */
		RunCommandOnChildren( ParseCommands("stoptweening") );
		PlayCommand( "On" );
	}

	PlayCommand( "GainFocus" );

	ActorFrame::GainFocus( fRate, bRewindMovie, bLoop );
}

void BGAnimationLayer::LoseFocus()
{
	if( !m_SubActors.size() )
		return;

	RunCommandOnChildren( ParseCommands("pause") );

	PlayCommand( "LoseFocus" );
}

void BGAnimationLayer::PlayCommand( const CString &sCommandName )
{
	ActorFrame::PlayCommand( sCommandName );

#if 0
	for( unsigned i=0; i<m_SubActors.size(); i++ )
		m_SubActors[i]->RunCommands( ParseCommands("playcommand,"+sCommandName) );

	CString sKey = sCommandName;
	sKey.MakeLower();
	map<CString, Commands>::const_iterator it = m_mapNameToCommands.find( sKey );

	if( it == m_mapNameToCommands.end() )
		return;

	for( unsigned i=0; i<m_SubActors.size(); i++ )
		m_SubActors[i]->RunCommands( it->second );
#endif
}

/*
 * (c) 2001-2004 Ben Nordstrom, Chris Danford, Glenn Maynard
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
