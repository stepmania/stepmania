#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: CourseContentsFrame

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "CourseContentsFrame.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "Course.h"
#include "SongManager.h"
#include <math.h>
#include "RageDisplay.h"
#include "ThemeManager.h"
#include "Notes.h"
#include "GameState.h"
#include "StyleDef.h"


#define TEXT_BANNER_X	THEME->GetMetricF("CourseContentDisplay","TextBannerX")
#define TEXT_BANNER_Y	THEME->GetMetricF("CourseContentDisplay","TextBannerY")
#define NUMBER_X		THEME->GetMetricF("CourseContentDisplay","NumberX")
#define NUMBER_Y		THEME->GetMetricF("CourseContentDisplay","NumberY")
#define FOOT_X			THEME->GetMetricF("CourseContentDisplay","FootX")
#define FOOT_Y			THEME->GetMetricF("CourseContentDisplay","FootY")
#define DIFFICULTY_X	THEME->GetMetricF("CourseContentDisplay","DifficultyX")
#define DIFFICULTY_Y	THEME->GetMetricF("CourseContentDisplay","DifficultyY")


CourseContentDisplay::CourseContentDisplay()
{
	m_sprFrame.Load( THEME->GetPathTo("Graphics","select course content bar") );
	this->AddChild( &m_sprFrame );

	m_textNumber.LoadFromFont( THEME->GetPathTo("Fonts","Header2") );
	m_textNumber.SetXY( NUMBER_X, NUMBER_Y );
	m_textNumber.TurnShadowOff();
	this->AddChild( &m_textNumber );

	m_TextBanner.SetXY( TEXT_BANNER_X, TEXT_BANNER_Y );
	this->AddChild( &m_TextBanner );

	m_textFoot.LoadFromTextureAndChars( THEME->GetPathTo("Graphics","select music meter 2x1"),"10" );
	m_textFoot.SetXY( FOOT_X, FOOT_Y );
	m_textFoot.TurnShadowOff();
	this->AddChild( &m_textFoot );

	m_textDifficultyNumber.LoadFromFont( THEME->GetPathTo("Fonts","normal") );
	m_textDifficultyNumber.SetXY( DIFFICULTY_X, DIFFICULTY_Y );
	m_textDifficultyNumber.SetZoom( 0.8f );
	m_textDifficultyNumber.TurnShadowOff();
	this->AddChild( &m_textDifficultyNumber );
}


void CourseContentDisplay::LoadFromSongAndNotes( int iNum, Song* pSong, Notes* pNotes )
{
	m_textNumber.SetText( ssprintf("%d", iNum) );

	RageColor colorGroup = SONGMAN->GetSongColor( pSong );
	RageColor colorNotes = SONGMAN->GetDifficultyColor( pNotes->GetDifficulty() );

	m_TextBanner.LoadFromSong( pSong );
	m_TextBanner.SetDiffuse( colorGroup );

	m_textFoot.SetText( "1" );
	m_textFoot.SetDiffuse( colorNotes );

	m_textDifficultyNumber.SetText( ssprintf("%d", pNotes->GetMeter()) );
	m_textDifficultyNumber.SetDiffuse( colorNotes );
}

void CourseContentDisplay::LoadFromDifficulty( int iNum, Difficulty dc )
{
	m_textNumber.SetText( ssprintf("%d", iNum) );

	RageColor colorGroup = RageColor(1,1,1,1);	// TODO: What should this be?
	RageColor colorNotes = SONGMAN->GetDifficultyColor( dc );

	m_TextBanner.LoadFromString( "??????????", "??????????", "", "", "", "" );
	m_TextBanner.SetDiffuse( colorGroup );

	m_textFoot.SetText( "1" );
	m_textFoot.SetDiffuse( colorNotes );

	m_textDifficultyNumber.SetText( "?" );
	m_textDifficultyNumber.SetDiffuse( colorNotes );
}

void CourseContentDisplay::LoadFromMeterRange( int iNum, int iLow, int iHigh )
{
	m_textNumber.SetText( ssprintf("%d", iNum) );

	RageColor colorGroup = RageColor(1,1,1,1);	// TODO: What should this be?
	RageColor colorNotes = RageColor(1,1,1,1);

	m_TextBanner.LoadFromString( "??????????", "??????????", "", "", "", "" );
	m_TextBanner.SetDiffuse( colorGroup );

	m_textFoot.SetText( "1" );
	m_textFoot.SetDiffuse( colorNotes );

	m_textDifficultyNumber.SetText( ssprintf("%d-%d", iLow, iHigh) );
	m_textDifficultyNumber.SetDiffuse( colorNotes );
}

