#include "global.h"
#include <cassert>

#include "Sprite.h"
#include "RageTextureManager.h"
#include "IniFile.h"
#include "RageLog.h"
#include "RageException.h"
#include "PrefsManager.h"
#include "RageDisplay.h"
#include "RageTexture.h"
#include "ActorUtil.h"
#include "arch/Dialog/Dialog.h"
#include "Foreach.h"

Sprite::Sprite()
{
	m_pTexture = NULL;
	m_bDrawIfTextureNull = false;
	m_iCurState = 0;
	m_fSecsIntoState = 0.0;
	m_bUsingCustomTexCoords = false;
	
	m_fRememberedClipWidth = -1;
	m_fRememberedClipHeight = -1;

	m_fTexCoordVelocityX = 0;
	m_fTexCoordVelocityY = 0;
}


Sprite::~Sprite()
{
	UnloadTexture();
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
	/* Song banners often have HOT PINK color keys. */
	ID.bHotPinkColorKey = true;

	/* Ignore the texture color depth preference and always use 32-bit textures
	 * if possible.  Banners are loaded while moving the wheel, so we want it to
	 * be as fast as possible. */
	ID.iColorDepth = 32;

	/* If we don't support RGBA8 (and will probably fall back on RGBA4), we're probably
	 * on something very old and slow, so let's opt for banding instead of slowing things
	 * down further by dithering. */
	// ID.bDither = true;

	ID.Policy = RageTextureID::TEX_VOLATILE;

	return ID;
}

/* deprecated */
bool Sprite::LoadBG( RageTextureID ID )
{
	return Load( SongBGTexture(ID) );
}

bool Sprite::Load( RageTextureID ID )
{
	bool result;
	if( ID.filename == "" ) result = true;
	if( ID.filename.Right(7) == ".sprite" )
		result = LoadFromSpriteFile( ID );
	else 
		result = LoadFromTexture( ID );

	return result;
};

// Sprite file has the format:
//
// [Sprite]
// Texture=Textures\Logo.bmp
// Frame0000=0
// Delay0000=1.0
// Frame0001=3
// Delay0000=2.0
// BaseRotationXDegrees=0
// BaseRotationYDegrees=0
// BaseRotationZDegrees=0
// BaseZoomX=1
// BaseZoomY=1
// BaseZoomZ=1
bool Sprite::LoadFromSpriteFile( RageTextureID ID )
{
	LOG->Trace( ssprintf("Sprite::LoadFromSpriteFile(%s)", ID.filename.c_str()) );

retry:

	//Init();

	m_sSpritePath = ID.filename;

	// read sprite file
	IniFile ini;
	if( !ini.ReadFile( m_sSpritePath ) )
		RageException::Throw( "Error opening Sprite file '%s'.", m_sSpritePath.c_str() );

	CString sTextureFile;
	ini.GetValue( "Sprite", "Texture", sTextureFile );
	if( sTextureFile == ""  )
		RageException::Throw( "Error reading value 'Texture' from %s.", m_sSpritePath.c_str() );

	// save the path of the real texture
	ID.filename = Dirname(m_sSpritePath) + sTextureFile;
	{
		vector<CString> asElementPaths;
		GetDirListing( ID.filename + "*", asElementPaths, false, true );
		if(asElementPaths.size() == 0)
		{
			CString sMessage = ssprintf( "The sprite file '%s' points to a texture '%s' which doesn't exist.", m_sSpritePath.c_str(), ID.filename.c_str() );
			switch( Dialog::AbortRetryIgnore(sMessage) )
			{
			case Dialog::abort:	
				RageException::Throw( "Error reading value 'Texture' from %s.", m_sSpritePath.c_str() );
			case Dialog::retry:	
				goto retry;
			case Dialog::ignore:
				return false;
			default:
				ASSERT(0);
			}
		}
		if(asElementPaths.size() > 1)
		{
			CString message = ssprintf( 
				"There is more than one file that matches "
				"'%s'.  Please remove all but one of these matches.",
				ID.filename.c_str() );

			RageException::Throw( message ); 
		}
		ID.filename = asElementPaths[0];
	}

	// Load the texture
	LoadFromTexture( ID );

	// Read in frames and delays from the sprite file, 
	// overwriting the states that LoadFromTexture created.
	// If the .sprite file doesn't define any states, leave
	// frames and delays created during LoadFromTexture().
	for( int i=0; true; i++ )
	{
		CString sFrameKey = ssprintf( "Frame%04d", i );
		CString sDelayKey = ssprintf( "Delay%04d", i );
		State newState;

		if( !ini.GetValue( "Sprite", sFrameKey, newState.iFrameIndex ) )
			break;
		if( newState.iFrameIndex >= m_pTexture->GetNumFrames() )
			RageException::Throw( "In '%s', %s is %d, but the texture %s only has %d frames.",
				m_sSpritePath.c_str(), sFrameKey.c_str(), newState.iFrameIndex, ID.filename.c_str(), m_pTexture->GetNumFrames() );

		if( !ini.GetValue( "Sprite", sDelayKey, newState.fDelay ) )
			break;

		if( i == 0 )	// the ini file defines at least one frame
			m_States.clear();	// clear before adding

		m_States.push_back( newState );
	}

	float f;
	if( ini.GetValue( "Sprite", "BaseRotationXDegrees", f ) )	Actor::SetBaseRotationX( f );
	if( ini.GetValue( "Sprite", "BaseRotationYDegrees", f ) )	Actor::SetBaseRotationY( f );
	if( ini.GetValue( "Sprite", "BaseRotationZDegrees", f ) )	Actor::SetBaseRotationZ( f );
	if( ini.GetValue( "Sprite", "BaseZoomX", f ) )				Actor::SetBaseZoomX( f );
	if( ini.GetValue( "Sprite", "BaseZoomY", f ) )				Actor::SetBaseZoomY( f );
	if( ini.GetValue( "Sprite", "BaseZoomZ", f ) )				Actor::SetBaseZoomZ( f );


	return true;
}

