#include "global.h"
#include "BGAnimationLayer.h"
#include "GameState.h"
#include "XmlFile.h"
#include "RageMath.h"
#include "RageLog.h"
#include "song.h"
#include "ScreenDimensions.h"
#include "Sprite.h"
#include "RageDisplay.h"
#include "ActorUtil.h"
#include "arch/ArchHooks/ArchHooks.h"
#include "LuaManager.h"
#include "AutoActor.h"
#include "ThemeManager.h"


const float PARTICLE_SPEED = 300;

const float SPIRAL_MAX_ZOOM = 2;
const float SPIRAL_MIN_ZOOM = 0.3f;

#define MAX_TILES_WIDE int(SCREEN_WIDTH/32+2)
#define MAX_TILES_HIGH int(SCREEN_HEIGHT/32+2)
#define MAX_SPRITES (MAX_TILES_WIDE*MAX_TILES_HIGH)

inline float GetOffScreenLeft(  Actor* pActor ) { return SCREEN_LEFT  - pActor->GetZoomedWidth()/2; }
inline float GetOffScreenRight( Actor* pActor ) { return SCREEN_RIGHT + pActor->GetZoomedWidth()/2; }
inline float GetOffScreenTop(   Actor* pActor ) { return SCREEN_TOP   - pActor->GetZoomedHeight()/2; }
inline float GetOffScreenBottom(Actor* pActor ) { return SCREEN_BOTTOM+ pActor->GetZoomedHeight()/2; }

inline bool IsOffScreenLeft(  Actor* pActor ) { return pActor->GetX() < GetOffScreenLeft(pActor); }
inline bool IsOffScreenRight( Actor* pActor ) { return pActor->GetX() > GetOffScreenRight(pActor); }
inline bool IsOffScreenTop(   Actor* pActor ) { return pActor->GetY() < GetOffScreenTop(pActor); }
inline bool IsOffScreenBottom(Actor* pActor ) { return pActor->GetY() > GetOffScreenBottom(pActor); }
inline bool IsOffScreen(      Actor* pActor ) { return IsOffScreenLeft(pActor) || IsOffScreenRight(pActor) || IsOffScreenTop(pActor) || IsOffScreenBottom(pActor); }

// guard rail is the area that keeps particles from going off screen
inline float GetGuardRailLeft(  Actor* pActor ) { return SCREEN_LEFT  + pActor->GetZoomedWidth()/2; }
inline float GetGuardRailRight( Actor* pActor ) { return SCREEN_RIGHT - pActor->GetZoomedWidth()/2; }
inline float GetGuardRailTop(   Actor* pActor ) { return SCREEN_TOP   + pActor->GetZoomedHeight()/2; }
inline float GetGuardRailBottom(Actor* pActor ) { return SCREEN_BOTTOM- pActor->GetZoomedHeight()/2; }

inline bool HitGuardRailLeft(  Actor* pActor ) { return pActor->GetX() < GetGuardRailLeft(pActor); }
inline bool HitGuardRailRight( Actor* pActor ) { return pActor->GetX() > GetGuardRailRight(pActor); }
inline bool HitGuardRailTop(   Actor* pActor ) { return pActor->GetY() < GetGuardRailTop(pActor); }
inline bool HitGuardRailBottom(Actor* pActor ) { return pActor->GetY() > GetGuardRailBottom(pActor); }

