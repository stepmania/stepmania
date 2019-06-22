#include "global.h"
#include <cassert>
#include <float.h>

#include "Sprite.h"
#include "RageTextureManager.h"
#include "XmlFile.h"
#include "RageLog.h"
#include "RageDisplay.h"
#include "RageTexture.h"
#include "RageTimer.h"
#include "RageUtil.h"
#include "ActorUtil.h"
#include "LuaBinding.h"
#include "LuaManager.h"
#include "ImageCache.h"
#include "ThemeMetric.h"
#include <numeric>

REGISTER_ACTOR_CLASS( Sprite );

const float min_state_delay= 0.0001f;

Sprite::Sprite()
{
	m_pTexture = nullptr;
	m_iCurState = 0;
	m_fSecsIntoState = 0.0f;
	m_animation_length_seconds= 0.0f;
	m_bUsingCustomTexCoords = false;
	m_bUsingCustomPosCoords = false;
	m_bSkipNextUpdate = true;
	m_DecodeMovie= true;
	m_EffectMode = EffectMode_Normal;
	
	m_fRememberedClipWidth = -1;
	m_fRememberedClipHeight = -1;

	m_fTexCoordVelocityX = 0;
	m_fTexCoordVelocityY = 0;
	m_use_effect_clock_for_texcoords= false;
}

// NoteSkinManager needs a sprite with a texture set to return in cases where
// the noteskin doesn't return a valid actor.  I would really prefer to make
// Sprite::Sprite load the default texture, but that causes problems for
// banners on ScreenSelectMusic and videos on ScreenGameplay.  So rather than
// dig through either of those, NoteSkinManager uses this special function.
Sprite* Sprite::NewBlankSprite()
{
	Sprite* news= new Sprite;
	news->Load(TEXTUREMAN->GetDefaultTextureID());
	return news;
}

Sprite::~Sprite()
{
	UnloadTexture();
}

Sprite::Sprite( const Sprite &cpy ):
	Actor( cpy )
{
#define CPY(a) a = cpy.a
	CPY( m_States );
	CPY(m_animation_length_seconds);
	CPY( m_iCurState );
	CPY( m_fSecsIntoState );
	CPY( m_bUsingCustomTexCoords );
	CPY( m_bUsingCustomPosCoords );
	CPY( m_bSkipNextUpdate );
	CPY( m_DecodeMovie );
	CPY( m_EffectMode );
	memcpy( m_CustomTexCoords, cpy.m_CustomTexCoords, sizeof(m_CustomTexCoords) );
	memcpy( m_CustomPosCoords, cpy.m_CustomPosCoords, sizeof(m_CustomPosCoords) );
	CPY( m_fRememberedClipWidth );
	CPY( m_fRememberedClipHeight );
	CPY( m_fTexCoordVelocityX );
	CPY( m_fTexCoordVelocityY );
	CPY(m_use_effect_clock_for_texcoords);
#undef CPY

	if( cpy.m_pTexture != nullptr )
		m_pTexture = TEXTUREMAN->CopyTexture( cpy.m_pTexture );
	else
		m_pTexture = nullptr;
}

Sprite &Sprite::operator=( Sprite other )
{
	using std::swap;
#define SWAP(a) swap(a, other.a)
	SWAP( m_States );
	SWAP(m_animation_length_seconds);
	SWAP( m_iCurState );
	SWAP( m_fSecsIntoState );
	SWAP( m_bUsingCustomTexCoords );
	SWAP( m_bUsingCustomPosCoords );
	SWAP( m_bSkipNextUpdate );
	SWAP( m_DecodeMovie );
	SWAP( m_EffectMode );
	memcpy( m_CustomTexCoords, other.m_CustomTexCoords, sizeof(m_CustomTexCoords) );
	memcpy( m_CustomPosCoords, other.m_CustomPosCoords, sizeof(m_CustomPosCoords) );
	SWAP( m_fRememberedClipWidth );
	SWAP( m_fRememberedClipHeight );
	SWAP( m_fTexCoordVelocityX );
	SWAP( m_fTexCoordVelocityY );
	SWAP(m_use_effect_clock_for_texcoords);
	SWAP(m_pTexture);
#undef SWAP
	return *this;
}

void Sprite::InitState()
{
	Actor::InitState();
	m_iCurState = 0;
	m_fSecsIntoState = 0.0f;
	m_bSkipNextUpdate = true;
}

void Sprite::SetAllStateDelays(float fDelay)
{
	for(unsigned int i=0;i<m_States.size();i++)
	{
		m_States[i].fDelay = fDelay;
	}
	RecalcAnimationLengthSeconds();
}

RageTextureID Sprite::SongBGTexture( RageTextureID ID )
{
	ID.bMipMaps = true;

	/* Song backgrounds are, by definition, in the background, so there's no need to keep alpha. */
	ID.iAlphaBits = 0;

	/* By default, song graphics are volatile: they're removed after one use.  This
	 * is because some screens iteratively load and display lots of them (eg. ScreenSelectMusic,,
	 * ScreenEditMenu) one at a time, and we don't want to have hundreds of banners loaded at once. */
	ID.Policy = RageTextureID::TEX_VOLATILE;

	ID.bDither = true;

	return ID;
}

RageTextureID Sprite::SongBannerTexture( RageTextureID ID )
{
	// Older song banners often have HOT PINK color keys.
	/* Use color keying only if the graphic is a diagonal banner.
	 * The color key convention is causing small holes in moderm banners that
	 * use magenta, and it's not good to require graphic makers to know about
	 * archaic color key conventions. -Chris */
	// I disabled this anyways, it's extremely annoying -Colby
	ID.bHotPinkColorKey = false;

	/* Ignore the texture color depth preference and always use 32-bit textures
	 * if possible. Banners are loaded while moving the wheel, so we want it to
	 * be as fast as possible. */
	ID.iColorDepth = 32;

	/* If we don't support RGBA8 (and will probably fall back on RGBA4), we're
	 * probably on something very old and slow, so let's opt for banding
	 * instead of slowing things down further by dithering. */
	// ID.bDither = true;

	ID.Policy = RageTextureID::TEX_VOLATILE;

	return ID;
}

void Sprite::Load( RageTextureID ID )
{
	if( !ID.filename.empty() ) 
		LoadFromTexture( ID );

	LoadStatesFromTexture();
};