void Sprite::UnloadTexture()
{
	if( m_pTexture != NULL )			// If there was a previous bitmap...
	{
		TEXTUREMAN->UnloadTexture( m_pTexture );	// Unload it.
		m_pTexture = NULL;

		/* Make sure we're reset to frame 0, so if we're reused, we aren't left on
		 * a frame number that may be greater than the number of frames in the newly
		 * loaded image. */
		SetState( 0 );
	}
}

void Sprite::EnableAnimation( bool bEnable )
{ 
	Actor::EnableAnimation( bEnable ); 
	if(m_pTexture) 
		bEnable ? m_pTexture->Play() : m_pTexture->Pause(); 
}

bool Sprite::LoadFromTexture( RageTextureID ID )
{
	LOG->Trace( "Sprite::LoadFromTexture( %s )", ID.filename.c_str() );

	if( !m_pTexture || !(m_pTexture->GetID() == ID) )
	{
		/* Load the texture if it's not already loaded.  We still need
		 * to do the rest, even if it's the same texture, since we need
		 * to reset Sprite::m_size, etc. */
		UnloadTexture();
		m_pTexture = TEXTUREMAN->LoadTexture( ID );
		ASSERT( m_pTexture->GetTextureWidth() >= 0 );
		ASSERT( m_pTexture->GetTextureHeight() >= 0 );
	}

	ASSERT( m_pTexture != NULL );

	/* Hack: if we load "_blank", mark the actor hidden, so we can short-circuit
	 * rendering later on.  (This helps NoteField rendering.) */
	if( !SetExtension(Basename(ID.filename), "").CompareNoCase("_blank") )
		this->SetHidden( true );
	
	// the size of the sprite is the size of the image before it was scaled
	Sprite::m_size.x = (float)m_pTexture->GetSourceFrameWidth();
	Sprite::m_size.y = (float)m_pTexture->GetSourceFrameHeight();		

	// Assume the frames of this animation play in sequential order with 0.1 second delay.
	m_States.clear();
	for( int i=0; i<m_pTexture->GetNumFrames(); i++ )
	{
		State newState = { i, 0.1f };
		m_States.push_back( newState );
	}

	// apply clipping (if any)
	if( m_fRememberedClipWidth != -1 && m_fRememberedClipHeight != -1 )
		ScaleToClipped( m_fRememberedClipWidth, m_fRememberedClipHeight );

	return true;
}


