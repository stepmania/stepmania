#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenBookkeeping

 Desc: Where the player maps device input to pad input.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenBookkeeping.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "InputMapper.h"
#include "GameManager.h"
#include "GameState.h"
#include "RageSounds.h"
#include "ThemeManager.h"
#include "RageDisplay.h"
#include "Bookkeeper.h"


ScreenBookkeeping::ScreenBookkeeping( CString sClassName ) : Screen( sClassName )
{
	LOG->Trace( "ScreenBookkeeping::ScreenBookkeeping()" );
	
	m_textTitle.LoadFromFont( THEME->GetPathToF("Common title") );
	m_textTitle.SetText( "header" );
	m_textTitle.SetXY( CENTER_X, 60 );
	m_textTitle.SetDiffuse( RageColor(1,1,1,1) );
	m_textTitle.SetZoom( 0.8f );
	this->AddChild( &m_textTitle );

	for( int i=0; i<NUM_BOOKKEEPING_COLS; i++ )
	{
		float fX = SCALE( i, 0.f, NUM_BOOKKEEPING_COLS-1, SCREEN_LEFT+50, SCREEN_RIGHT-50 );
		float fY = CENTER_Y+16;
		m_textCols[i].LoadFromFont( THEME->GetPathToF("Common normal") );
		m_textCols[i].SetText( ssprintf("%d",i) );
		m_textCols[i].SetXY( fX, fY );
		m_textCols[i].SetDiffuse( RageColor(1,1,1,1) );
		m_textCols[i].SetZoom( 0.6f );
		this->AddChild( &m_textCols[i] );
	}

	m_Menu.Load( "ScreenBookkeeping" );
	this->AddChild( &m_Menu );

	ChangeView( (View)0 );

	SOUND->PlayMusic( THEME->GetPathToS("ScreenBookkeeping music") );
}

ScreenBookkeeping::~ScreenBookkeeping()
{
	LOG->Trace( "ScreenBookkeeping::~ScreenBookkeeping()" );
}

void ScreenBookkeeping::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenBookkeeping::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( type != IET_FIRST_PRESS && type != IET_SLOW_REPEAT )
		return;	// ignore

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default handler
}

void ScreenBookkeeping::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GoToNextScreen:
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( "ScreenOptionsMenu" );
		break;
	}
}

void ScreenBookkeeping::MenuLeft( PlayerNumber pn )
{
	m_View = (View)(m_View-1);
	CLAMP( (int&)m_View, 0, NUM_VIEWS-1 );
	ChangeView( m_View );
}

void ScreenBookkeeping::MenuRight( PlayerNumber pn )
{
	m_View = (View)(m_View+1);
	CLAMP( (int&)m_View, 0, NUM_VIEWS-1 );
	ChangeView( m_View );
}

void ScreenBookkeeping::MenuStart( PlayerNumber pn )
{
	if( !m_Menu.IsTransitioning() )
	{
		SOUND->PlayOnce( THEME->GetPathToS("Common start") );
		m_Menu.StartTransitioning( SM_GoToNextScreen );		
	}
}

void ScreenBookkeeping::MenuBack( PlayerNumber pn )
{
	if(!m_Menu.IsTransitioning())
	{
		SOUND->PlayOnce( THEME->GetPathToS("Common start") );
		m_Menu.StartTransitioning( SM_GoToPrevScreen );		
	}
}
	
void ScreenBookkeeping::ChangeView( View newView )
{
	m_View = newView;

	switch( m_View )
	{
	case VIEW_LAST_DAYS:
		{
			m_textTitle.SetText( ssprintf("Coin Data of Last %d days", NUM_LAST_DAYS) );

			int coins[NUM_LAST_DAYS];
			BOOKKEEPER->GetCoinsLastDays( coins );
			int iTotalLast = 0;

			CString sTitle, sData;
			for( int i=0; i<NUM_LAST_DAYS; i++ )
			{
				sTitle += LAST_DAYS_NAME[i] + "\n";
				sData += ssprintf("%d",coins[i]) + "\n";
				iTotalLast += coins[i];
			}

			sTitle += "\n";
			sData += "\n";

			sTitle += "Average\n";
			sData += ssprintf("%d\n",iTotalLast/NUM_LAST_DAYS);
			
			m_textCols[0].SetHorizAlign( Actor::align_left );
			m_textCols[0].SetText( sTitle );
			m_textCols[1].SetText( "" );
			m_textCols[2].SetText( "" );
			m_textCols[3].SetHorizAlign( Actor::align_right );
			m_textCols[3].SetText( sData );
		}
		break;
	case VIEW_LAST_WEEKS:
		{
			m_textTitle.SetText( ssprintf("Last %d weeks", NUM_LAST_WEEKS) );

			int coins[NUM_LAST_WEEKS];
			BOOKKEEPER->GetCoinsLastWeeks( coins );

			CString sTitle, sData;
			for( int col=0; col<4; col++ )
			{
				CString sTemp;
				for( int row=0; row<52/4; row++ )
				{
					int week = row*4+col;
					sTemp += ssprintf("%d ago: %d\n", week+1, coins[week]);
				}

				m_textCols[col].SetHorizAlign( Actor::align_left );
				m_textCols[col].SetText( sTemp );
			}
		}
		break;
	case VIEW_DAY_OF_WEEK:
		{
			m_textTitle.SetText( "Day of week" );

			int coins[DAYS_IN_WEEK];
			BOOKKEEPER->GetCoinsByDayOfWeek( coins );

			CString sTitle, sData;
			for( int i=0; i<DAYS_IN_WEEK; i++ )
			{
				sTitle += DAY_OF_WEEK_TO_NAME[i] + "\n";
				sData += ssprintf("%d",coins[i]) + "\n";
			}
			
			m_textCols[0].SetHorizAlign( Actor::align_left );
			m_textCols[0].SetText( sTitle );
			m_textCols[1].SetText( "" );
			m_textCols[2].SetText( "" );
			m_textCols[3].SetHorizAlign( Actor::align_right );
			m_textCols[3].SetText( sData );
		}
		break;
	case VIEW_HOUR_OF_DAY:
		{
			m_textTitle.SetText( "Hour of day" );

			int coins[HOURS_PER_DAY];
			BOOKKEEPER->GetCoinsByHour( coins );

			CString sTitle, sData;
			for( int i=0; i<HOURS_PER_DAY; i++ )
			{
				sTitle += HourToString(i) + "\n";
				sData += ssprintf("%d",coins[i]) + "\n";
			}
			
			m_textCols[0].SetHorizAlign( Actor::align_left );
			m_textCols[0].SetText( sTitle );
			m_textCols[1].SetText( "" );
			m_textCols[2].SetText( "" );
			m_textCols[3].SetHorizAlign( Actor::align_right );
			m_textCols[3].SetText( sData );
		}
		break;
	default:
		ASSERT(0);
	}
}
