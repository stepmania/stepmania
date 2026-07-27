#include "global.h"
#include "ScreenBookkeeping.h"
#include "ScreenManager.h"
#include "RageLog.h"
#include "ThemeManager.h"
#include "Bookkeeper.h"
#include "ScreenDimensions.h"
#include "InputEventPlus.h"
#include "RageUtil.h"
#include "LocalizedString.h"
#include "Song.h"
#include "SongManager.h"
#include "UnlockManager.h"
#include "ProfileManager.h"
#include "Profile.h"

#include <vector>


static const char *BookkeepingViewNames[] = {
	"SongPlays",
	"LastDays",
	"LastWeeks",
	"DayOfWeek",
	"HourOfDay",
};
XToString( BookkeepingView );


REGISTER_SCREEN_CLASS( ScreenBookkeeping );

void ScreenBookkeeping::Init()
{
	ScreenWithMenuElements::Init();

	m_textAllTime.LoadFromFont( THEME->GetPathF(m_sName,"AllTime") );
	m_textAllTime.SetName( "AllTime" );
	LOAD_ALL_COMMANDS_AND_SET_XY( m_textAllTime );
	this->AddChild( &m_textAllTime );

	m_textTitle.LoadFromFont( THEME->GetPathF(m_sName,"title") );
	m_textTitle.SetName( "Title" );
	LOAD_ALL_COMMANDS_AND_SET_XY( m_textTitle );
	this->AddChild( &m_textTitle );

	for( int i=0; i<NUM_BOOKKEEPING_COLS; i++ )
	{
		m_textData[i].LoadFromFont( THEME->GetPathF(m_sName,"data") );
		m_textData[i].SetName( "Data" );
		LOAD_ALL_COMMANDS_AND_SET_XY( m_textData[i] );
		float fX = SCALE( i, 0.f, NUM_BOOKKEEPING_COLS-1, SCREEN_LEFT+50, SCREEN_RIGHT-160 );
		m_textData[i].SetX( fX );
		this->AddChild( &m_textData[i] );
	}

	FOREACH_ENUM( BookkeepingView, i )
	{
		if( THEME->GetMetricB(m_sName,"Show"+BookkeepingViewToString(i)) )
			m_vBookkeepingViews.push_back( i );
	}

	m_iViewIndex = 0;

	UpdateView();
}

void ScreenBookkeeping::Update( float fDelta )
{
	UpdateView();	// refresh so that counts change in real-time

	ScreenWithMenuElements::Update( fDelta );
}

bool ScreenBookkeeping::Input( const InputEventPlus &input )
{
	if( input.type != IET_FIRST_PRESS && input.type != IET_REPEAT )
		return false;	// ignore

	return Screen::Input( input );	// default handler
}

bool ScreenBookkeeping::MenuLeft( const InputEventPlus &input )
{
	m_iViewIndex--;
	CLAMP( m_iViewIndex, 0, m_vBookkeepingViews.size()-1 );

	UpdateView();
	return true;
}

bool ScreenBookkeeping::MenuRight( const InputEventPlus &input )
{
	m_iViewIndex++;
	CLAMP( m_iViewIndex, 0, m_vBookkeepingViews.size()-1 );

	UpdateView();
	return true;
}

bool ScreenBookkeeping::MenuStart( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return false;

	SCREENMAN->PlayStartSound();
	StartTransitioningScreen( SM_GoToNextScreen );
	return true;
}

bool ScreenBookkeeping::MenuBack( const InputEventPlus &input )
{
	if( IsTransitioning() )
		return false;

	SCREENMAN->PlayStartSound();
	StartTransitioningScreen( SM_GoToPrevScreen );
	return true;
}

bool ScreenBookkeeping::MenuCoin( const InputEventPlus &input )
{
	UpdateView();

	return Screen::MenuCoin( input );
}