void Sprite::UpdateAnimationState()
{
	// Don't bother with state switching logic if there's only one state.  
	// We already know what's going to show.
	if( m_States.size() > 1 )
	{
		while( m_fSecsIntoState > m_States[m_iCurState].fDelay )	// it's time to switch frames
		{
			// increment frame and reset the counter
			m_fSecsIntoState -= m_States[m_iCurState].fDelay;		// leave the left over time for the next frame
			m_iCurState = (m_iCurState+1) % m_States.size();
		}
	}
}

void Sprite::Update( float fDelta )
{
	Actor::Update( fDelta );	// do tweening

	if( !m_bIsAnimating )
	    return;

	if( !m_pTexture )	// no texture, nothing to animate
	    return;

	m_fSecsIntoState += fDelta;
	UpdateAnimationState();

	//
	// update scrolling
	//
	if( m_fTexCoordVelocityX != 0 || m_fTexCoordVelocityY != 0 )
	{
		float fTexCoords[8];
		Sprite::GetActiveTextureCoords( fTexCoords );
 
		// top left, bottom left, bottom right, top right
		fTexCoords[0] += fDelta*m_fTexCoordVelocityX;
		fTexCoords[1] += fDelta*m_fTexCoordVelocityY; 
		fTexCoords[2] += fDelta*m_fTexCoordVelocityX;
		fTexCoords[3] += fDelta*m_fTexCoordVelocityY;
		fTexCoords[4] += fDelta*m_fTexCoordVelocityX;
		fTexCoords[5] += fDelta*m_fTexCoordVelocityY;
		fTexCoords[6] += fDelta*m_fTexCoordVelocityX;
		fTexCoords[7] += fDelta*m_fTexCoordVelocityY;

		Sprite::SetCustomTextureCoords( fTexCoords );
	}
}

static void TexCoordsFromArray(RageSpriteVertex *v, const float *f)
{
	v[0].t = RageVector2( f[0], f[1] );	// top left
	v[1].t = RageVector2( f[2],	f[3] );	// bottom left
	v[2].t = RageVector2( f[4],	f[5] );	// bottom right
	v[3].t = RageVector2( f[6],	f[7] );	// top right
}

void TexCoordArrayFromRect(float fImageCoords[8], const RectF &rect)
{
	fImageCoords[0] = rect.left;	fImageCoords[1] = rect.top;		// top left
	fImageCoords[2] = rect.left;	fImageCoords[3] = rect.bottom;	// bottom left
	fImageCoords[4] = rect.right;	fImageCoords[5] = rect.bottom;	// bottom right
	fImageCoords[6] = rect.right;	fImageCoords[7] = rect.top;		// top right
}