void Sprite::LoadFromNode( const XNode* pNode )
{
	/* Texture may refer to the ID of a render target; if it's already
	 * registered, use it without trying to resolve it. */
	RString sPath;
	pNode->GetAttrValue( "Texture", sPath );
	if( !sPath.empty() && !TEXTUREMAN->IsTextureRegistered( RageTextureID(sPath) ) )
		ActorUtil::GetAttrPath( pNode, "Texture", sPath );

	if( !sPath.empty() )
	{
		// Load the texture
		LoadFromTexture( sPath );

		LoadStatesFromTexture();

		// Read in frames and delays from the sprite file, 
		// overwriting the states that LoadFromTexture created.
		// If the .sprite file doesn't define any states, leave
		// frames and delays created during LoadFromTexture().
		vector<State> aStates;

		const XNode *pFrames = pNode->GetChild( "Frames" );
		if( pFrames != nullptr )
		{
			/* All attributes are optional.  If Frame is omitted, use the previous state's
			 * frame (or 0 if the first).
			 * Frames = {
			 *  { Delay=1.0f; Frame=0; { x=0, y=0 }, { x=1, y=1 } };
			 * }
			 */
			int iFrameIndex = 0;
			for( int i=0; true; i++ )
			{
				const XNode *pFrame = pFrames->GetChild( ssprintf("%i", i+1) ); // +1 for Lua's arrays
				if( pFrame == nullptr )
					break;

				State newState;
				if( !pFrame->GetAttrValue("Delay", newState.fDelay) )
				{
					newState.fDelay = 0.1f;
				}

				pFrame->GetAttrValue( "Frame", iFrameIndex );
				if( iFrameIndex >= m_pTexture->GetNumFrames() )
				{
					LuaHelpers::ReportScriptErrorFmt( "%s: State #%i is frame %d, but the texture \"%s\" only has %d frames",
						ActorUtil::GetWhere(pNode).c_str(), i+1, iFrameIndex, sPath.c_str(), m_pTexture->GetNumFrames() );
				}
				newState.rect = *m_pTexture->GetTextureCoordRect( iFrameIndex );

				const XNode *pPoints[2] = { pFrame->GetChild( "1" ), pFrame->GetChild( "2" ) };
				if( pPoints[0] != nullptr && pPoints[1] != nullptr )
				{
					RectF r = newState.rect;

					float fX = 1.0f, fY = 1.0f;
					pPoints[0]->GetAttrValue( "x", fX );
					pPoints[0]->GetAttrValue( "y", fY );
					newState.rect.left = SCALE( fX, 0.0f, 1.0f, r.left, r.right );
					newState.rect.top = SCALE( fY, 0.0f, 1.0f, r.top, r.bottom );

					pPoints[1]->GetAttrValue( "x", fX );
					pPoints[1]->GetAttrValue( "y", fY );
					newState.rect.right = SCALE( fX, 0.0f, 1.0f, r.left, r.right );
					newState.rect.bottom = SCALE( fY, 0.0f, 1.0f, r.top, r.bottom );
				}

				aStates.push_back( newState );
			}
		}
		else for( int i=0; true; i++ )
		{
			// deprecated
			RString sFrameKey = ssprintf( "Frame%04d", i );
			RString sDelayKey = ssprintf( "Delay%04d", i );
			State newState;

			int iFrameIndex;
			if( !pNode->GetAttrValue(sFrameKey, iFrameIndex) )
				break;
			if( iFrameIndex >= m_pTexture->GetNumFrames() )
				LuaHelpers::ReportScriptErrorFmt( "%s: %s is %d, but the texture \"%s\" only has %d frames",
					ActorUtil::GetWhere(pNode).c_str(), sFrameKey.c_str(), iFrameIndex, sPath.c_str(), m_pTexture->GetNumFrames() );

			newState.rect = *m_pTexture->GetTextureCoordRect( iFrameIndex );

			if( !pNode->GetAttrValue(sDelayKey, newState.fDelay) )
				break;

			aStates.push_back( newState );
		}

		if( !aStates.empty() )
		{
			m_States = aStates;
			Sprite::m_size.x = aStates[0].rect.GetWidth() / m_pTexture->GetSourceToTexCoordsRatioX();
			Sprite::m_size.y = aStates[0].rect.GetHeight() / m_pTexture->GetSourceToTexCoordsRatioY();
		}
	}

	Actor::LoadFromNode( pNode );
	RecalcAnimationLengthSeconds();
}

void Sprite::UnloadTexture()
{
	if( m_pTexture != nullptr ) // If there was a previous bitmap...
	{
		TEXTUREMAN->UnloadTexture( m_pTexture ); // Unload it.
		m_pTexture = nullptr;

		/* Make sure we're reset to frame 0, so if we're reused, we aren't left
		 * on a frame number that may be greater than the number of frames in
		 * the newly loaded image. */
		SetState( 0 );
	}
}

void Sprite::EnableAnimation( bool bEnable )
{ 
	bool bWasEnabled = m_bIsAnimating;
	Actor::EnableAnimation( bEnable ); 

	if( bEnable && !bWasEnabled )
	{
		/* When we start animating a movie, we need to discard the first update; send
		 * 0 instead of the passed time. This is for two reasons:
		 *
		 * First, and most fundamentally, the time we'll receive on the next update
		 * represents time that passed *before* the movie was started. For example,
		 * 1: 20ms passes; 2: EnableAnimation(true); 3: Update(.020). We don't want
		 * to send that 20ms to the texture; on the first update, the movie's time
		 * should be 0.
		 *
		 * Second, we don't receive Update() calls if we're in a BGAnimation that
		 * doesn't have focus. It looks like 1: EnableAnimation(false); 2: 30 seconds
		 * pass; 3: EnableAnimation(true); 4: Update(30). We must be sure not to send
		 * that long 30-second update to the movie.
		 *
		 * (detail: the timestamps here are actually coming from GetEffectDeltaTime())
		 */
		m_bSkipNextUpdate = true;
	}
}

void Sprite::SetTexture( RageTexture *pTexture )
{
	ASSERT( pTexture != nullptr );

	if( m_pTexture != pTexture )
	{
		UnloadTexture();
		m_pTexture = pTexture;
	}

	ASSERT( m_pTexture->GetTextureWidth() >= 0 );
	ASSERT( m_pTexture->GetTextureHeight() >= 0 );

	// the size of the sprite is the size of the image before it was scaled
	Sprite::m_size.x = (float)m_pTexture->GetSourceFrameWidth();
	Sprite::m_size.y = (float)m_pTexture->GetSourceFrameHeight();

	// apply clipping (if any)
	if( m_fRememberedClipWidth != -1 && m_fRememberedClipHeight != -1 )
		ScaleToClipped( m_fRememberedClipWidth, m_fRememberedClipHeight );

	// Load default states if we haven't before.
	if( m_States.empty() )
		LoadStatesFromTexture();
}

