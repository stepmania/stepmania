#include "global.h"
#include "DualScrollBar.h"
#include "ThemeManager.h"

DualScrollBar::DualScrollBar()
{
	m_fBarHeight = 100;
	m_fBarTime = 1;
}

void DualScrollBar::Load()
{
	int pn;

	for( pn=0; pn < NUM_PLAYERS; ++pn )
	{
		m_sprScrollThumbUnderHalf[pn].SetName( ssprintf("ThumbP%i", pn+1) );
		m_sprScrollThumbUnderHalf[pn].Load( THEME->GetPathToG( ssprintf("%s thumb p%i", m_sName.c_str(), pn+1) ) );
		this->AddChild( &m_sprScrollThumbUnderHalf[pn] );
	}

	for( pn=0; pn < NUM_PLAYERS; ++pn )
	{
		m_sprScrollThumbOverHalf[pn].SetName( ssprintf("ThumbP%i", pn+1) );
		m_sprScrollThumbOverHalf[pn].Load( THEME->GetPathToG( ssprintf("%s thumb p%i", m_sName.c_str(), pn+1) ) );
		this->AddChild( &m_sprScrollThumbOverHalf[pn] );
	}

	m_sprScrollThumbUnderHalf[0].SetCropLeft( .5f );
	m_sprScrollThumbUnderHalf[1].SetCropRight( .5f );

	m_sprScrollThumbOverHalf[0].SetCropRight( .5f );
	m_sprScrollThumbOverHalf[1].SetCropLeft( .5f );

	for( pn=0; pn < NUM_PLAYERS; ++pn )
		SetPercentage( (PlayerNumber) pn, 0 );

	FinishTweening();
}

void DualScrollBar::EnablePlayer( PlayerNumber pn, bool on )
{
	m_sprScrollThumbUnderHalf[pn].SetHidden( !on );
	m_sprScrollThumbOverHalf[pn].SetHidden( !on );
}

void DualScrollBar::SetPercentage( PlayerNumber pn, float fPercent )
{
	const float bottom = m_fBarHeight/2 - m_sprScrollThumbUnderHalf[pn].GetZoomedHeight()/2;
	const float top = -bottom;

	/* Position both thumbs. */
	m_sprScrollThumbUnderHalf[pn].StopTweening();
	m_sprScrollThumbUnderHalf[pn].BeginTweening( m_fBarTime );
	m_sprScrollThumbUnderHalf[pn].SetY( SCALE( fPercent, 0, 1, top, bottom ) );

	m_sprScrollThumbOverHalf[pn].StopTweening();
	m_sprScrollThumbOverHalf[pn].BeginTweening( m_fBarTime );
	m_sprScrollThumbOverHalf[pn].SetY( SCALE( fPercent, 0, 1, top, bottom ) );
}
