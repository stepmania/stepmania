#include "global.h"
#include "ScreenBookkeeping.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "InputMapper.h"
#include "GameManager.h"
#include "GameState.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "RageDisplay.h"
#include "Bookkeeper.h"
#include "ScreenDimensions.h"


REGISTER_SCREEN_CLASS( ScreenBookkeeping );
ScreenBookkeeping::ScreenBookkeeping( CString sClassName ) : ScreenWithMenuElements( sClassName )
{
	LOG->Trace( "ScreenBookkeeping::ScreenBookkeeping()" );
}

void ScreenBookkeeping::Init()
{
	ScreenWithMenuElements::Init();

	m_textTitle.LoadFromFont( THEME->GetPathF("Common","title") );
	m_textTitle.SetText( "header" );
	m_textTitle.SetXY( SCREEN_CENTER_X, 60 );
	m_textTitle.SetZoom( 0.8f );
	this->AddChild( &m_textTitle );

	for( int i=0; i<NUM_BOOKKEEPING_COLS; i++ )
	{
		float fX = SCALE( i, 0.f, NUM_BOOKKEEPING_COLS-1, SCREEN_LEFT+50, SCREEN_RIGHT-160 );
		float fY = SCREEN_CENTER_Y+16;
		m_textCols[i].LoadFromFont( THEME->GetPathF("Common","normal") );
		m_textCols[i].SetText( ssprintf("%d",i) );
		m_textCols[i].SetXY( fX, fY );
		m_textCols[i].SetZoom( 0.6f );
		this->AddChild( &m_textCols[i] );
	}

	ChangeView( (View)0 );
}

ScreenBookkeeping::~ScreenBookkeeping()
{
	LOG->Trace( "ScreenBookkeeping::~ScreenBookkeeping()" );
}

void ScreenBookkeeping::DrawPrimitives()
{
	Screen::DrawPrimitives();
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
	if( !IsTransitioning() )
	{
		SCREENMAN->PlayStartSound();
		StartTransitioning( SM_GoToNextScreen );		
	}
}

void ScreenBookkeeping::MenuBack( PlayerNumber pn )
{
	if(!IsTransitioning())
	{
		SCREENMAN->PlayStartSound();
		StartTransitioning( SM_GoToPrevScreen );		
	}
}

void ScreenBookkeeping::MenuCoin( PlayerNumber pn )
{
	ChangeView( m_View );

	Screen::MenuCoin( pn );
}

void ScreenBookkeeping::ChangeView( View newView )
{
	m_View = newView;

	switch( m_View )
	{
	case VIEW_LAST_DAYS:
		{
			m_textTitle.SetText( ssprintf("Coin Data of Last %d Days", NUM_LAST_DAYS) );

			int coins[NUM_LAST_DAYS];
			BOOKKEEPER->GetCoinsLastDays( coins );
			int iTotalLast = 0;

			CString sTitle, sData;
			for( int i=0; i<NUM_LAST_DAYS; i++ )
			{
				sTitle += LastDayToString(i) + "\n";
				sData += ssprintf("%d",coins[i]) + "\n";
				iTotalLast += coins[i];
			}

			sTitle += "\n";
			sData += "\n";

			sTitle += "Total This Week\n";
			sData += ssprintf("%i\n", iTotalLast);

			sTitle += "Daily Average\n";
			float fAverage = iTotalLast/(float)NUM_LAST_DAYS;
			sData += ssprintf("%.1f\n",fAverage);
			
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
			m_textTitle.SetText( ssprintf("Coin Data of Last %d Weeks", NUM_LAST_WEEKS) );

			int coins[NUM_LAST_WEEKS];
			BOOKKEEPER->GetCoinsLastWeeks( coins );

			CString sTitle, sData;
			for( int col=0; col<4; col++ )
			{
				CString sTemp;
				for( int row=0; row<52/4; row++ )
				{
					int week = row*4+col;
					sTemp += LastWeekToString(week) + ssprintf(": %d",coins[week]) + "\n";
				}

				m_textCols[col].SetHorizAlign( Actor::align_left );
				m_textCols[col].SetText( sTemp );
			}
		}
		break;
	case VIEW_DAY_OF_WEEK:
		{
			m_textTitle.SetText( "Coin Data by Day of Week" );

			int coins[DAYS_IN_WEEK];
			BOOKKEEPER->GetCoinsByDayOfWeek( coins );

			CString sTitle, sData;
			for( int i=0; i<DAYS_IN_WEEK; i++ )
			{
				sTitle += DayOfWeekToString(i) + "\n";
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
			m_textTitle.SetText( "Coin Data by Hour of Day" );

			int coins[HOURS_IN_DAY];
			BOOKKEEPER->GetCoinsByHour( coins );

			CString sTitle1, sData1;
			for( int i=0; i<HOURS_IN_DAY/2; i++ )
			{
				sTitle1 += HourInDayToString(i) + "\n";
				sData1 += ssprintf("%d",coins[i]) + "\n";
			}
			
			CString sTitle2, sData2;
			for( int i=(HOURS_IN_DAY/2); i<HOURS_IN_DAY; i++ )
			{
				sTitle2 += HourInDayToString(i) + "\n";
				sData2 += ssprintf("%d",coins[i]) + "\n";
			}
			
			m_textCols[0].SetHorizAlign( Actor::align_left );
			m_textCols[0].SetText( sTitle1 );
			m_textCols[1].SetHorizAlign( Actor::align_right );
			m_textCols[1].SetText( sData1 );
			m_textCols[2].SetHorizAlign( Actor::align_left );
			m_textCols[2].SetText( sTitle2 );
			m_textCols[3].SetHorizAlign( Actor::align_right );
			m_textCols[3].SetText( sData2 );
		}
		break;
	default:
		ASSERT(0);
	}
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