void Sprite::LoadFromTexture( RageTextureID ID )
{
	// LOG->Trace( "Sprite::LoadFromTexture( %s )", ID.filename.c_str() );

	RageTexture *pTexture = nullptr;
	if( m_pTexture && m_pTexture->GetID() == ID )
		pTexture = m_pTexture;
	else
		pTexture = TEXTUREMAN->LoadTexture( ID );

	SetTexture( pTexture );
}

void Sprite::LoadFromCached( const RString &sDir, const RString &sPath )
{
	if( sPath.empty() )
	{
		Load( THEME->GetPathG("Common","fallback %s", sDir) );
		return;
	}

	RageTextureID ID;
	
	// Try to load the low quality version.
	ID = IMAGECACHE->LoadCachedImage( sDir, sPath );

	if( TEXTUREMAN->IsTextureRegistered(ID) )
		Load( ID );
	else if( IsAFile(sPath) )
		Load( sPath );
	else
		Load( THEME->GetPathG("Common","fallback %s", sDir) );
}

void Sprite::LoadStatesFromTexture()
{
	// Assume the frames of this animation play in sequential order with 0.1 second delay.
	m_States.clear();

	if( m_pTexture == nullptr )
	{
		State newState;
		newState.fDelay = 0.1f;
		newState.rect = RectF( 0, 0, 1, 1 );
		m_States.push_back( newState );
		return;
	}

	for( int i=0; i<m_pTexture->GetNumFrames(); ++i )
	{
		State newState;
		newState.fDelay = 0.1f;
		newState.rect = *m_pTexture->GetTextureCoordRect( i );
		m_States.push_back( newState );
	}
	RecalcAnimationLengthSeconds();
}

void Sprite::UpdateAnimationState()
{
	// Don't bother with state switching logic if there's only one state.  
	// We already know what's going to show.
	if( m_States.size() > 1 )
	{
		// UpdateAnimationState changed to not loop forever on negative state
		// delay.  This allows a state to last forever when it is reached, so
		// the animation has a built-in ending point. -Kyz
		while(m_States[m_iCurState].fDelay > min_state_delay &&
			m_fSecsIntoState+min_state_delay > m_States[m_iCurState].fDelay)
		// it's time to switch frames
		{
			// increment frame and reset the counter
			m_fSecsIntoState -= m_States[m_iCurState].fDelay;		// leave the left over time for the next frame
			m_iCurState = (m_iCurState+1) % m_States.size();
			if(m_iCurState == 0)
			{
				PlayCommand("AnimationFinished");
			}
		}
	}
}

/* We treat frame animation and movie animation slightly differently.
 *
 * Sprite animation is always tied directly to the effect timer. If you pause
 * animation, wait a while and restart it, sprite animations will snap back to
 * the effect timer. This allows sprites to animate to the beat in BGAnimations,
 * where they might be disabled for a while.
 *
 * Movies don't do this; if you pause a movie, wait a while and restart it,
 * it'll pick up where it left off. We may have a lot of movies loaded, so
 * it's too expensive to decode movies that aren't being displayed. Movies
 * also don't loop when the effect timer loops.
 *
 * (I'd like to handle sprite and movie animation as consistently as possible;
 * the above is just documentation of current practice.) -glenn */
// todo: see if "current" practice is just that. -aj
void Sprite::Update( float fDelta )
{
	Actor::Update( fDelta ); // do tweening

	const bool bSkipThisMovieUpdate = m_bSkipNextUpdate;
	m_bSkipNextUpdate = false;

	if( !m_bIsAnimating )
		return;

	if( !m_pTexture ) // no texture, nothing to animate
		return;

	float fTimePassed = GetEffectDeltaTime();
	m_fSecsIntoState += fTimePassed;

	if( m_fSecsIntoState < 0 )
		wrap( m_fSecsIntoState, GetAnimationLengthSeconds() );

	UpdateAnimationState();

	// If the texture is a movie, decode frames.
	if(!bSkipThisMovieUpdate && m_DecodeMovie)
		m_pTexture->DecodeSeconds( max(0, fTimePassed) );

	// update scrolling
	if( m_fTexCoordVelocityX != 0 || m_fTexCoordVelocityY != 0 )
	{
		float coord_delta= fDelta;
		if(m_use_effect_clock_for_texcoords)
		{
			coord_delta= fTimePassed;
		}
		float fTexCoords[8];
		Sprite::GetActiveTextureCoords( fTexCoords );
 
		// top left, bottom left, bottom right, top right
		fTexCoords[0] += coord_delta * m_fTexCoordVelocityX;
		fTexCoords[1] += coord_delta * m_fTexCoordVelocityY;
		fTexCoords[2] += coord_delta * m_fTexCoordVelocityX;
		fTexCoords[3] += coord_delta * m_fTexCoordVelocityY;
		fTexCoords[4] += coord_delta * m_fTexCoordVelocityX;
		fTexCoords[5] += coord_delta * m_fTexCoordVelocityY;
		fTexCoords[6] += coord_delta * m_fTexCoordVelocityX;
		fTexCoords[7] += coord_delta * m_fTexCoordVelocityY;

		/* When wrapping, avoid gradual loss of precision and sending
		 * unreasonably large texture coordinates to the renderer by pushing
		 * texture coordinates back to 0. As long as we adjust all four
		 * coordinates by the same amount, this won't be visible. */
		if( m_bTextureWrapping )
		{
			const float fXAdjust = floorf( fTexCoords[0] );
			const float fYAdjust = floorf( fTexCoords[1] );
			fTexCoords[0] -= fXAdjust;
			fTexCoords[2] -= fXAdjust;
			fTexCoords[4] -= fXAdjust;
			fTexCoords[6] -= fXAdjust;
			fTexCoords[1] -= fYAdjust;
			fTexCoords[3] -= fYAdjust;
			fTexCoords[5] -= fYAdjust;
			fTexCoords[7] -= fYAdjust;
		}

		Sprite::SetCustomTextureCoords( fTexCoords );
	}
}

void TexCoordArrayFromRect( float fImageCoords[8], const RectF &rect )
{
	fImageCoords[0] = rect.left;	fImageCoords[1] = rect.top;	// top left
	fImageCoords[2] = rect.left;	fImageCoords[3] = rect.bottom;	// bottom left
	fImageCoords[4] = rect.right;	fImageCoords[5] = rect.bottom;	// bottom right
	fImageCoords[6] = rect.right;	fImageCoords[7] = rect.top;	// top right
}