CourseContentsFrame::CourseContentsFrame()
{
	m_iNumContents = 0;
	m_quad.SetDiffuse( RageColor(0,0,0,0) );	// invisible, since we want to write only to the Zbuffer

	m_fTimeUntilScroll = 0;
	m_fItemAtTopOfList = 0;

	/* These are all the same; grab the dimensions. */
	ContentsBarHeight = m_CourseContentDisplays[0].m_sprFrame.GetTexture()->GetSourceFrameHeight();
	ContentsBarWidth = m_CourseContentDisplays[0].m_sprFrame.GetTexture()->GetSourceFrameWidth();
}

void CourseContentsFrame::SetFromCourse( Course* pCourse )
{
	ASSERT( pCourse != NULL );

	m_fTimeUntilScroll = 3;
	m_fItemAtTopOfList = 0;

	m_iNumContents = 0; 

	vector<Song*> vSongs;
	vector<Notes*> vNotes;
	vector<CString> vsModifiers;
	pCourse->GetStageInfo( vSongs, vNotes, vsModifiers, GAMESTATE->GetCurrentStyleDef()->m_NotesType, false );

	for( int i=0; i<min((int)vSongs.size(), MAX_TOTAL_CONTENTS); i++ )
	{
		CourseContentDisplay& display = m_CourseContentDisplays[m_iNumContents];
	
		if( pCourse->IsMysterySong(i) )
		{
			Difficulty dc = pCourse->GetDifficulty(i);
			int iMeterLow, iMeterHigh;
			pCourse->GetMeterRange(i, iMeterLow, iMeterHigh);

			if( dc == DIFFICULTY_INVALID )
				display.LoadFromMeterRange( m_iNumContents+1, iMeterLow, iMeterHigh );
			else
				display.LoadFromDifficulty( m_iNumContents+1, pCourse->GetDifficulty(i) );
		}
		else
		{
			Song* pSong = vSongs[i];
			Notes* pNotes = vNotes[i];
			CString sModifiers = vsModifiers[i];
			display.LoadFromSongAndNotes( m_iNumContents+1, pSong, pNotes );
		}

		display.SetXY( 0, -((MAX_VISIBLE_CONTENTS-1)/2) * float(ContentsBarHeight) );
		display.StopTweening();
		display.BeginTweening( m_iNumContents*0.1f );
		display.SetTweenY( (-(MAX_VISIBLE_CONTENTS-1)/2 + m_iNumContents) * float(ContentsBarHeight) );
		
		m_iNumContents ++;
	}
}

void CourseContentsFrame::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	if( m_fTimeUntilScroll > 0  &&  m_iNumContents > MAX_VISIBLE_CONTENTS)
		m_fTimeUntilScroll -= fDeltaTime;
	if( m_fTimeUntilScroll <= 0 ) {
		m_fItemAtTopOfList += fDeltaTime;
		m_fItemAtTopOfList = fmodf(m_fItemAtTopOfList, float(m_iNumContents));
	}

	for( int i=0; i<m_iNumContents; i++ )
		m_CourseContentDisplays[i].Update( fDeltaTime );
}

void CourseContentsFrame::DrawPrimitives()
{
	// turn on Z buffer to clip items
	DISPLAY->EnableZBuffer();

	// write to z buffer so that top and bottom are clipped
	m_quad.SetZ( 1 );

	RectI rectBarSize(-ContentsBarWidth/2, -ContentsBarHeight/2,
						ContentsBarWidth/2, ContentsBarHeight/2);
	m_quad.StretchTo( rectBarSize );

	m_quad.SetY( (-(MAX_VISIBLE_CONTENTS-1)/2 - 1) * float(ContentsBarHeight) );
	m_quad.Draw();

	m_quad.SetY( ((MAX_VISIBLE_CONTENTS-1)/2 + 1) * float(ContentsBarHeight) );
	m_quad.Draw();

	int iItemToDraw = (int)m_fItemAtTopOfList;

	// HACK:  Insert a little pause as a new item appears on the screen
	float fRemainder = m_fItemAtTopOfList - (int)m_fItemAtTopOfList;
	fRemainder = min( fRemainder*1.5f, 1 );

	const float fY = (-fRemainder-(MAX_VISIBLE_CONTENTS-1)/2) * ContentsBarHeight;

	for( int i=0; i<min(MAX_VISIBLE_CONTENTS+1, m_iNumContents); i++ )
	{
		if( m_fTimeUntilScroll <= 0 )
			m_CourseContentDisplays[iItemToDraw].SetY( fY + i*ContentsBarHeight);
		m_CourseContentDisplays[iItemToDraw].Draw();
		iItemToDraw = (iItemToDraw+1) % m_iNumContents;
	}

	// turn off Z buffer
	DISPLAY->DisableZBuffer();
}
