#include "global.h"
#include "GroupList.h"
#include "ThemeManager.h"
#include "SongManager.h"
#include "RageLog.h"

/* If this actor is used anywhere other than SelectGroup, we
 * can add a setting that changes which metric group we pull
 * settings out of, so it can be configured separately. */
#define START_X				THEME->GetMetricF("GroupList","StartX")
#define START_Y				THEME->GetMetricF("GroupList","StartY")
#define SPACING_X			THEME->GetMetricF("GroupList","SpacingX")
#define SPACING_Y			THEME->GetMetricF("GroupList","SpacingY")
#define SCROLL_TWEEN_COMMAND THEME->GetMetric ("GroupList","ScrollTweenCommand")
#define GAIN_FOCUS_COMMAND	THEME->GetMetric ("GroupList","GainFocusCommand")
#define LOSE_FOCUS_COMMAND	THEME->GetMetric ("GroupList","LoseFocusCommand")
#define HIDE_ITEM_COMMAND	THEME->GetMetric ("GroupList","HideItemCommand")
#define SHOW_ITEM_COMMAND	THEME->GetMetric ("GroupList","ShowItemCommand")

const int MAX_GROUPS_ONSCREEN = 7;

GroupList::GroupList()
{
	m_iSelection = m_iTop = 0;
}

GroupList::~GroupList()
{
	for( unsigned i = 0; i < m_sprButtons.size(); ++i )
	{
		delete m_sprButtons[i];
		delete m_textLabels[i];
	}
}


bool GroupList::ItemIsOnScreen( int n ) const
{
	const int offset = n - m_iTop;
	return offset >= 0 && offset < MAX_GROUPS_ONSCREEN;
}

void GroupList::Load( const CStringArray& asGroupNames )
{
	m_asLabels = asGroupNames;

	this->AddChild( &m_Frame );

	for( unsigned i=0; i < m_asLabels.size(); i++ )
	{
		Sprite *button = new Sprite;
		BitmapText *label = new BitmapText;

		m_sprButtons.push_back( button );
		m_textLabels.push_back( label );

		button->Load( THEME->GetPathToG("GroupList bar") );
		label->LoadFromFont( THEME->GetPathToF("GroupList label") );
		label->SetShadowLength( 2 );
		label->SetText( SONGMAN->ShortenGroupName( asGroupNames[i] ) );
		
		m_Frame.AddChild( button );
		m_Frame.AddChild( label );

		button->SetXY( START_X + i*SPACING_X, START_Y + i*SPACING_Y );
		label->SetXY( START_X + i*SPACING_X, START_Y + i*SPACING_Y );

		if( i == 0 )
		{
			label->TurnRainbowOn();
		}
		else 
		{
			label->TurnRainbowOff();
			label->SetDiffuse( SONGMAN->GetGroupColor(asGroupNames[i]) );
		}

		m_bHidden.push_back( ItemIsOnScreen(i) );

		ResetTextSize( i );
	}

	AfterChange();
}

void GroupList::ResetTextSize( int i )
{
	BitmapText *label = m_textLabels[i];
	const float fTextWidth = (float)label->GetWidestLineWidthInSourcePixels();
	const float fButtonWidth = m_sprButtons[i]->GetZoomedWidth();

	float fZoom = fButtonWidth/fTextWidth;
	fZoom = min( fZoom, 0.8f );
	
	label->SetZoomX( fZoom );
	label->SetZoomY( 0.8f );
}

void GroupList::BeforeChange()
{
	m_sprButtons[m_iSelection]->Command( LOSE_FOCUS_COMMAND );
	m_textLabels[m_iSelection]->Command( LOSE_FOCUS_COMMAND );
}