void Sprite::DrawTexture( const TweenState *state )
{
	Actor::SetGlobalRenderStates(); // set Actor-specified render states

	RectF crop = state->crop;
	// bail if cropped all the way 
	if( crop.left + crop.right >= 1  || 
		crop.top + crop.bottom >= 1 ) 
		return; 

	// use m_temp_* variables to draw the object
	RectF quadVerticies;
	quadVerticies.left   = -m_size.x/2.0f;
	quadVerticies.right  = +m_size.x/2.0f;
	quadVerticies.top    = -m_size.y/2.0f;
	quadVerticies.bottom = +m_size.y/2.0f;

	/* Don't draw anything outside of the texture's image area.  Texels outside 
	 * of the image area aren't guaranteed to be initialized. */
	/* HACK: Clamp the crop values. It would be more accurate to clip the 
	 * vertices so that the diffuse value is adjusted. */
	CLAMP( crop.left, 0, 1 );
	CLAMP( crop.right, 0, 1 );
	CLAMP( crop.top, 0, 1 );
	CLAMP( crop.bottom, 0, 1 );

	RectF croppedQuadVerticies = quadVerticies; 
#define IF_CROP_POS(side,opp_side) \
	if(state->crop.side!=0) \
		croppedQuadVerticies.side = \
			SCALE( crop.side, 0.f, 1.f, quadVerticies.side, quadVerticies.opp_side )
	IF_CROP_POS( left, right ); 
	IF_CROP_POS( top, bottom ); 
	IF_CROP_POS( right, left ); 
	IF_CROP_POS( bottom, top ); 

	static RageSpriteVertex v[4];
	v[0].p = RageVector3( croppedQuadVerticies.left,	croppedQuadVerticies.top,	0 );	// top left
	v[1].p = RageVector3( croppedQuadVerticies.left,	croppedQuadVerticies.bottom,	0 );	// bottom left
	v[2].p = RageVector3( croppedQuadVerticies.right,	croppedQuadVerticies.bottom,	0 );	// bottom right
	v[3].p = RageVector3( croppedQuadVerticies.right,	croppedQuadVerticies.top,	0 );	// top right
	if( m_bUsingCustomPosCoords )
	{
		for( int i=0; i < 4; ++i)
		{
			v[i].p.x+= m_CustomPosCoords[i*2];
			v[i].p.y+= m_CustomPosCoords[(i*2)+1];
		}
	}

	DISPLAY->ClearAllTextures();
	DISPLAY->SetTexture( TextureUnit_1, m_pTexture? m_pTexture->GetTexHandle():0 );

	// Must call this after setting the texture or else texture 
	// parameters have no effect.
	Actor::SetTextureRenderStates(); // set Actor-specified render states
	DISPLAY->SetEffectMode( m_EffectMode );

	if( m_pTexture )
	{
		float f[8];
		GetActiveTextureCoords( f );

		if( state->crop.left || state->crop.right || state->crop.top || state->crop.bottom )
		{
			RageVector2 texCoords[4] = {
				RageVector2( f[0], f[1] ),	// top left
				RageVector2( f[2], f[3] ),	// bottom left
				RageVector2( f[4], f[5] ),	// bottom right
				RageVector2( f[6], f[7] ) 	// top right
			};

			for( int i = 0; i < 4; ++i )
			{
				RageSpriteVertex *pVert = &v[i];

				float fTopX = SCALE( pVert->p.x, quadVerticies.left, quadVerticies.right, texCoords[0].x, texCoords[3].x );
				float fBottomX = SCALE( pVert->p.x, quadVerticies.left, quadVerticies.right, texCoords[1].x, texCoords[2].x );
				pVert->t.x = SCALE( pVert->p.y, quadVerticies.top, quadVerticies.bottom, fTopX, fBottomX );

				float fLeftY = SCALE( pVert->p.y, quadVerticies.top, quadVerticies.bottom, texCoords[0].y, texCoords[1].y );
				float fRightY = SCALE( pVert->p.y, quadVerticies.top, quadVerticies.bottom, texCoords[3].y, texCoords[2].y );
				pVert->t.y = SCALE( pVert->p.x, quadVerticies.left, quadVerticies.right, fLeftY, fRightY );
			}
		}
		else
		{
			v[0].t = RageVector2( f[0], f[1] );	// top left
			v[1].t = RageVector2( f[2], f[3] );	// bottom left
			v[2].t = RageVector2( f[4], f[5] );	// bottom right
			v[3].t = RageVector2( f[6], f[7] );	// top right
		}
	}
	else
	{
		// Just make sure we don't throw NaN/INF at the renderer:
		for( unsigned i = 0; i < 4; ++i )
			v[i].t.x = v[i].t.y = 0;
	}

	// Draw if we're not fully transparent
	if( state->diffuse[0].a > 0 || 
		state->diffuse[1].a > 0 ||
		state->diffuse[2].a > 0 ||
		state->diffuse[3].a > 0 )
	{
		DISPLAY->SetTextureMode( TextureUnit_1, TextureMode_Modulate );

		// render the shadow
		if( m_fShadowLengthX != 0  ||  m_fShadowLengthY != 0 )
		{
			DISPLAY->PushMatrix();
			DISPLAY->TranslateWorld( m_fShadowLengthX, m_fShadowLengthY, 0 );	// shift by 5 units
			RageColor c = m_ShadowColor;
			c.a *= state->diffuse[0].a;
			v[0].c = v[1].c = v[2].c = v[3].c = c;	// semi-transparent black
			DISPLAY->DrawQuad( v );
			DISPLAY->PopMatrix();
		}

		// render the diffuse pass
		v[0].c = state->diffuse[0]; // top left
		v[1].c = state->diffuse[2]; // bottom left
		v[2].c = state->diffuse[3]; // bottom right
		v[3].c = state->diffuse[1]; // top right
		DISPLAY->DrawQuad( v );
	}

	// render the glow pass
	if( state->glow.a > 0.0001f )
	{
		DISPLAY->SetTextureMode( TextureUnit_1, TextureMode_Glow );
		v[0].c = v[1].c = v[2].c = v[3].c = state->glow;
		DISPLAY->DrawQuad( v );
	}
	DISPLAY->SetEffectMode( EffectMode_Normal );
}

bool Sprite::EarlyAbortDraw() const
{
	return m_pTexture == nullptr;
}

