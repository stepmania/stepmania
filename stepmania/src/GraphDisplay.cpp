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


void GraphDisplay::Load( const CString &TexturePath, float fInitialHeight, const CString &sJustBarelyPath )
{
	m_Position = 1;
	memset( m_CurValues, 0, sizeof(m_CurValues) );
	memset( m_DestValues, 0, sizeof(m_DestValues) );
	memset( m_LastValues, 0, sizeof(m_LastValues) );

	Unload();
	m_pTexture = TEXTUREMAN->LoadTexture( TexturePath );
	m_size.x = (float) m_pTexture->GetSourceWidth();
	m_size.y = (float) m_pTexture->GetSourceHeight();

	for( int i = 0; i < VALUE_RESOLUTION; ++i )
		m_CurValues[i] = fInitialHeight;
	UpdateVerts();

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

	memcpy( m_LastValues, m_CurValues, sizeof(m_CurValues) );
	m_Position = 0;
	pss.GetLifeRecord( m_DestValues, VALUE_RESOLUTION, ss.GetTotalPossibleStepsSeconds() );
	for( unsigned i=0; i<ARRAYSIZE(m_DestValues); i++ )
		CLAMP( m_DestValues[i], 0.f, 1.f );
	UpdateVerts();


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
		float fMinLifeSoFarAtSecond = 0;
		FOREACHM_CONST( float, float, pss.fLifeRecord, i )
		{
			float fSec = i->first;
			float fLife = i->second;
			if( fLife < fMinLifeSoFar )
			{
				fMinLifeSoFar = fLife;
				fMinLifeSoFarAtSecond = fSec;
			}
		}
		
		if( fMinLifeSoFar > 0.0f  &&  fMinLifeSoFar < 0.1f )
		{
			float fX = SCALE( fMinLifeSoFarAtSecond, 0, fTotalStepSeconds, m_quadVertices.left, m_quadVertices.right );
			fX = Quantize( fX, 1.0f );
			m_sprJustBarely->SetXY( fX, 0 );
		}
		else
		{
			m_sprJustBarely->SetHidden( true );
		}
		this->AddChild( m_sprJustBarely );
	}
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

	int NumSlices = VALUE_RESOLUTION-1;

	for( int i = 0; i < 4*NumSlices; ++i )
		m_Slices[i].c = RageColor(1,1,1,1);

	for( int i = 0; i < NumSlices; ++i )
	{
		const float Left = SCALE( float(i), 0.0f, float(NumSlices), m_quadVertices.left, m_quadVertices.right );
		const float Right = SCALE( float(i+1), 0.0f, float(NumSlices), m_quadVertices.left, m_quadVertices.right );
		const float LeftTop = SCALE( float(m_CurValues[i]), 0.0f, 1.0f, m_quadVertices.bottom, m_quadVertices.top );
		const float RightTop = SCALE( float(m_CurValues[i+1]), 0.0f, 1.0f, m_quadVertices.bottom, m_quadVertices.top );

		m_Slices[i*4+0].p = RageVector3( Left,		LeftTop,	0 );	// top left
		m_Slices[i*4+1].p = RageVector3( Left,		m_quadVertices.bottom,	0 );	// bottom left
		m_Slices[i*4+2].p = RageVector3( Right,		m_quadVertices.bottom,	0 );	// bottom right
		m_Slices[i*4+3].p = RageVector3( Right,		RightTop,	0 );	// top right

	//	m_Slices[i*4+0].c = RageColor(.2,.2,.2,1);
	//	m_Slices[i*4+1].c = RageColor(1,1,1,1);
	//	m_Slices[i*4+2].c = RageColor(1,1,1,1);
	//	m_Slices[i*4+3].c = RageColor(.2,.2,.2,1);
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

	if( m_Position == 1 )
		return;

	m_Position = clamp( m_Position+fDeltaTime, 0, 1 );
	for( int i = 0; i < VALUE_RESOLUTION; ++i )
		m_CurValues[i] = m_DestValues[i]*m_Position + m_LastValues[i]*(1-m_Position);

	UpdateVerts();
}

void GraphDisplay::DrawPrimitives()
{
	Actor::SetGlobalRenderStates();	// set Actor-specified render states

	DISPLAY->ClearAllTextures();
	DISPLAY->SetTexture( 0, m_pTexture );
	// don't bother setting texture render states for a null texture
	//Actor::SetTextureRenderStates();

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
