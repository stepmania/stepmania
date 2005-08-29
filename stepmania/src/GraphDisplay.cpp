#include "global.h"
#include "GraphDisplay.h"
#include "ThemeManager.h"
#include "RageTextureManager.h"
#include "RageDisplay.h"
#include "RageUtil.h"
#include "StageStats.h"
#include "Foreach.h"
#include "song.h"

//#define DIVIDE_LINE_WIDTH			THEME->GetMetricI(m_sName,"TexturedBottomHalf")

GraphDisplay::GraphDisplay()
{
	m_pTexture = NULL;
}

/* Draw the graph as a z-buffer mask. */
void GraphDisplay::Load( const CString &TexturePath, const CString &sJustBarelyPath )
{
	memset( m_Values, 0, sizeof(m_Values) );

	Unload();
	m_pTexture = TEXTUREMAN->LoadTexture( TexturePath );
	m_size.x = (float) m_pTexture->GetSourceWidth();
	m_size.y = (float) m_pTexture->GetSourceHeight();

	m_sprJustBarely.Load( sJustBarelyPath );
}

void GraphDisplay::Unload()
{
	if( m_pTexture != NULL )
		TEXTUREMAN->UnloadTexture( m_pTexture );

	m_pTexture = NULL;

	FOREACH( AutoActor*, m_vpSongBoundaries, p )
		SAFE_DELETE( *p );
	m_vpSongBoundaries.clear();

	m_sprJustBarely.Unload();

	ActorFrame::RemoveAllChildren();
}

void GraphDisplay::LoadFromStageStats( const StageStats &ss, const PlayerStageStats &pss, const CString &sSongBoundaryPath )
{
	float fTotalStepSeconds = ss.GetTotalPossibleStepsSeconds();

	pss.GetLifeRecord( m_Values, VALUE_RESOLUTION, ss.GetTotalPossibleStepsSeconds() );
	for( unsigned i=0; i<ARRAYSIZE(m_Values); i++ )
		CLAMP( m_Values[i], 0.f, 1.f );

	//
	// Show song boundaries
	//
	float fSec = 0;
	FOREACH_CONST( Song*, ss.vpPossibleSongs, song )
	{
		if( song == ss.vpPossibleSongs.end()-1 )
			continue;
		fSec += (*song)->GetStepsSeconds();

		AutoActor *p = new AutoActor;
		m_vpSongBoundaries.push_back( p );
		p->Load( sSongBoundaryPath );
		float fX = SCALE( fSec, 0, fTotalStepSeconds, m_quadVertices.left, m_quadVertices.right );
		(*p)->SetX( fX );
		this->AddChild( *p );
	}

	if( !pss.bFailed && !pss.bGaveUp )
	{
		//
		// Search for the min life record to show "Just Barely!"
		//
		float fMinLifeSoFar = 1.0f;
		int iMinLifeSoFarAt = 0;

		int NumSlices = VALUE_RESOLUTION-1;
		for( int i = 0; i < NumSlices; ++i )
		{
			float fLife = m_Values[i];
			if( fLife < fMinLifeSoFar )
			{
				fMinLifeSoFar = fLife;
				iMinLifeSoFarAt = i;
			}
		}

		if( fMinLifeSoFar > 0.0f  &&  fMinLifeSoFar < 0.1f )
		{
			float fX = SCALE( float(iMinLifeSoFarAt), 0.0f, float(NumSlices), m_quadVertices.left, m_quadVertices.right );
			m_sprJustBarely->SetXY( fX, 0 );
		}
		else
		{
			m_sprJustBarely->SetHidden( true );
		}
		this->AddChild( m_sprJustBarely );
	}

	UpdateVerts();
}

void GraphDisplay::UpdateVerts()
{
	switch( m_HorizAlign )
	{
	case align_left:	m_quadVertices.left = 0;			m_quadVertices.right = m_size.x;		break;
	case align_center:	m_quadVertices.left = -m_size.x/2;	m_quadVertices.right = m_size.x/2;	break;
	case align_right:	m_quadVertices.left = -m_size.x;	m_quadVertices.right = 0;			break;
	default:		ASSERT( false );
	}

	switch( m_VertAlign )
	{
	case align_top:		m_quadVertices.top = 0;				m_quadVertices.bottom = m_size.y;	break;
	case align_middle:	m_quadVertices.top = -m_size.y/2;	m_quadVertices.bottom = m_size.y/2;	break;
	case align_bottom:	m_quadVertices.top = -m_size.y;		m_quadVertices.bottom = 0;			break;
	default:		ASSERT(0);
	}

	int iNumSlices = VALUE_RESOLUTION-1;

	for( int i = 0; i < 4*iNumSlices; ++i )
		m_Slices[i].c = RageColor(1,1,1,1);

	for( int i = 0; i < iNumSlices; ++i )
	{
		const float fLeft = SCALE( float(i), 0.0f, float(iNumSlices), m_quadVertices.left, m_quadVertices.right );
		const float fRight = SCALE( float(i+1), 0.0f, float(iNumSlices), m_quadVertices.left, m_quadVertices.right );
		const float fLeftBottom = SCALE( float(m_Values[i]), 0.0f, 1.0f, m_quadVertices.top, m_quadVertices.bottom );
		const float fRightBottom = SCALE( float(m_Values[i+1]), 0.0f, 1.0f, m_quadVertices.top, m_quadVertices.bottom );

		m_Slices[i*4+0].p = RageVector3( fLeft,		m_quadVertices.top,	0 );	// top left
		m_Slices[i*4+1].p = RageVector3( fLeft,		fLeftBottom,	0 );	// bottom left
		m_Slices[i*4+2].p = RageVector3( fRight,	fRightBottom,	0 );	// bottom right
		m_Slices[i*4+3].p = RageVector3( fRight,	m_quadVertices.top,	0 );	// top right
	}

	const RectF *tex = m_pTexture->GetTextureCoordRect( 0 );
	for( unsigned i = 0; i < ARRAYSIZE(m_Slices); ++i )
	{
		m_Slices[i].t = RageVector2( 
			SCALE( m_Slices[i].p.x, m_quadVertices.left, m_quadVertices.right, tex->left, tex->right ),
			SCALE( m_Slices[i].p.y, m_quadVertices.top, m_quadVertices.bottom, tex->top, tex->bottom )
			);
	}
}

void GraphDisplay::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	UpdateVerts();
}

void GraphDisplay::DrawPrimitives()
{
	Actor::SetGlobalRenderStates();	// set Actor-specified render states

	DISPLAY->ClearAllTextures();
	DISPLAY->SetTexture( 0, m_pTexture );

	// Must call this after setting the texture or else texture 
	// parameters have no effect.
	Actor::SetTextureRenderStates();

	DISPLAY->SetTextureModeModulate();
	DISPLAY->DrawQuads( m_Slices, ARRAYSIZE(m_Slices) );
	DISPLAY->SetTexture( 0, NULL );

	ActorFrame::DrawPrimitives();
}

/*
 * (c) 2003 Glenn Maynard
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