void GroupList::AfterChange()
{
	m_Frame.StopTweening();
	m_Frame.Command( SCROLL_TWEEN_COMMAND );
	m_Frame.SetY( -m_iTop*SPACING_Y );

	for( int i=0; i < (int) m_asLabels.size(); i++ )
	{
		const bool IsHidden = !ItemIsOnScreen(i);
		const bool WasHidden = m_bHidden[i];

		if( IsHidden && !WasHidden )
		{
			m_sprButtons[i]->Command( HIDE_ITEM_COMMAND );
			m_textLabels[i]->Command( HIDE_ITEM_COMMAND );
		}
		else if( !IsHidden && WasHidden )
		{
			m_sprButtons[i]->Command( SHOW_ITEM_COMMAND );
			m_textLabels[i]->Command( SHOW_ITEM_COMMAND );
			ResetTextSize( i );
		}

		m_bHidden[i] = IsHidden;
	}

	m_sprButtons[m_iSelection]->Command( GAIN_FOCUS_COMMAND );
	m_textLabels[m_iSelection]->Command( GAIN_FOCUS_COMMAND );
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

	if( (int)m_asLabels.size() <= MAX_GROUPS_ONSCREEN ||
		  sel <= MAX_GROUPS_ONSCREEN/2 )
		m_iTop = 0;
	else if ( sel >= m_asLabels.size() - MAX_GROUPS_ONSCREEN/2 )
		m_iTop = m_asLabels.size() - MAX_GROUPS_ONSCREEN;
	else
		m_iTop = sel - MAX_GROUPS_ONSCREEN/2;

	/* The current selection must always be visible. */
	ASSERT( m_iTop <= m_iSelection );
	ASSERT( m_iTop+MAX_GROUPS_ONSCREEN > m_iSelection );

	AfterChange();
}

/* either tweened off screen (to come on screen),
   tweened off screen (invisible),
   onscreen (unselected),
   or onscreen (selected)

   coming on/ofscreen is internal
	always invis<->unselected<->selected
   */


void GroupList::TweenOnScreen()
{
	for( int i=0; i < (int) m_asLabels.size(); i++ )
	{
		const int offset = max(0, i-m_iTop);
		
		m_sprButtons[i]->SetX( 400 );
		m_textLabels[i]->SetX( 400 );

		m_sprButtons[i]->BeginTweening( 0.1f*offset, TWEEN_BOUNCE_END );
		m_textLabels[i]->BeginTweening( 0.1f*offset, TWEEN_BOUNCE_END );
		
		m_sprButtons[i]->BeginTweening( 0.2f, TWEEN_BOUNCE_END );
		m_textLabels[i]->BeginTweening( 0.2f, TWEEN_BOUNCE_END );
		
		m_sprButtons[i]->SetX( 0 );
		m_textLabels[i]->SetX( 0 );

		/* If this item isn't visible, hide it and skip tweens. */
		if( !ItemIsOnScreen(i) )
		{
			m_sprButtons[i]->Command( HIDE_ITEM_COMMAND );
			m_textLabels[i]->Command( HIDE_ITEM_COMMAND );
			
			m_sprButtons[i]->FinishTweening();
			m_textLabels[i]->FinishTweening();
		}
	}
}

void GroupList::TweenOffScreen()
{
	for( int i=m_iTop; i < min(m_iTop+MAX_GROUPS_ONSCREEN, (int) m_asLabels.size()); i++ )
	{
		const int offset = max(0, i-m_iTop);
		if( i == m_iSelection )
		{
			m_sprButtons[i]->BeginTweening( 1.0f, TWEEN_BOUNCE_BEGIN );
			m_textLabels[i]->BeginTweening( 1.0f, TWEEN_BOUNCE_BEGIN );
		}
		else
		{
			m_sprButtons[i]->BeginTweening( 0.1f*offset, TWEEN_BOUNCE_BEGIN );
			m_textLabels[i]->BeginTweening( 0.1f*offset, TWEEN_BOUNCE_BEGIN );
		}

		m_sprButtons[i]->BeginTweening( 0.2f, TWEEN_BOUNCE_BEGIN );
		m_textLabels[i]->BeginTweening( 0.2f, TWEEN_BOUNCE_BEGIN );

		m_sprButtons[i]->SetX( 400 );
		m_textLabels[i]->SetX( 400 );
	}
}