void Sprite::DrawPrimitives()
{
	if( m_pTempState->fade.top > 0 ||
		m_pTempState->fade.bottom > 0 ||
		m_pTempState->fade.left > 0 ||
		m_pTempState->fade.right > 0 )
	{
		// We're fading the edges.
		const RectF &FadeDist = m_pTempState->fade;

		// Actual size of the fade on each side:
		RectF FadeSize = FadeDist;

		// If the cropped size is less than the fade distance in either dimension, clamp.
		const float HorizRemaining = 1.0f - (m_pTempState->crop.left + m_pTempState->crop.right);
		if( FadeDist.left+FadeDist.right > 0 &&
			HorizRemaining < FadeDist.left+FadeDist.right )
		{
			const float LeftPercent = FadeDist.left/(FadeDist.left+FadeDist.right);
			FadeSize.left = LeftPercent * HorizRemaining;
			FadeSize.right = (1.0f-LeftPercent) * HorizRemaining;
		}

		const float VertRemaining = 1.0f - (m_pTempState->crop.top + m_pTempState->crop.bottom);
		if( FadeDist.top+FadeDist.bottom > 0 &&
			VertRemaining < FadeDist.top+FadeDist.bottom )
		{
			const float TopPercent = FadeDist.top/(FadeDist.top+FadeDist.bottom);
			FadeSize.top = TopPercent * VertRemaining;
			FadeSize.bottom = (1.0f-TopPercent) * VertRemaining;
		}

		// Alpha value of the un-faded side of each fade rect:
		const float RightAlpha  = SCALE( FadeSize.right,  FadeDist.right,  0, 1, 0 );
		const float LeftAlpha   = SCALE( FadeSize.left,   FadeDist.left,   0, 1, 0 );
		const float TopAlpha    = SCALE( FadeSize.top,    FadeDist.top,    0, 1, 0 );
		const float BottomAlpha = SCALE( FadeSize.bottom, FadeDist.bottom, 0, 1, 0 );

		// Draw the inside:
		TweenState ts = *m_pTempState;
		ts.crop.left += FadeDist.left;
		ts.crop.right += FadeDist.right;
		ts.crop.top += FadeDist.top;
		ts.crop.bottom += FadeDist.bottom;
		DrawTexture( &ts );

		if( FadeSize.left > 0.001f )
		{
			// Draw the left:
			ts.crop = m_pTempState->crop; // restore
			ts.crop.right = 1 - (ts.crop.left + FadeSize.left);
			ts.crop.top += FadeDist.top;		// lop off the corner if fading both x and y
			ts.crop.bottom += FadeDist.bottom;

			ts.diffuse[0] = m_pTempState->diffuse[0];	// top left -> top left
			ts.diffuse[2] = m_pTempState->diffuse[2];	// bottom left -> bottom left
			ts.diffuse[3] = m_pTempState->diffuse[2];	// bottom left -> bottom right
			ts.diffuse[1] = m_pTempState->diffuse[0];	// top left -> top right
			ts.diffuse[0].a = 0;				// top left
			ts.diffuse[2].a = 0;				// bottom left
			ts.diffuse[3].a *= LeftAlpha;			// bottom right
			ts.diffuse[1].a *= LeftAlpha;			// top right
			DrawTexture( &ts );
		}

		if( FadeSize.right > 0.001f )
		{
			// Draw the right:
			ts.crop = m_pTempState->crop; // restore
			ts.crop.left = 1 - (ts.crop.right + FadeSize.right);
			ts.crop.top += FadeDist.top;
			ts.crop.bottom += FadeDist.bottom;

			ts.diffuse[0] = m_pTempState->diffuse[1];	// top right -> top left
			ts.diffuse[2] = m_pTempState->diffuse[3];	// bottom right -> bottom left
			ts.diffuse[3] = m_pTempState->diffuse[3];	// bottom right -> bottom right
			ts.diffuse[1] = m_pTempState->diffuse[1];	// top right -> top right
			ts.diffuse[0].a *= RightAlpha;			// top left
			ts.diffuse[2].a *= RightAlpha;			// bottom left
			ts.diffuse[3].a = 0;				// bottom right
			ts.diffuse[1].a = 0;				// top right

			DrawTexture( &ts );
		}

		if( FadeSize.top > 0.001f )
		{
			// Draw the top:
			ts.crop = m_pTempState->crop; // restore
			ts.crop.bottom = 1 - (ts.crop.top + FadeSize.top);
			ts.crop.left += FadeDist.left;
			ts.crop.right += FadeDist.right;

			ts.diffuse[0] = m_pTempState->diffuse[0];	// top left -> top left
			ts.diffuse[2] = m_pTempState->diffuse[0];	// top left -> bottom left
			ts.diffuse[3] = m_pTempState->diffuse[1];	// top right -> bottom right
			ts.diffuse[1] = m_pTempState->diffuse[1];	// top right -> top right
			ts.diffuse[0].a = 0;				// top left
			ts.diffuse[2].a *= TopAlpha;			// bottom left
			ts.diffuse[3].a *= TopAlpha;			// bottom right
			ts.diffuse[1].a = 0;				// top right

			DrawTexture( &ts );
		}

		if( FadeSize.bottom > 0.001f )
		{
			// Draw the bottom:
			ts.crop = m_pTempState->crop; // restore
			ts.crop.top = 1 - (ts.crop.bottom + FadeSize.bottom);
			ts.crop.left += FadeDist.left;
			ts.crop.right += FadeDist.right;

			ts.diffuse[0] = m_pTempState->diffuse[2];	// bottom left -> top left
			ts.diffuse[2] = m_pTempState->diffuse[2];	// bottom left -> bottom left
			ts.diffuse[3] = m_pTempState->diffuse[3];	// bottom right -> bottom right
			ts.diffuse[1] = m_pTempState->diffuse[3];	// bottom right -> top right
			ts.diffuse[0].a *= BottomAlpha;			// top left
			ts.diffuse[2].a = 0;				// bottom left
			ts.diffuse[3].a = 0;				// bottom right
			ts.diffuse[1].a *= BottomAlpha;			// top right
			DrawTexture( &ts );
		}
	}
	else
	{
		DrawTexture( m_pTempState );
	}
}


int Sprite::GetNumStates() const
{
	return m_States.size(); 
}

void Sprite::SetState( int iNewState )
{
	/*
	 * This assert will likely trigger if the "missing" theme element graphic 
	 * is loaded in place of a multi-frame sprite. We want to know about these
	 * problems in debug builds, but they're not fatal.
	 */

	// Never warn about setting state 0.
	if( iNewState != 0 && (iNewState < 0  ||  iNewState >= (int)m_States.size()) )
	{
		// Don't warn about number of states in "_blank" or "_missing".
		if( !m_pTexture || (m_pTexture->GetID().filename.find("_blank") == string::npos &&
			m_pTexture->GetID().filename.find("_missing") == string::npos) )
		{
			RString sError;
			if( m_pTexture )
				sError = ssprintf("A Sprite '%s' (\"%s\") tried to set state to frame %d, but it has only %u frames.",
					/*
					 * Using the state directly tends to give you an error message like "tried to set frame 6 of 6"
					 * which is very confusing if you don't know that one is 0-indexed and the other is 1-indexed.
					 * - Colby
					 */
					m_pTexture->GetID().filename.c_str(), this->m_sName.c_str(), iNewState+1, unsigned(m_States.size()));
			else
				sError = ssprintf("A Sprite (\"%s\") tried to set state index %d, but no texture is loaded.", 
					this->m_sName.c_str(), iNewState );
			LuaHelpers::ReportScriptError(sError, "SPRITE_INVALID_FRAME");
		}
	}

	CLAMP(iNewState, 0, (int)m_States.size()-1);
	m_iCurState = iNewState;
	m_fSecsIntoState = 0.0f;
}

