#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: CourseContentsList

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "CourseContentsList.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "Course.h"
#include "SongManager.h"
#include <math.h>
#include "ThemeManager.h"
#include "Notes.h"
#include "GameState.h"
#include "StyleDef.h"


CourseContentsList::CourseContentsList()
{
	m_iNumContents = 0;
	m_quad.SetDiffuse( RageColor(0,0,0,1) );
	m_quad.SetBlendMode( BLEND_NO_EFFECT );		// invisible, since we want to write only to the Zbuffer

	m_fTimeUntilScroll = 0;
	m_fItemAtTopOfList = 0;

	m_quad.SetUseZBuffer( true );
	for( int i = 0; i < MAX_TOTAL_CONTENTS; ++i )
		m_CourseContentDisplays[i].SetUseZBuffer( true );

	/* These are all the same; grab the dimensions. */
	ContentsBarHeight = m_CourseContentDisplays[0].m_sprFrame.GetTexture()->GetSourceFrameHeight();
	ContentsBarWidth = m_CourseContentDisplays[0].m_sprFrame.GetTexture()->GetSourceFrameWidth();
}


void CourseContentsList::SetFromCourse( Course* pCourse )
{
	if( pCourse == NULL )
	{
		m_iNumContents = 0;
		return;
	}

	m_iNumContents = 0; 

	vector<Course::Info> ci;
	pCourse->GetCourseInfo( GAMESTATE->GetCurrentStyleDef()->m_NotesType, ci );

	for( int i=0; i<min((int)ci.size(), MAX_TOTAL_CONTENTS); i++ )
	{
		CourseEntryDisplay& display = m_CourseContentDisplays[m_iNumContents];
	
		if( ci[i].Random )
		{
			Difficulty dc = pCourse->GetDifficulty( ci[i] );
			int iMeterLow, iMeterHigh;
			pCourse->GetMeterRange(ci[i], iMeterLow, iMeterHigh);

			if( dc == DIFFICULTY_INVALID )
				display.LoadFromMeterRange( m_iNumContents+1, iMeterLow, iMeterHigh, ci[i].Modifiers );
			else
				display.LoadFromDifficulty( m_iNumContents+1, dc, ci[i].Modifiers );
		}
		else
		{
			display.LoadFromSongAndNotes( m_iNumContents+1, ci[i].Song, ci[i].Notes, ci[i].Modifiers );
		}
		
		m_iNumContents ++;
	}
}

void CourseContentsList::Update( float fDeltaTime )
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

void CourseContentsList::DrawPrimitives()
{
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
}

void CourseContentsList::TweenInAfterChangedCourse()
{
	m_fItemAtTopOfList = 0;
	m_fTimeUntilScroll = 3;

	for( int i=0; i<m_iNumContents; i++ )
	{
		CourseEntryDisplay& display = m_CourseContentDisplays[i];

		display.StopTweening();
		display.SetXY( 0, -((MAX_VISIBLE_CONTENTS-1)/2) * float(ContentsBarHeight) );
		display.BeginTweening( i*0.1f );
		display.SetY( (-(MAX_VISIBLE_CONTENTS-1)/2 + i) * float(ContentsBarHeight) );
	}
}
