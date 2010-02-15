#include "global.h"
#include "StreamDisplay.h"
#include "GameState.h"
#include <float.h>
#include "RageDisplay.h"
#include "ThemeManager.h"
#include "EnumHelper.h"

static const char *StreamTypeNames[] = {
	"Normal",
	"Passing",
	"Hot",
};
XToString( StreamType );

StreamDisplay::StreamDisplay()
{
	m_fPercent = 0;
	m_fTrailingPercent = 0;
	m_fVelocity = 0;
	m_fPassingAlpha = 0;
	m_fHotAlpha = 0;
	m_fThreePartWidth = 0;
	m_bAlwaysBounce = false;
}

void StreamDisplay::Load( const RString &_sMetricsGroup )
{
	// XXX: actually load from the metrics group passed in -aj
	RString sMetricsGroup = "StreamDisplay";

	m_transformPill.SetFromReference( THEME->GetMetricR(sMetricsGroup,"PillTransformFunction") );

	float fTextureCoordScaleX = THEME->GetMetricF(sMetricsGroup,"TextureCoordScaleX");
	int iNumPills = THEME->GetMetricF(sMetricsGroup,"NumPills");
	m_bUsingThreePart = THEME->GetMetricB(sMetricsGroup,"UseThreePartMethod");
	m_bAlwaysBounce = THEME->GetMetricB(sMetricsGroup,"AlwaysBounceNormalBar");

	// three part method only uses 3 pills
	if(iNumPills != 3)
		m_bUsingThreePart = false;

	FOREACH_ENUM( StreamType, st )
	{
		ASSERT( m_vpSprPill[st].empty() );

		for( int i=0; i<iNumPills; i++ )
		{
			Sprite *pSpr = new Sprite;

			if( m_bUsingThreePart )
			{
				pSpr->Load( THEME->GetPathG( sMetricsGroup, ssprintf("%s part%d",StreamTypeToString(st).c_str(),i) ) );
				
				if(pSpr->GetNumStates() > 1)
				{
					pSpr->SetAllStateDelays( THEME->GetMetricF( sMetricsGroup, ssprintf("%spart%ddelay",StreamTypeToString(st).c_str(),i) ) );
				}
			}
			else
				pSpr->Load( THEME->GetPathG( sMetricsGroup, StreamTypeToString(st) ) );
			m_vpSprPill[st].push_back( pSpr );

			if( !m_bUsingThreePart )
			{
				m_transformPill.TransformItemDirect( *pSpr, -1, i, iNumPills );
				float f = 1 / fTextureCoordScaleX;
				pSpr->SetCustomTextureRect( RectF(f*i,0,f*(i+1),1) );
			}

			this->AddChild( pSpr );
		}

		if( m_bUsingThreePart )
		{
			m_fThreePartWidth = THEME->GetMetricF( sMetricsGroup, "ThreePartWidth" );
			// first element positioned depending on metric width specified
			m_vpSprPill[st][0]->AddX( -(m_fThreePartWidth/2 + m_vpSprPill[st][0]->GetZoomedWidth()/2) );			
		}
	}
}

void StreamDisplay::Update( float fDeltaSecs )
{
	ActorFrame::Update( fDeltaSecs );

	// HACK:  Tweaking these values is very difficult.  Update the
	// "physics" many times so that the spring motion appears faster
	for( int i=0; i<10; i++ )
	{
		const float fDelta = m_fPercent - m_fTrailingPercent;

		// Don't apply spring and viscous forces if we're full or empty.
		// Just move straight to either full or empty.
		if( m_fPercent <= 0 || m_fPercent >= 1 )
		{
			if( fabsf(fDelta) < 0.00001f )
				m_fVelocity = 0; // prevent div/0
			else
				m_fVelocity = (fDelta / fabsf(fDelta)) * 4;
		}
		else
		{
			const float fSpringForce = fDelta * 2.0f;
			m_fVelocity += fSpringForce * fDeltaSecs;

			const float fViscousForce = -m_fVelocity * 0.2f;
			if( !m_bAlwaysBounce )
				m_fVelocity += fViscousForce * fDeltaSecs;
		}

		CLAMP( m_fVelocity, -.06f, +.02f );

		m_fTrailingPercent += m_fVelocity * fDeltaSecs;
	}

	// Don't clamp life percentage a little outside the visible range so
	// that the clamp doesn't dampen the "jiggle" of the meter.
	CLAMP( m_fTrailingPercent, -0.1f, 1.1f );


	// set crop of pills
	const float fPillWidthPercent = 1.0f / m_vpSprPill[0].size();
	FOREACH_ENUM( StreamType, st )
	{
		for( int i=0; i<(int)m_vpSprPill[st].size(); i++ )
		{
			Sprite *pSpr = m_vpSprPill[st][i];
			float fPercentFilledThisPill = SCALE( m_fTrailingPercent, fPillWidthPercent*i, fPillWidthPercent*(i+1), 0.0f, 1.0f );
			CLAMP( fPercentFilledThisPill, 0.0f, 1.0f );

			// XXX scale by current song speed

			if( !m_bUsingThreePart ) // usual lifebar
			{
				pSpr->SetCropRight( 1.0f - fPercentFilledThisPill );
				pSpr->SetTexCoordVelocity( -1, 0 );
			}
			else // using the three-part method
			{

				if( i==1 ) // middle pill
				{
					float fMiddleWidth = m_fThreePartWidth * m_fTrailingPercent;
					pSpr->ZoomToWidth( fMiddleWidth );

					float fMiddleX = m_vpSprPill[st][0]->GetX() + ( fMiddleWidth/2 ) + ( m_vpSprPill[st][0]->GetZoomedWidth()/2 );
					pSpr->SetX( fMiddleX );
				}
				else if( i!=0 ) // last pill
				{
					float fEndX = m_vpSprPill[st][1]->GetX() + ( m_vpSprPill[st][1]->GetZoomedWidth()/2 ) + ( pSpr->GetZoomedWidth()/2 );
					pSpr->SetX( fEndX );
				}
			}
			// Optimization: Don't draw pills that are covered up
			switch( st )
			{
			DEFAULT_FAIL( st );
			case StreamType_Normal:
				pSpr->SetVisible( m_fPassingAlpha < 1  ||  m_fHotAlpha < 1 );
				pSpr->SetDiffuseAlpha( 1 );
				break;
			case StreamType_Passing:
				pSpr->SetDiffuseAlpha( m_fPassingAlpha );
				pSpr->SetVisible( m_fHotAlpha < 1 );
				break;
			case StreamType_Hot:
				pSpr->SetDiffuseAlpha( m_fHotAlpha );
				break;
			}
		}
	}
}

void StreamDisplay::SetPercent( float fPercent )
{
#ifdef DEBUG
	float fLifeMultiplier = THEME->GetMetricF("LifeMeterBar","LifeMultiplier");
#endif
	DEBUG_ASSERT( fPercent >= 0.0f && fPercent <= 1.0f * fLifeMultiplier );
	if( isnan(fPercent) )
	{
		DEBUG_ASSERT_M( 0, "fPercent is NaN" );
		fPercent = 1;
	}
	if( !isfinite(fPercent) )
	{
		DEBUG_ASSERT_M( 0, "fPercent is infinite" );
		fPercent = 1;
	}

	float fDeltaPercent = fPercent - m_fPercent;
	m_fVelocity += fDeltaPercent;
	m_fPercent = fPercent;
}


/*
 * (c) 2003-2004 Chris Danford
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