void Sprite::DrawTexture( const TweenState *state )
{
	// bail if cropped all the way 
    if( state->crop.left + state->crop.right >= 1  || 
		state->crop.top + state->crop.bottom >= 1 ) 
		return; 

	// use m_temp_* variables to draw the object
	RectF quadVerticies;

	switch( m_HorizAlign )
	{
	case align_left:	quadVerticies.left = 0;				quadVerticies.right = m_size.x;		break;
	case align_center:	quadVerticies.left = -m_size.x/2;	quadVerticies.right = m_size.x/2;	break;
	case align_right:	quadVerticies.left = -m_size.x;		quadVerticies.right = 0;			break;
	default:		ASSERT( false );
	}

	switch( m_VertAlign )
	{
	case align_top:		quadVerticies.top = 0;				quadVerticies.bottom = m_size.y;	break;
	case align_middle:	quadVerticies.top = -m_size.y/2;	quadVerticies.bottom = m_size.y/2;	break;
	case align_bottom:	quadVerticies.top = -m_size.y;		quadVerticies.bottom = 0;			break;
	default:		ASSERT(0);
	}


	/* Don't draw anything outside of the texture's image area.  Texels outside 
	 * of the image area aren't guaranteed to be initialized. */
	/* HACK: Clamp the crop values. It would be more accurate to clip the 
	 * vertices to that the diffuse value is adjusted. */
	RectF crop = state->crop;
	CLAMP( crop.left, 0, 1 );
	CLAMP( crop.right, 0, 1 );
	CLAMP( crop.top, 0, 1 );
	CLAMP( crop.bottom, 0, 1 );

	RectF croppedQuadVerticies = quadVerticies; 
#define IF_CROP_POS(side,opp_side) \
	if(state->crop.side!=0) \
		croppedQuadVerticies.side = \
			SCALE( crop.side, 0.f, 1.f, quadVerticies.side, quadVerticies.opp_side ); 
	IF_CROP_POS( left, right ); 
	IF_CROP_POS( top, bottom ); 
	IF_CROP_POS( right, left ); 
	IF_CROP_POS( bottom, top ); 

	static RageSpriteVertex v[4];
	v[0].p = RageVector3( croppedQuadVerticies.left,	croppedQuadVerticies.top,		0 );	// top left
	v[1].p = RageVector3( croppedQuadVerticies.left,	croppedQuadVerticies.bottom,	0 );	// bottom left
	v[2].p = RageVector3( croppedQuadVerticies.right,	croppedQuadVerticies.bottom,	0 );	// bottom right
	v[3].p = RageVector3( croppedQuadVerticies.right,	croppedQuadVerticies.top,		0 );	// top right

	DISPLAY->ClearAllTextures();
	DISPLAY->SetTexture( 0, m_pTexture );

	// Must call this after setting the texture or else texture 
	// parameters have no effect.
	Actor::SetRenderStates();	// set Actor-specified render states

	if( m_pTexture )
	{
		float f[8];
		GetActiveTextureCoords(f);
		TexCoordsFromArray(v, f);


		RageVector2 texCoords[4] = {
			RageVector2( f[0], f[1] ),	// top left
			RageVector2( f[2],	f[3] ),	// bottom left
			RageVector2( f[4],	f[5] ),	// bottom right
			RageVector2( f[6],	f[7] ) 	// top right
		};
	

		if( crop.left!=0 )
		{
			v[0].t.x = SCALE( crop.left, 0.f, 1.f, texCoords[0].x, texCoords[3].x );
			v[1].t.x = SCALE( crop.left, 0.f, 1.f, texCoords[1].x, texCoords[2].x );
		}
		if( crop.right!=0 )
		{
			v[2].t.x = SCALE( crop.right, 0.f, 1.f, texCoords[2].x, texCoords[1].x );
			v[3].t.x = SCALE( crop.right, 0.f, 1.f, texCoords[3].x, texCoords[0].x );
		}
		if( crop.top!=0 )
		{
			v[0].t.y = SCALE( crop.top, 0.f, 1.f, texCoords[0].y, texCoords[1].y );
			v[3].t.y = SCALE( crop.top, 0.f, 1.f, texCoords[3].y, texCoords[2].y );
		}
		if( crop.bottom!=0 )
		{
			v[1].t.y = SCALE( crop.bottom, 0.f, 1.f, texCoords[1].y, texCoords[0].y );
			v[2].t.y = SCALE( crop.bottom, 0.f, 1.f, texCoords[2].y, texCoords[3].y );
		}
	}
	else
	{
		// Just make sure we don't throw NaN/INF at the renderer:
		for( unsigned i = 0; i < 4; ++i )
			v[i].t.x = v[i].t.y = 0;
	}

	/* Draw if we're not fully transparent */
	if( state->diffuse[0].a > 0 || 
		state->diffuse[1].a > 0 ||
		state->diffuse[2].a > 0 ||
		state->diffuse[3].a > 0 )
	{
		DISPLAY->SetTextureModeModulate();

		//////////////////////
		// render the shadow
		//////////////////////
		if( m_fShadowLength != 0 )
		{
			DISPLAY->PushMatrix();
			DISPLAY->TranslateWorld( m_fShadowLength, m_fShadowLength, 0 );	// shift by 5 units
			v[0].c = v[1].c = v[2].c = v[3].c = RageColor(0,0,0,0.5f*state->diffuse[0].a);	// semi-transparent black
			DISPLAY->DrawQuad( v );
			DISPLAY->PopMatrix();
		}

		//////////////////////
		// render the diffuse pass
		//////////////////////
		v[0].c = state->diffuse[0];	// top left
		v[1].c = state->diffuse[2];	// bottom left
		v[2].c = state->diffuse[3];	// bottom right
		v[3].c = state->diffuse[1];	// top right
		DISPLAY->DrawQuad( v );
	}

	//////////////////////
	// render the glow pass
	//////////////////////
	if( state->glow.a > 0.0001f )
	{
		DISPLAY->SetTextureModeGlow(state->glowmode);
		v[0].c = v[1].c = v[2].c = v[3].c = state->glow;
		DISPLAY->DrawQuad( v );
	}
}