void Sprite::RecalcAnimationLengthSeconds()
{
	m_animation_length_seconds = 0;
	for (State const &s : m_States)
	{
		m_animation_length_seconds += s.fDelay;
	}
}

void Sprite::SetSecondsIntoAnimation( float fSeconds )
{
	SetState( 0 );	// rewind to the first state
	if( m_pTexture )
		m_pTexture->SetPosition( fSeconds );
	m_fSecsIntoState = fSeconds;
	UpdateAnimationState();
}

RString	Sprite::GetTexturePath() const
{
	if( m_pTexture == nullptr )
		return RString();

	return m_pTexture->GetID().filename;
}

void Sprite::SetCustomTextureRect( const RectF &new_texcoord_frect ) 
{ 
	m_bUsingCustomTexCoords = true;
	m_bTextureWrapping = true;
	TexCoordArrayFromRect( m_CustomTexCoords, new_texcoord_frect );
}

void Sprite::SetCustomTextureCoords( float fTexCoords[8] ) // order: top left, bottom left, bottom right, top right
{ 
	m_bUsingCustomTexCoords = true;
	m_bTextureWrapping = true;
	for( int i=0; i<8; i++ )
		m_CustomTexCoords[i] = fTexCoords[i]; 
}

void Sprite::SetCustomImageRect( RectF rectImageCoords )
{
	// Convert to a rectangle in texture coordinate space.
	rectImageCoords.left	*= m_pTexture->GetImageWidth()	/ (float)m_pTexture->GetTextureWidth();
	rectImageCoords.right	*= m_pTexture->GetImageWidth()	/ (float)m_pTexture->GetTextureWidth();
	rectImageCoords.top	*= m_pTexture->GetImageHeight()	/ (float)m_pTexture->GetTextureHeight(); 
	rectImageCoords.bottom	*= m_pTexture->GetImageHeight()	/ (float)m_pTexture->GetTextureHeight(); 

	SetCustomTextureRect( rectImageCoords );
}

void Sprite::SetCustomImageCoords( float fImageCoords[8] )	// order: top left, bottom left, bottom right, top right
{
	// convert image coords to texture coords in place
	for( int i=0; i<8; i+=2 )
	{
		fImageCoords[i+0] *= m_pTexture->GetImageWidth()	/ (float)m_pTexture->GetTextureWidth(); 
		fImageCoords[i+1] *= m_pTexture->GetImageHeight()	/ (float)m_pTexture->GetTextureHeight(); 
	}

	SetCustomTextureCoords( fImageCoords );
}

void Sprite::SetCustomPosCoords( float fPosCoords[8] )	// order: top left, bottom left, bottom right, top right
{
	m_bUsingCustomPosCoords= true;
	for( int i=0; i<8; ++i )
	{
		m_CustomPosCoords[i]= fPosCoords[i];
	}
}

const RectF *Sprite::GetCurrentTextureCoordRect() const
{
	return GetTextureCoordRectForState( m_iCurState );
}

const RectF *Sprite::GetTextureCoordRectForState( int iState ) const
{
	ASSERT_M( iState < (int) m_States.size(), ssprintf("%d, %d", int(iState), int(m_States.size())) );

	return &m_States[iState].rect;
}

/* If we're using custom coordinates, return them; otherwise return the
 * coordinates for the current state. */
void Sprite::GetActiveTextureCoords( float fTexCoordsOut[8] ) const
{
	if(m_bUsingCustomTexCoords) 
	{
		// GetCustomTextureCoords
		for( int i=0; i<8; i++ )
			fTexCoordsOut[i] = m_CustomTexCoords[i]; 
	}
	else
	{
		// GetCurrentTextureCoords
		const RectF *pTexCoordRect = GetCurrentTextureCoordRect();
		TexCoordArrayFromRect( fTexCoordsOut, *pTexCoordRect );
	}
}


void Sprite::StopUsingCustomCoords()
{
	m_bUsingCustomTexCoords = false;
}

void Sprite::StopUsingCustomPosCoords()
{
	m_bUsingCustomPosCoords = false;
}

void Sprite::SetTexCoordVelocity(float fVelX, float fVelY)
{
	m_fTexCoordVelocityX = fVelX;
	m_fTexCoordVelocityY = fVelY;
}

void Sprite::ScaleToClipped( float fWidth, float fHeight )
{
	m_fRememberedClipWidth = fWidth;
	m_fRememberedClipHeight = fHeight;

	if( !m_pTexture )
		return;

	float fScaleFudgePercent = 0.15f;	// scale up to this amount in one dimension to avoid clipping.

	// save the original X and Y.  We're going to restore them later.
	float fOriginalX = GetX();
	float fOriginalY = GetY();

	if( fWidth != -1 && fHeight != -1 )
	{
		// this is probably a background graphic or something not intended to be a CroppedSprite
		Sprite::StopUsingCustomCoords();

		// first find the correct zoom
		Sprite::ScaleToCover( RectF(0, 0, fWidth, fHeight) );
		// find which dimension is larger
		bool bXDimNeedsToBeCropped = GetZoomedWidth() > fWidth+0.01;

		if( bXDimNeedsToBeCropped ) // crop X
		{
			float fPercentageToCutOff = (this->GetZoomedWidth() - fWidth) / this->GetZoomedWidth();
			fPercentageToCutOff = max( fPercentageToCutOff-fScaleFudgePercent, 0 );
			float fPercentageToCutOffEachSide = fPercentageToCutOff / 2;

			// generate a rectangle with new texture coordinates
			RectF fCustomImageRect( 
				fPercentageToCutOffEachSide,
				0,
				1 - fPercentageToCutOffEachSide,
				1 );
			SetCustomImageRect( fCustomImageRect );
		}
		else // crop Y
		{
			float fPercentageToCutOff = (this->GetZoomedHeight() - fHeight) / this->GetZoomedHeight();
			fPercentageToCutOff = max( fPercentageToCutOff-fScaleFudgePercent, 0 );
			float fPercentageToCutOffEachSide = fPercentageToCutOff / 2;

			// generate a rectangle with new texture coordinates
			RectF fCustomImageRect( 
				0,
				fPercentageToCutOffEachSide,
				1,
				1 - fPercentageToCutOffEachSide );
			SetCustomImageRect( fCustomImageRect );
		}
		m_size = RageVector2( fWidth, fHeight );
		SetZoom( 1 );
	}

	// restore original XY
	SetXY( fOriginalX, fOriginalY );
}


