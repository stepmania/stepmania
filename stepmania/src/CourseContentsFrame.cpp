#include "stdafx.h"
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


const float TEXT_BANNER_X	= 0;
const float TEXT_BANNER_Y	= 0;

const float NUMBER_X		= -118;
const float NUMBER_Y		= 0;

const float FOOT_X			= 102;
const float FOOT_Y			= 8;

const float DIFFICULTY_X	= FOOT_X+18;
const float DIFFICULTY_Y	= FOOT_Y;

const float CONTENTS_BAR_WIDTH	= 270;
const float CONTENTS_BAR_HEIGHT = 44;


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

void CourseContentDisplay::Load( int iNum, Song* pSong, Notes* pNotes )
{
	m_textNumber.SetText( ssprintf("%d", iNum) );

	RageColor colorGroup = SONGMAN->GetSongColor( pSong );
	RageColor colorNotes = pNotes->GetColor();

	m_TextBanner.LoadFromSong( pSong );
	m_TextBanner.SetDiffuse( colorGroup );

	m_textFoot.SetText( "1" );
	m_textFoot.SetDiffuse( colorNotes );

	m_textDifficultyNumber.SetText( ssprintf("%d", pNotes->m_iMeter) );
	m_textDifficultyNumber.SetDiffuse( colorNotes );
}



CourseContentsFrame::CourseContentsFrame()
{
	m_iNumContents = 0;
	m_quad.SetDiffuse( RageColor(0,0,0,0) );	// invisible, since we want to write only to the Zbuffer

	m_fTimeUntilScroll = 0;
	m_fItemAtTopOfList = 0;
}

void CourseContentsFrame::SetFromCourse( Course* pCourse )
{
	ASSERT( pCourse != NULL );

	m_fTimeUntilScroll = 3;
	m_fItemAtTopOfList = 0;

	m_iNumContents = 0; 

	for( int i=0; i<min(pCourse->m_iStages, MAX_TOTAL_CONTENTS); i++ )
	{
		Song* pSong = pCourse->m_apSongs[i];
		Notes* pNotes = pCourse->GetNotesForStage(i);

		if( pNotes == NULL )
			continue;	// skip

		printf( "Adding song '%s'\n", pSong->m_sMainTitle.GetString() );
		m_CourseContentDisplays[m_iNumContents].Load( m_iNumContents+1, pSong, pNotes );
		m_CourseContentDisplays[m_iNumContents].SetXY( 0, -((MAX_VISIBLE_CONTENTS-1)/2) * CONTENTS_BAR_HEIGHT );
		m_CourseContentDisplays[m_iNumContents].StopTweening();
		m_CourseContentDisplays[m_iNumContents].BeginTweening( m_iNumContents*0.1f );
		m_CourseContentDisplays[m_iNumContents].SetTweenY( (-(MAX_VISIBLE_CONTENTS-1)/2 + m_iNumContents) * CONTENTS_BAR_HEIGHT );
		
		m_iNumContents ++;
	}
	printf( "m_iNumContents is %d\n", m_iNumContents );
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

	RectI rectBarSize(-(int)CONTENTS_BAR_WIDTH/2, -(int)CONTENTS_BAR_HEIGHT/2, (int)CONTENTS_BAR_WIDTH/2, (int)CONTENTS_BAR_HEIGHT/2);
	m_quad.StretchTo( rectBarSize );

	m_quad.SetY( (-(MAX_VISIBLE_CONTENTS-1)/2 - 1) * CONTENTS_BAR_HEIGHT );
	m_quad.Draw();

	m_quad.SetY( ((MAX_VISIBLE_CONTENTS-1)/2 + 1) * CONTENTS_BAR_HEIGHT );
	m_quad.Draw();


	int iItemToDraw = (int)m_fItemAtTopOfList;

	// HACK:  Insert a little pause as a new item appears on the screen
	float fRemainder = m_fItemAtTopOfList - (int)m_fItemAtTopOfList;
	fRemainder = min( fRemainder*1.5f, 1 );

	const float fY = (-fRemainder-(MAX_VISIBLE_CONTENTS-1)/2) * CONTENTS_BAR_HEIGHT;

	for( int i=0; i<min(MAX_VISIBLE_CONTENTS+1, m_iNumContents); i++ )
	{
		if( m_fTimeUntilScroll <= 0 )
			m_CourseContentDisplays[iItemToDraw].SetY( fY + i*CONTENTS_BAR_HEIGHT);
		m_CourseContentDisplays[iItemToDraw].Draw();
		iItemToDraw = (iItemToDraw+1) % m_iNumContents;
	}

	// turn off Z buffer
	DISPLAY->DisableZBuffer();
}