static LocalizedString ALL_TIME		( "ScreenBookkeeping", "All-time Coin Total:" );
static LocalizedString SONG_PLAYS	( "ScreenBookkeeping", "Total Song Plays: %d" );
static LocalizedString LAST_DAYS	( "ScreenBookkeeping", "Coin Data of Last %d Days" );
static LocalizedString LAST_WEEKS	( "ScreenBookkeeping", "Coin Data of Last %d Weeks" );
static LocalizedString DAY_OF_WEEK	( "ScreenBookkeeping", "Coin Data by Day of Week, All-Time" );
static LocalizedString HOUR_OF_DAY	( "ScreenBookkeeping", "Coin Data by Hour of Day, All-Time" );
void ScreenBookkeeping::UpdateView()
{
	BookkeepingView view = m_vBookkeepingViews[m_iViewIndex];


	{
		RString s;
		s += ALL_TIME.GetValue();
		s += ssprintf( " %i\n", BOOKKEEPER->GetCoinsTotal() );
		m_textAllTime.SetText( s );
	}

	switch( view )
	{
	case BookkeepingView_SongPlays:
		{
			Profile *pProfile = PROFILEMAN->GetMachineProfile();

			std::vector<Song*> vpSongs;
			int iCount = 0;
			for (Song *pSong : SONGMAN->GetAllSongs())
			{
				if( UNLOCKMAN->SongIsLocked(pSong) & ~LOCKED_DISABLED )
					continue;
				iCount += pProfile->GetSongNumTimesPlayed( pSong );
				vpSongs.push_back( pSong );
			}
			m_textTitle.SetText( ssprintf(SONG_PLAYS.GetValue(), iCount) );
			SongUtil::SortSongPointerArrayByNumPlays( vpSongs, pProfile, true );

			const int iSongPerCol = 15;

			int iSongIndex = 0;
			for( int i=0; i<NUM_BOOKKEEPING_COLS; i++ )
			{
				RString s;
				for( int j=0; j<iSongPerCol; j++ )
				{
					if( iSongIndex < (int)vpSongs.size() )
					{
						Song *pSong = vpSongs[iSongIndex];
						iCount = pProfile->GetSongNumTimesPlayed( pSong );
						RString sTitle = ssprintf("%4d",iCount) + " " + pSong->GetDisplayFullTitle();
						if( sTitle.length() > 22 )
							sTitle = sTitle.Left(20) + "...";
						s += sTitle + "\n";
						iSongIndex++;
					}
				}
				m_textData[i].SetText( s );
				m_textData[i].SetHorizAlign( align_left );
			}
		}
		break;
	case BookkeepingView_LastDays:
		{
			m_textTitle.SetText( ssprintf(LAST_DAYS.GetValue(), NUM_LAST_DAYS) );

			int coins[NUM_LAST_DAYS];
			BOOKKEEPER->GetCoinsLastDays( coins );
			int iTotalLast = 0;

			RString sTitle, sData;
			for( int i=0; i<NUM_LAST_DAYS; i++ )
			{
				sTitle += LastDayToLocalizedString(i) + "\n";
				sData += ssprintf("%d",coins[i]) + "\n";
				iTotalLast += coins[i];
			}

			sTitle += ALL_TIME.GetValue()+"\n";
			sData += ssprintf("%i\n", iTotalLast);

			m_textData[0].SetText( "" );
			m_textData[1].SetHorizAlign( align_left );
			m_textData[1].SetText( sTitle );
			m_textData[2].SetText( "" );
			m_textData[3].SetHorizAlign( align_right );
			m_textData[3].SetText( sData );
		}
		break;
	case BookkeepingView_LastWeeks:
		{
			m_textTitle.SetText( ssprintf(LAST_WEEKS.GetValue(), NUM_LAST_WEEKS) );

			int coins[NUM_LAST_WEEKS];
			BOOKKEEPER->GetCoinsLastWeeks( coins );

			RString sTitle, sData;
			for( int col=0; col<4; col++ )
			{
				RString sTemp;
				for( int row=0; row<52/4; row++ )
				{
					int week = row*4+col;
					sTemp += LastWeekToLocalizedString(week) + ssprintf(": %d",coins[week]) + "\n";
				}

				m_textData[col].SetHorizAlign( align_left );
				m_textData[col].SetText( sTemp );
			}
		}
		break;
	case BookkeepingView_DayOfWeek:
		{
			m_textTitle.SetText( DAY_OF_WEEK );

			int coins[DAYS_IN_WEEK];
			BOOKKEEPER->GetCoinsByDayOfWeek( coins );

			RString sTitle, sData;
			for( int i=0; i<DAYS_IN_WEEK; i++ )
			{
				sTitle += DayOfWeekToString(i) + "\n";
				sData += ssprintf("%d",coins[i]) + "\n";
			}

			m_textData[0].SetText( "" );
			m_textData[1].SetHorizAlign( align_left );
			m_textData[1].SetText( sTitle );
			m_textData[2].SetText( "" );
			m_textData[3].SetHorizAlign( align_right );
			m_textData[3].SetText( sData );
		}
		break;
	case BookkeepingView_HourOfDay:
		{
			m_textTitle.SetText( HOUR_OF_DAY );

			int coins[HOURS_IN_DAY];
			BOOKKEEPER->GetCoinsByHour( coins );

			RString sTitle1, sData1;
			for( int i=0; i<HOURS_IN_DAY/2; i++ )
			{
				sTitle1 += HourInDayToLocalizedString(i) + "\n";
				sData1 += ssprintf("%d",coins[i]) + "\n";
			}

			RString sTitle2, sData2;
			for( int i=(HOURS_IN_DAY/2); i<HOURS_IN_DAY; i++ )
			{
				sTitle2 += HourInDayToLocalizedString(i) + "\n";
				sData2 += ssprintf("%d",coins[i]) + "\n";
			}

			m_textData[0].SetHorizAlign( align_left );
			m_textData[0].SetText( sTitle1 );
			m_textData[1].SetHorizAlign( align_right );
			m_textData[1].SetText( sData1 );
			m_textData[2].SetHorizAlign( align_left );
			m_textData[2].SetText( sTitle2 );
			m_textData[3].SetHorizAlign( align_right );
			m_textData[3].SetText( sData2 );
		}
		break;
	default:
		FAIL_M(ssprintf("Invalid BookkeepingView: %i", view));
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