// magic hurr
// This code should either be removed or refactored in the future -aj
void Sprite::CropTo( float fWidth, float fHeight )
{
	m_fRememberedClipWidth = fWidth;
	m_fRememberedClipHeight = fHeight;

	if( !m_pTexture )
		return;

	// save the original X&Y.  We're going to restore them later.
	float fOriginalX = GetX();
	float fOriginalY = GetY();

	if( fWidth != -1 && fHeight != -1 )
	{
		// this is probably a background graphic or something not intended to be a CroppedSprite
		Sprite::StopUsingCustomCoords();

		// first find the correct zoom
		Sprite::ScaleToCover( RectF(0, 0, fWidth, fHeight) );
		// find which dimension is larger
		bool bXDimNeedsToBeCropped = GetZoomedWidth() > fWidth+0.01;

		if( bXDimNeedsToBeCropped )	// crop X
		{
			float fPercentageToCutOff = (this->GetZoomedWidth() - fWidth) / this->GetZoomedWidth();
			float fPercentageToCutOffEachSide = fPercentageToCutOff / 2;

			// generate a rectangle with new texture coordinates
			RectF fCustomImageRect( 
				fPercentageToCutOffEachSide,
				0,
				1 - fPercentageToCutOffEachSide,
				1 );
			SetCustomImageRect( fCustomImageRect );
		}
		else		// crop Y
		{
			float fPercentageToCutOff = (this->GetZoomedHeight() - fHeight) / this->GetZoomedHeight();
			float fPercentageToCutOffEachSide = fPercentageToCutOff / 2;

			// generate a rectangle with new texture coordinates
			RectF fCustomImageRect( 
				0,
				fPercentageToCutOffEachSide,
				1,
				1 - fPercentageToCutOffEachSide );
			SetCustomImageRect( fCustomImageRect );
		}
		m_size = RageVector2( fWidth, fHeight );
		SetZoom( 1 );
	}

	// restore original XY
	SetXY( fOriginalX, fOriginalY );
}
// end magic

void Sprite::StretchTexCoords( float fX, float fY )
{
	float fTexCoords[8];
	GetActiveTextureCoords( fTexCoords );

	for( int j=0; j<8; j+=2 )
	{
		fTexCoords[j  ] += fX;
		fTexCoords[j+1] += fY;
	}

	SetCustomTextureCoords( fTexCoords );
}

void Sprite::AddImageCoords( float fX, float fY )
{
	float fTexCoords[8];
	GetActiveTextureCoords( fTexCoords );

	for( int j=0; j<8; j+=2 )
	{
		fTexCoords[j  ] += fX / (float)m_pTexture->GetTextureWidth();
		fTexCoords[j+1] += fY / (float)m_pTexture->GetTextureHeight();
	}

	SetCustomTextureCoords( fTexCoords );
}


// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the Sprite. */ 
class LunaSprite: public Luna<Sprite>
{
public:
	static int Load( T* p, lua_State *L )
	{
		if( lua_isnil(L, 1) )
		{
			p->UnloadTexture();
		}
		else
		{
			RageTextureID ID( SArg(1) );
			if(lua_isstring(L, 2))
			{
				RString additional_hints= SArg(2);
				ID.AdditionalTextureHints= additional_hints;
			}
			p->Load( ID );
		}
		COMMON_RETURN_SELF;
	}
	static int LoadBackground( T* p, lua_State *L )
	{
		RageTextureID ID( SArg(1) );
		TEXTUREMAN->DisableOddDimensionWarning();
		p->Load( Sprite::SongBGTexture(ID) );
		TEXTUREMAN->EnableOddDimensionWarning();
		return 1;
	}
	static int LoadBanner( T* p, lua_State *L )
	{
		RageTextureID ID( SArg(1) );
		TEXTUREMAN->DisableOddDimensionWarning();
		p->Load( Sprite::SongBannerTexture(ID) );
		TEXTUREMAN->EnableOddDimensionWarning();
		return 1;
	}