BGAnimationLayer::BGAnimationLayer()
{
	m_vParticleVelocity.clear();

	m_Type = TYPE_SPRITE;

	m_fTexCoordVelocityX = 0;
	m_fTexCoordVelocityY = 0;
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

BGAnimationLayer::~BGAnimationLayer()
{
	ActorFrame::DeleteAllChildren();
}

void BGAnimationLayer::LoadFromAniLayerFile( const RString& sPath )
{
	/* Generic BGAs are new.  Animation directories with no INI are old and obsolete. 
	 * Don't combine them. */
	RString lcPath = sPath;
	lcPath.MakeLower();

	if( lcPath.find("usesongbg") != RString::npos )
	{
		const Song* pSong = GAMESTATE->m_pCurSong;
		RString sSongBGPath;
		if( pSong && pSong->HasBackground() )
			sSongBGPath = pSong->GetBackgroundPath();
		else
			sSongBGPath = THEME->GetPathG("Common","fallback background");

		Sprite* pSprite = new Sprite;
		pSprite->Load( Sprite::SongBGTexture(sSongBGPath) );
		pSprite->StretchTo( FullScreenRectF );
		this->AddChild( pSprite );

		return;		// this will ignore other effects in the file name
	}

	enum Effect {
		EFFECT_CENTER,
		EFFECT_STRETCH_STILL,
		EFFECT_STRETCH_SCROLL_LEFT,
		EFFECT_STRETCH_SCROLL_RIGHT,
		EFFECT_STRETCH_SCROLL_UP,
		EFFECT_STRETCH_SCROLL_DOWN,
		EFFECT_STRETCH_WATER,
		EFFECT_STRETCH_BUBBLE,
		EFFECT_STRETCH_TWIST,
		EFFECT_STRETCH_SPIN,
		EFFECT_PARTICLES_SPIRAL_OUT,
		EFFECT_PARTICLES_SPIRAL_IN,
		EFFECT_PARTICLES_FLOAT_UP,
		EFFECT_PARTICLES_FLOAT_DOWN,
		EFFECT_PARTICLES_FLOAT_LEFT,
		EFFECT_PARTICLES_FLOAT_RIGHT,
		EFFECT_PARTICLES_BOUNCE,
		EFFECT_TILE_STILL,
		EFFECT_TILE_SCROLL_LEFT,
		EFFECT_TILE_SCROLL_RIGHT,
		EFFECT_TILE_SCROLL_UP,
		EFFECT_TILE_SCROLL_DOWN,
		EFFECT_TILE_FLIP_X,
		EFFECT_TILE_FLIP_Y,
		EFFECT_TILE_PULSE,
		NUM_EFFECTS,		// leave this at the end
		EFFECT_INVALID
	};

	const RString EFFECT_STRING[NUM_EFFECTS] = {
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
		if( lcPath.find(EFFECT_STRING[i]) != string::npos )
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
			pSprite->Load( Sprite::SongBGTexture(ID) );
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
			pSprite->Load( Sprite::SongBGTexture(sPath) );
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
			int iNumParticles = iMaxArea / iSpriteArea;
			iNumParticles = min( iNumParticles, MAX_SPRITES );

			for( int i=0; i<iNumParticles; i++ )
			{
				Sprite* pSprite = new Sprite;
				this->AddChild( pSprite );
				pSprite->Load( sPath );
				pSprite->SetZoom( 0.7f + 0.6f*i/(float)iNumParticles );
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


	RString sHint = sPath;
	sHint.MakeLower();

	if( sHint.find("cyclecolor") != RString::npos )
		for( unsigned i=0; i<m_SubActors.size(); i++ )
			m_SubActors[i]->SetEffectRainbow( 5 );

	if( sHint.find("cyclealpha") != RString::npos )
		for( unsigned i=0; i<m_SubActors.size(); i++ )
			m_SubActors[i]->SetEffectDiffuseShift( 2, RageColor(1,1,1,1), RageColor(1,1,1,0) );

	if( sHint.find("startonrandomframe") != RString::npos )
		for( unsigned i=0; i<m_SubActors.size(); i++ )
			m_SubActors[i]->SetState( RandomInt(m_SubActors[i]->GetNumStates()) );

	if( sHint.find("dontanimate") != RString::npos )
		for( unsigned i=0; i<m_SubActors.size(); i++ )
			m_SubActors[i]->StopAnimating();

	if( sHint.find("add") != RString::npos )
		for( unsigned i=0; i<m_SubActors.size(); i++ )
			m_SubActors[i]->SetBlendMode( BLEND_ADD );
}

void BGAnimationLayer::LoadFromNode( const XNode* pNode )
{
	{
		bool bCond;
		if( pNode->GetAttrValue("Condition", bCond) && !bCond )
			return;
	}

	bool bStretch = false;
	{
		RString type = "sprite";
		pNode->GetAttrValue( "Type", type );
		type.MakeLower();

		/* The preferred way of stretching a sprite to fit the screen is "Type=sprite"
		 * and "stretch=1".  "type=1" is for backwards-compatibility. */
		pNode->GetAttrValue( "Stretch", bStretch );

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

	pNode->GetAttrValue( "FOV", m_fFOV );
	pNode->GetAttrValue( "Lighting", m_bLighting );

	pNode->GetAttrValue( "TexCoordVelocityX", m_fTexCoordVelocityX );
	pNode->GetAttrValue( "TexCoordVelocityY", m_fTexCoordVelocityY );
	pNode->GetAttrValue( "DrawCond", m_sDrawCond );

	// compat:
	pNode->GetAttrValue( "StretchTexCoordVelocityX", m_fTexCoordVelocityX );
	pNode->GetAttrValue( "StretchTexCoordVelocityY", m_fTexCoordVelocityY );

	// particle and tile stuff
	float fZoomMin = 1;
	float fZoomMax = 1;
	pNode->GetAttrValue( "ZoomMin", fZoomMin );
	pNode->GetAttrValue( "ZoomMax", fZoomMax );

	float fVelocityXMin = 10, fVelocityXMax = 10;
	float fVelocityYMin = 0, fVelocityYMax = 0;
	float fVelocityZMin = 0, fVelocityZMax = 0;
	float fOverrideSpeed = 0;		// 0 means don't override speed
	pNode->GetAttrValue( "VelocityXMin", fVelocityXMin );
	pNode->GetAttrValue( "VelocityXMax", fVelocityXMax );
	pNode->GetAttrValue( "VelocityYMin", fVelocityYMin );
	pNode->GetAttrValue( "VelocityYMax", fVelocityYMax );
	pNode->GetAttrValue( "VelocityZMin", fVelocityZMin );
	pNode->GetAttrValue( "VelocityZMax", fVelocityZMax );
	pNode->GetAttrValue( "OverrideSpeed", fOverrideSpeed );

	int iNumParticles = 10;
	pNode->GetAttrValue( "NumParticles", iNumParticles );

	pNode->GetAttrValue( "ParticlesBounce", m_bParticlesBounce );
	pNode->GetAttrValue( "TilesStartX", m_fTilesStartX );
	pNode->GetAttrValue( "TilesStartY", m_fTilesStartY );
	pNode->GetAttrValue( "TilesSpacingX", m_fTilesSpacingX );
	pNode->GetAttrValue( "TilesSpacingY", m_fTilesSpacingY );
	pNode->GetAttrValue( "TileVelocityX", m_fTileVelocityX );
	pNode->GetAttrValue( "TileVelocityY", m_fTileVelocityY );


	switch( m_Type )
	{
	case TYPE_SPRITE:
		{
			Actor* pActor = ActorUtil::LoadFromNode( pNode );
			this->AddChild( pActor );
			if( bStretch )
				pActor->StretchTo( FullScreenRectF );
		}
		break;
	case TYPE_PARTICLES:
		{
			RString sFile;
			ActorUtil::GetAttrPath( pNode, "File", sFile );
			FixSlashesInPlace( sFile );
			
			CollapsePath( sFile );


			for( int i=0; i<iNumParticles; i++ )
			{
				Actor* pActor = ActorUtil::MakeActor( sFile );
				if( pActor == NULL )
					continue;
				this->AddChild( pActor );
				pActor->SetXY( randomf(float(FullScreenRectF.left),float(FullScreenRectF.right)),
							   randomf(float(FullScreenRectF.top),float(FullScreenRectF.bottom)) );
				pActor->SetZoom( randomf(fZoomMin,fZoomMax) );
				m_vParticleVelocity.push_back( RageVector3( 
					randomf(fVelocityXMin,fVelocityXMax),
					randomf(fVelocityYMin,fVelocityYMax),
					randomf(fVelocityZMin,fVelocityZMax) ) );
				if( fOverrideSpeed != 0 )
				{
					RageVec3Normalize( &m_vParticleVelocity[i], &m_vParticleVelocity[i] );
					m_vParticleVelocity[i] *= fOverrideSpeed;
				}
			}
		}
		break;
	case TYPE_TILES:
		{
			RString sFile;
			ActorUtil::GetAttrPath( pNode, "File", sFile );
			FixSlashesInPlace( sFile );
			
			CollapsePath( sFile );

			AutoActor s;
			s.Load( sFile );
			if( m_fTilesSpacingX == -1 )
				m_fTilesSpacingX = s->GetUnzoomedWidth();
			if( m_fTilesSpacingY == -1 )
				m_fTilesSpacingY = s->GetUnzoomedHeight();
			m_iNumTilesWide = 2+(int)(SCREEN_WIDTH /m_fTilesSpacingX);
			m_iNumTilesHigh = 2+(int)(SCREEN_HEIGHT/m_fTilesSpacingY);
			unsigned NumSprites = m_iNumTilesWide * m_iNumTilesHigh;
			for( unsigned i=0; i<NumSprites; i++ )
			{
				Actor* pSprite = ActorUtil::MakeActor( sFile );
				this->AddChild( pSprite );
				pSprite->SetTextureWrapping( true );		// gets rid of some "cracks"
				pSprite->SetZoom( randomf(fZoomMin,fZoomMax) );
			}
		}
		break;
	default:
		ASSERT(0);
	}

	bool bStartOnRandomFrame = false;
	pNode->GetAttrValue( "StartOnRandomFrame", bStartOnRandomFrame );
	if( bStartOnRandomFrame )
	{
		for( unsigned i=0; i<m_SubActors.size(); i++ )
			m_SubActors[i]->SetState( RandomInt(m_SubActors[i]->GetNumStates()) );
	}
}

void BGAnimationLayer::UpdateInternal( float fDeltaTime )
{
	ActorFrame::UpdateInternal( fDeltaTime );

	fDeltaTime *= m_fUpdateRate;

	switch( m_Type )
	{
	case TYPE_SPRITE:
		if( m_fTexCoordVelocityX || m_fTexCoordVelocityY )
		{
			for( unsigned i=0; i<m_SubActors.size(); i++ )
			{
				/* XXX: there's no longer any guarantee that this is a Sprite */
				Sprite *pSprite = (Sprite*)m_SubActors[i];
				pSprite->StretchTexCoords(
					fDeltaTime*m_fTexCoordVelocityX,
					fDeltaTime*m_fTexCoordVelocityY );
			}
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
			float fSecs = RageTimer::GetTimeSinceStartFast();
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
	default:
		ASSERT(0);
	}
}

bool BGAnimationLayer::EarlyAbortDraw() const
{
	if( m_sDrawCond.empty() )
		return false;

	// TODO: Is it ok to evaluate this every frame? 
	if( !LuaHelpers::RunExpressionB(m_sDrawCond) )
		return true;

	return false;
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