/* XXX: move to RageTypes.cpp */
RageColor scale( float x, float l1, float h1, const RageColor &a, const RageColor &b )
{
	return RageColor(
		SCALE( x, l1, h1, a.r, b.r ),
		SCALE( x, l1, h1, a.g, b.g ),
		SCALE( x, l1, h1, a.b, b.b ),
		SCALE( x, l1, h1, a.a, b.a ) );
}

bool Sprite::EarlyAbortDraw()
{
	return m_pTexture == NULL && !m_bDrawIfTextureNull;
//	return false;
}

void Sprite::DrawPrimitives()
{
	if( m_pTempState->fade.top > 0 ||
		m_pTempState->fade.bottom > 0 ||
		m_pTempState->fade.left > 0 ||
		m_pTempState->fade.right > 0 )
	{
		/* We're fading the edges.  */
		const RectF &FadeDist = m_pTempState->fade;

		/* Actual size of the fade on each side: */
		RectF FadeSize = FadeDist;

		/* If the cropped size is less than the fade distance in either dimension, clamp. */
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

		const RageColor &FadeColor = m_pTempState->fadecolor;

		/* Alpha value of the un-faded side of each fade rect: */
		const RageColor RightColor  = scale( FadeSize.right,  FadeDist.right,  0, RageColor(1,1,1,1), FadeColor );
		const RageColor LeftColor   = scale( FadeSize.left,   FadeDist.left,   0, RageColor(1,1,1,1), FadeColor );
		const RageColor TopColor    = scale( FadeSize.top,    FadeDist.top,    0, RageColor(1,1,1,1), FadeColor );
		const RageColor BottomColor = scale( FadeSize.bottom, FadeDist.bottom, 0, RageColor(1,1,1,1), FadeColor );

		/* Draw the inside: */
		TweenState ts = *m_pTempState;
		ts.crop.left += FadeDist.left;
		ts.crop.right += FadeDist.right;
		ts.crop.top += FadeDist.top;
		ts.crop.bottom += FadeDist.bottom;
		DrawTexture( &ts );

		if( FadeSize.left > 0.001f )
		{
			/* Draw the left: */
			ts.crop = m_pTempState->crop; // restore
			memcpy( ts.diffuse, m_pTempState->diffuse, sizeof(ts.diffuse) ); // restore

			ts.crop.right = 1 - (ts.crop.left + FadeSize.left);
			ts.crop.top += FadeDist.top;		// lop off the corner if fading both x and y
			ts.crop.bottom += FadeDist.bottom;
			ts.diffuse[0] *= FadeColor;			// top left
			ts.diffuse[2] *= FadeColor;			// bottom left
			ts.diffuse[3] *= LeftColor;			// bottom right
			ts.diffuse[1] *= LeftColor;			// top right
			DrawTexture( &ts );
		}

		if( FadeSize.right > 0.001f )
		{
			/* Draw the right: */
			ts.crop = m_pTempState->crop; // restore
			memcpy( ts.diffuse, m_pTempState->diffuse, sizeof(ts.diffuse) ); // restore

			ts.crop.left = 1 - (ts.crop.right + FadeSize.right);
			ts.crop.top += FadeDist.top;
			ts.crop.bottom += FadeDist.bottom;
			ts.diffuse[0] *= RightColor;		// top left
			ts.diffuse[2] *= RightColor;		// bottom left
			ts.diffuse[3] *= FadeColor;			// bottom right
			ts.diffuse[1] *= FadeColor;			// top right
			DrawTexture( &ts );
		}

		if( FadeSize.top > 0.001f )
		{
			/* Draw the top: */
			ts.crop = m_pTempState->crop; // restore
			memcpy( ts.diffuse, m_pTempState->diffuse, sizeof(ts.diffuse) ); // restore

			ts.crop.bottom = 1 - (ts.crop.top + FadeSize.top);
			ts.crop.left += FadeDist.left;
			ts.crop.right += FadeDist.right;
			ts.diffuse[0] *= FadeColor;			// top left
			ts.diffuse[2] *= TopColor;			// bottom left
			ts.diffuse[3] *= TopColor;			// bottom right
			ts.diffuse[1] *= FadeColor;			// top right
			DrawTexture( &ts );
		}

		if( FadeSize.bottom > 0.001f )
		{
			/* Draw the bottom: */
			ts.crop = m_pTempState->crop; // restore
			memcpy( ts.diffuse, m_pTempState->diffuse, sizeof(ts.diffuse) ); // restore

			ts.crop.top = 1 - (ts.crop.bottom + FadeSize.bottom);
			ts.crop.left += FadeDist.left;
			ts.crop.right += FadeDist.right;
			ts.diffuse[0] *= BottomColor;		// top left
			ts.diffuse[2] *= FadeColor;			// bottom left
			ts.diffuse[3] *= FadeColor;			// bottom right
			ts.diffuse[1] *= BottomColor;		// top right
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
	// This assert will likely trigger if the "missing" theme element graphic 
	// is loaded in place of a multi-frame sprite.  We want to know about these
	// problems in debug builds, but they're not fatal.
	if( iNewState < 0  ||  iNewState >= (int)m_States.size() )
	{
		CString sError;
		if( m_pTexture )
			sError = ssprintf("The Sprite '%s' (\"%s\") tried to set state index %d, but it has only %u states", 
				m_pTexture->GetID().filename.c_str(), this->m_sName.c_str(), iNewState, unsigned(m_States.size()));
		else
			sError = ssprintf("A Sprite (\"%s\") tried to set state index %d but no texture is loaded.", 
				this->m_sName.c_str(), iNewState );
		Dialog::OK( sError );
	}

	CLAMP(iNewState, 0, (int)m_States.size()-1);
	m_iCurState = iNewState;
	m_fSecsIntoState = 0.0; 
}

float Sprite::GetAnimationLengthSeconds() const
{
	float fTotal = 0;
	FOREACH_CONST( State, m_States, s )
		fTotal += s->fDelay;
	return fTotal;
}

void Sprite::SetSecondsIntoAnimation( float fSeconds )
{
	SetState( 0 );	// rewind to the first state
	m_fSecsIntoState = fSeconds;
	UpdateAnimationState();
}

CString	Sprite::GetTexturePath() const
{
	if( m_pTexture==NULL )
		return "";

	return m_pTexture->GetID().filename;
}

void Sprite::SetCustomTextureRect( const RectF &new_texcoord_frect ) 
{ 
	m_bUsingCustomTexCoords = true;
	m_bTextureWrapping = true;
	TexCoordArrayFromRect(m_CustomTexCoords, new_texcoord_frect);
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
	rectImageCoords.top		*= m_pTexture->GetImageHeight()	/ (float)m_pTexture->GetTextureHeight(); 
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

const RectF *Sprite::GetCurrentTextureCoordRect() const
{
	ASSERT_M( m_iCurState < (int) m_States.size(), ssprintf("%d, %d", int(m_iCurState), int(m_States.size())) );

	unsigned int uFrameNo = m_States[m_iCurState].iFrameIndex;
	return m_pTexture->GetTextureCoordRect( uFrameNo );
}


/* If we're using custom coordinates, return them; otherwise return the coordinates
 * for the current state. */
void Sprite::GetActiveTextureCoords(float fTexCoordsOut[8]) const
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
		TexCoordArrayFromRect(fTexCoordsOut, *pTexCoordRect);
	}
}


void Sprite::StopUsingCustomCoords()
{
	m_bUsingCustomTexCoords = false;
}


void Sprite::ScaleToClipped( float fWidth, float fHeight )
{
	m_fRememberedClipWidth = fWidth;
	m_fRememberedClipHeight = fHeight;

	if( !m_pTexture )
		return;

	int iSourceWidth	= m_pTexture->GetSourceWidth();
	int iSourceHeight	= m_pTexture->GetSourceHeight();

	// save the original X&Y.  We're going to resore them later.
	float fOriginalX = GetX();
	float fOriginalY = GetY();

	if( IsDiagonalBanner(iSourceWidth, iSourceHeight) )		// this is a SSR/DWI CroppedSprite
	{
		float fCustomImageCoords[8] = {
			0.02f,	0.78f,	// top left
			0.22f,	0.98f,	// bottom left
			0.98f,	0.22f,	// bottom right
			0.78f,	0.02f,	// top right
		};
		Sprite::SetCustomImageCoords( fCustomImageCoords );

		if( fWidth != -1 && fHeight != -1)
			m_size = RageVector2( fWidth, fHeight );
		else
		{
			/* If no crop size is set, then we're only being used to crop diagonal
			 * banners so they look like regular ones. We don't actually care about
			 * the size of the image, only that it has an aspect ratio of 4:1.  */
			m_size = RageVector2(256, 64);
		}
		SetZoom( 1 );
	}
	else if( m_pTexture->GetID().filename.find( "(was rotated)" ) != m_pTexture->GetID().filename.npos && 
			 fWidth != -1 && fHeight != -1 )
	{
		/* Dumb hack.  Normally, we crop all sprites except for diagonal banners,
		 * which are stretched.  Low-res versions of banners need to do the same
		 * thing as their full resolution counterpart, so the crossfade looks right.
		 * However, low-res diagonal banners are un-rotated, to save space.  BannerCache
		 * drops the above text into the "filename" (which is otherwise unused for
		 * these banners) to tell us this.
		 */
		Sprite::StopUsingCustomCoords();
		m_size = RageVector2( fWidth, fHeight );
		SetZoom( 1 );
	}
	else if( fWidth != -1 && fHeight != -1 )
	{
		// this is probably a background graphic or something not intended to be a CroppedSprite
		Sprite::StopUsingCustomCoords();

		// first find the correct zoom
		Sprite::ScaleToCover( RectI(0, 0,
									(int)fWidth,
									(int)fHeight )
							 );
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

bool Sprite::IsDiagonalBanner( int iWidth, int iHeight )
{
	/* A diagonal banner is a square.  Give a couple pixels of leeway. */
	return iWidth >= 100 && abs(iWidth - iHeight) < 2;
}

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

void Sprite::HandleCommand( const ParsedCommand &command )
{
	HandleParams;

	const CString& sName = sParam(0);

	// Commands that go in the tweening queue:
	// Commands that take effect immediately (ignoring the tweening queue):
	if( sName=="customtexturerect" )	SetCustomTextureRect( RectF(fParam(1),fParam(2),fParam(3),fParam(4)) );
	else if( sName=="texcoordvelocity" )	SetTexCoordVelocity( fParam(1),fParam(2) );
	else if( sName=="scaletoclipped" )	ScaleToClipped( fParam(1),fParam(2) );
	else if( sName=="stretchtexcoords" )	StretchTexCoords( fParam(1),fParam(2) );

	/* Texture commands; these could be moved to RageTexture* (even though that's
	 * not an Actor) if these are needed for other things that use textures.
	 * We'd need to break the command helpers into a separate function; RageTexture
	 * shouldn't depend on Actor. */
	else if( sName=="position" )		GetTexture()->SetPosition( fParam(1) );
	else if( sName=="loop" )			GetTexture()->SetLooping( bParam(1) );
	else if( sName=="play" )			GetTexture()->Play();
	else if( sName=="pause" )			GetTexture()->Pause();
	else if( sName=="rate" )			GetTexture()->SetPlaybackRate( fParam(1) );
	else
	{
		Actor::HandleCommand( command );
		return;
	}

	CheckHandledParams;
}

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
