#include "global.h"
#include "GroupList.h"
#include "ThemeManager.h"
#include "SongManager.h"

/* If this actor is used anywhere other than SelectGroup, we
 * can add a setting that changes which metric group we pull
 * settings out of, so it can be configured separately. */
#define START_X				THEME->GetMetricF("GroupList","StartX")
#define START_Y				THEME->GetMetricF("GroupList","StartY")
#define SPACING_X			THEME->GetMetricF("GroupList","SpacingX")
#define SPACING_Y			THEME->GetMetricF("GroupList","SpacingY")
#define GAIN_FOCUS_COMMAND	THEME->GetMetric ("GroupList","GainFocusCommand")
#define LOSE_FOCUS_COMMAND	THEME->GetMetric ("GroupList","LoseFocusCommand")


GroupList::GroupList()
{
	m_iSelection = m_iTop = 0;
}

void GroupList::Load( const CStringArray& asGroupNames )
{
	m_asLabels = asGroupNames;

	for( unsigned i=0; i<min(m_asLabels.size(), MAX_GROUPS_ONSCREEN); i++ )
	{
		float fX = START_X + i*SPACING_X;
		float fY = START_Y + i*SPACING_Y;

		m_sprButton[i].Load( THEME->GetPathTo("Graphics","GroupList bar") );
		m_sprButton[i].SetXY( fX, fY );
		this->AddChild( &m_sprButton[i] );

		m_screenLabels[i].LoadFromFont( THEME->GetPathTo("Fonts","GroupList label") );
		m_screenLabels[i].SetXY( fX, fY );
		m_screenLabels[i].SetShadowLength( 2 );
		this->AddChild( &m_screenLabels[i] );
	}

	SetLabels();
	AfterChange();
}

void GroupList::SetLabels()
{
	for( unsigned i=0; i<min(m_asLabels.size(), MAX_GROUPS_ONSCREEN); i++ )
	{
		CString &label = m_asLabels[m_iTop+i];
		m_screenLabels[i].SetText( SONGMAN->ShortenGroupName( label ) );
		float fTextWidth = (float)m_screenLabels[i].GetWidestLineWidthInSourcePixels();
		float fButtonWidth = m_sprButton[i].GetZoomedWidth();

		float fZoom = fButtonWidth/fTextWidth;
		fZoom = min( fZoom, 0.8f );
		m_screenLabels[i].SetZoomX( fZoom );
		m_screenLabels[i].SetZoomY( 0.8f );

		if( m_iTop+i == 0 )	m_screenLabels[i].TurnRainbowOn();
		else {
			m_screenLabels[i].TurnRainbowOff();
			m_screenLabels[i].SetDiffuse( SONGMAN->GetGroupColor(label) );
		}
	}
}

void GroupList::BeforeChange()
{
	int iSel = m_iSelection-m_iTop;

	m_sprButton[iSel].Command( LOSE_FOCUS_COMMAND );
	m_screenLabels[iSel].Command( LOSE_FOCUS_COMMAND );
}


void GroupList::AfterChange()
{
	int iSel = m_iSelection-m_iTop;

	m_sprButton[iSel].Command( GAIN_FOCUS_COMMAND );
	m_screenLabels[iSel].Command( GAIN_FOCUS_COMMAND );
}

void GroupList::Up()
{
	BeforeChange();

	if( m_iSelection == 0 )
		SetSelection(m_asLabels.size()-1);
	else
		SetSelection(m_iSelection-1);

	AfterChange();
}

void GroupList::Down()
{
	BeforeChange();

	SetSelection((m_iSelection+1) % m_asLabels.size());
	
	AfterChange();
}

void GroupList::SetSelection( unsigned sel )
{
	BeforeChange();

	m_iSelection=sel;

	if( m_asLabels.size() <= MAX_GROUPS_ONSCREEN ||
		  sel <= MAX_GROUPS_ONSCREEN/2 )
		m_iTop = 0;
	else if ( sel >= m_asLabels.size() - MAX_GROUPS_ONSCREEN/2 )
		m_iTop = m_asLabels.size() - MAX_GROUPS_ONSCREEN;
	else
		m_iTop = sel - MAX_GROUPS_ONSCREEN/2;

	/* The current selection must always be visible. */
	ASSERT( m_iTop <= m_iSelection );
	ASSERT( m_iTop+MAX_GROUPS_ONSCREEN > m_iSelection );

	SetLabels();
	AfterChange();
}


void GroupList::TweenOnScreen()
{
	for( unsigned i=0; i<min(m_asLabels.size(), MAX_GROUPS_ONSCREEN); i++ )
	{
		m_sprButton[i].SetX( 400 );
		m_sprButton[i].BeginTweening( 0.1f*i, TWEEN_BOUNCE_END );
		m_sprButton[i].BeginTweening( 0.2f, TWEEN_BOUNCE_END );
		m_sprButton[i].SetTweenX( 0 );

		m_screenLabels[i].SetX( 400 );
		m_screenLabels[i].BeginTweening( 0.1f*i, TWEEN_BOUNCE_END );
		m_screenLabels[i].BeginTweening( 0.2f, TWEEN_BOUNCE_END );
		m_screenLabels[i].SetTweenX( 0 );
	}
}

void GroupList::TweenOffScreen()
{
	for( unsigned i=0; i<min(m_asLabels.size(), MAX_GROUPS_ONSCREEN); i++ )
	{
		if( i == m_iSelection )
			m_sprButton[i].BeginTweening( 1.0f, TWEEN_BOUNCE_BEGIN );
		else
			m_sprButton[i].BeginTweening( 0.1f*i, TWEEN_BOUNCE_BEGIN );
		m_sprButton[i].BeginTweening( 0.2f, TWEEN_BOUNCE_BEGIN );
		m_sprButton[i].SetTweenX( 400 );

		if( i == m_iSelection )
			m_screenLabels[i].BeginTweening( 1.0f, TWEEN_BOUNCE_BEGIN );
		else
			m_screenLabels[i].BeginTweening( 0.1f*i, TWEEN_BOUNCE_BEGIN );
		m_screenLabels[i].BeginTweening( 0.2f, TWEEN_BOUNCE_BEGIN );
		m_screenLabels[i].SetTweenX( 400 );
	}

}