	/* Commands that go in the tweening queue: 
	 * Commands that take effect immediately (ignoring the tweening queue): */
	static int customtexturerect( T* p, lua_State *L )	{ p->SetCustomTextureRect( RectF(FArg(1),FArg(2),FArg(3),FArg(4)) ); COMMON_RETURN_SELF; }
	static int SetCustomImageRect( T* p, lua_State *L )	{ p->SetCustomImageRect( RectF(FArg(1),FArg(2),FArg(3),FArg(4)) ); COMMON_RETURN_SELF; }
	static int SetCustomPosCoords( T* p, lua_State *L )
	{
		float coords[8];
		for( int i=0; i<8; ++i )
		{
			coords[i]= FArg(i+1);
			if( isnan(coords[i]) )
			{
				coords[i]= 0.0f;
			}
		}
		p->SetCustomPosCoords(coords);
		COMMON_RETURN_SELF;
	}
	static int StopUsingCustomPosCoords( T* p, lua_State *L ) { p->StopUsingCustomPosCoords(); COMMON_RETURN_SELF; }
	static int texcoordvelocity( T* p, lua_State *L )	{ p->SetTexCoordVelocity( FArg(1),FArg(2) ); COMMON_RETURN_SELF; }
	static int get_use_effect_clock_for_texcoords(T* p, lua_State* L)
	{
		lua_pushboolean(L, p->m_use_effect_clock_for_texcoords);
		return 1;
	}
	static int set_use_effect_clock_for_texcoords(T* p, lua_State* L)
	{
		p->m_use_effect_clock_for_texcoords= BArg(1);
		COMMON_RETURN_SELF;
	}
	static int scaletoclipped( T* p, lua_State *L )		{ p->ScaleToClipped( FArg(1),FArg(2) ); COMMON_RETURN_SELF; }
	static int CropTo( T* p, lua_State *L )		{ p->CropTo( FArg(1),FArg(2) ); COMMON_RETURN_SELF; }
	static int stretchtexcoords( T* p, lua_State *L )	{ p->StretchTexCoords( FArg(1),FArg(2) ); COMMON_RETURN_SELF; }
	static int addimagecoords( T* p, lua_State *L )		{ p->AddImageCoords( FArg(1),FArg(2) ); COMMON_RETURN_SELF; }
	static int setstate( T* p, lua_State *L )		{ p->SetState( IArg(1) ); COMMON_RETURN_SELF; }
	static int GetState( T* p, lua_State *L )		{ lua_pushnumber( L, p->GetState() ); return 1; }
	static int SetStateProperties(T* p, lua_State* L)
	{
		// States table example:
		// {{Frame= 0, Delay= .016, {0, 0}, {.25, .25}}}
		// Each element in the States table must have a Frame and a Delay.
		// Frame is optional, defaulting to 0.
		// Delay is optional, defaulting to 0.
		// The two tables in the state are the upper left and lower right and are
		// optional.
		if(!lua_istable(L, 1))
		{
			luaL_error(L, "State properties must be in a table.");
		}
		vector<Sprite::State> new_states;
		size_t num_states= lua_objlen(L, 1);
		if(num_states == 0)
		{
			luaL_error(L, "A Sprite cannot have zero states.");
		}
		for(size_t s= 0; s < num_states; ++s)
		{
			Sprite::State new_state;
			lua_rawgeti(L, 1, s+1);
			lua_getfield(L, -1, "Frame");
			int frame_index= 0;
			if(lua_isnumber(L, -1))
			{
				frame_index= IArg(-1);
				if(frame_index < 0 || frame_index >= p->GetTexture()->GetNumFrames())
				{
					luaL_error(L, "Frame index out of range 0-%d.",
						p->GetTexture()->GetNumFrames()-1);
				}
			}
			new_state.rect= *p->GetTexture()->GetTextureCoordRect(frame_index);
			lua_pop(L, 1);
			lua_getfield(L, -1, "Delay");
			if(lua_isnumber(L, -1))
			{
				new_state.fDelay= FArg(-1);
			}
			lua_pop(L, 1);
			RectF r= new_state.rect;
			lua_rawgeti(L, -1, 1);
			if(lua_istable(L, -1))
			{
				lua_rawgeti(L, -1, 1);
				// I have no idea why the points are from 0 to 1 and make it use only
				// a portion of the state.  This is just copied from LoadFromNode.
				// -Kyz
				new_state.rect.left= SCALE(FArg(-1), 0.0f, 1.0f, r.left, r.right);
				lua_pop(L, 1);
				lua_rawgeti(L, -1, 2);
				new_state.rect.top= SCALE(FArg(-1), 0.0f, 1.0f, r.top, r.bottom);
				lua_pop(L, 1);
			}
			lua_pop(L, 1);
			lua_rawgeti(L, -1, 2);
			if(lua_istable(L, -1))
			{
				lua_rawgeti(L, -1, 1);
				// I have no idea why the points are from 0 to 1 and make it use only
				// a portion of the state.  This is just copied from LoadFromNode.
				// -Kyz
				new_state.rect.right= SCALE(FArg(-1), 0.0f, 1.0f, r.left, r.right);
				lua_pop(L, 1);
				lua_rawgeti(L, -1, 2);
				new_state.rect.bottom= SCALE(FArg(-1), 0.0f, 1.0f, r.top, r.bottom);
				lua_pop(L, 1);
			}
			lua_pop(L, 1);
			new_states.push_back(new_state);
			lua_pop(L, 1);
		}
		p->SetStateProperties(new_states);
		COMMON_RETURN_SELF;
	}
	static int GetAnimationLengthSeconds( T* p, lua_State *L ) { lua_pushnumber( L, p->GetAnimationLengthSeconds() ); return 1; }
	static int SetSecondsIntoAnimation( T* p, lua_State *L )	{ p->SetSecondsIntoAnimation(FArg(0)); COMMON_RETURN_SELF; }
	static int SetTexture( T* p, lua_State *L )
	{
		RageTexture *pTexture = Luna<RageTexture>::check(L, 1);
		pTexture = TEXTUREMAN->CopyTexture( pTexture );
		p->SetTexture( pTexture );
		COMMON_RETURN_SELF;
	}
	static int GetTexture( T* p, lua_State *L )
	{
		RageTexture *pTexture = p->GetTexture();
		if( pTexture != nullptr )
			pTexture->PushSelf(L);
		else
			lua_pushnil( L );
		return 1;
	}
	static int SetEffectMode( T* p, lua_State *L )
	{
		EffectMode em = Enum::Check<EffectMode>(L, 1);
		p->SetEffectMode( em );
		COMMON_RETURN_SELF;
	}
	static int GetNumStates( T* p, lua_State *L ) { lua_pushnumber( L, p->GetNumStates() ); return 1; }
	static int SetAllStateDelays( T* p, lua_State *L )
	{
		p->SetAllStateDelays(FArg(-1));
		COMMON_RETURN_SELF;
	}
	DEFINE_METHOD(GetDecodeMovie, m_DecodeMovie);
	static int SetDecodeMovie(T* p, lua_State *L)
	{
		p->m_DecodeMovie= BArg(1);
		COMMON_RETURN_SELF;
	}
	static int LoadFromCached( T* p, lua_State *L )
	{ 
		p->LoadFromCached( SArg(1), SArg(2) );
		COMMON_RETURN_SELF;
	}

	LunaSprite()
	{
		ADD_METHOD( Load );
		ADD_METHOD( LoadBanner );
		ADD_METHOD( LoadBackground );
		ADD_METHOD( LoadFromCached );
		ADD_METHOD( customtexturerect );
		ADD_METHOD( SetCustomImageRect );
		ADD_METHOD( SetCustomPosCoords );
		ADD_METHOD( StopUsingCustomPosCoords );
		ADD_METHOD( texcoordvelocity );
		ADD_METHOD(get_use_effect_clock_for_texcoords);
		ADD_METHOD(set_use_effect_clock_for_texcoords);
		ADD_METHOD( scaletoclipped );
		ADD_METHOD( CropTo );
		ADD_METHOD( stretchtexcoords );
		ADD_METHOD( addimagecoords );
		ADD_METHOD( setstate );
		ADD_METHOD( GetState );
		ADD_METHOD( SetStateProperties );
		ADD_METHOD( GetAnimationLengthSeconds );
		ADD_METHOD( SetSecondsIntoAnimation );
		ADD_METHOD( SetTexture );
		ADD_METHOD( GetTexture );
		ADD_METHOD( SetEffectMode );
		ADD_METHOD( GetNumStates );
		ADD_METHOD( SetAllStateDelays );
		ADD_METHOD(GetDecodeMovie);
		ADD_METHOD(SetDecodeMovie);
	}
};

LUA_REGISTER_DERIVED_CLASS( Sprite, Actor )
// lua end

/*
 * (c) 2001-2004 Chris Danford
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
