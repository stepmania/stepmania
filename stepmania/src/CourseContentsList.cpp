#include "global.h"
#include "CourseContentsList.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "Course.h"
#include "SongManager.h"
#include "ThemeManager.h"
#include "Steps.h"
#include "GameState.h"
#include "Style.h"
#include "RageTexture.h"


CourseContentsList::CourseContentsList()
{
	m_iNumContents = 0;
	m_quad.SetDiffuse( RageColor(0,0,0,1) );
	m_quad.SetBlendMode( BLEND_NO_EFFECT );		// invisible, since we want to write only to the Zbuffer

	m_fTimeUntilScroll = 0;
	m_fItemAtTopOfList = 0;

	m_quad.SetUseZBuffer( true );
	for( int i = 0; i < MAX_TOTAL_CONTENTS; ++i )
	{
		m_CourseContentDisplays[i].SetName( "CourseEntryDisplay" );
		m_CourseContentDisplays[i].Load();
		m_CourseContentDisplays[i].SetUseZBuffer( true );
	}

	/* These are all the same; grab the dimensions. */
	ContentsBarHeight = m_CourseContentDisplays[0].GetUnzoomedHeight();
	ContentsBarWidth = m_CourseContentDisplays[0].GetUnzoomedWidth();
}


void CourseContentsList::SetFromGameState()
{
	Course* pCourse = GAMESTATE->m_pCurCourse;

	if( pCourse == NULL )
	{
		m_iNumContents = 0;
		return;
	}

	// FIXME: Is there a better way to handle when players don't have 
	// the same number of TrailEntries?
	// They have to have the same number, and of the same songs, or gameplay
	// isn't going to line up.
	Trail* pMasterTrail = GAMESTATE->m_pCurTrail[GAMESTATE->m_MasterPlayerNumber];
	if( pMasterTrail == NULL )
		return;
	int iNumEntriesToShow = min((int)pMasterTrail->m_vEntries.size(), MAX_TOTAL_CONTENTS);

	m_iNumContents = 0;
	
	for( int i=0; i<iNumEntriesToShow; i++ )
	{
		CourseEntryDisplay& display = m_CourseContentDisplays[m_iNumContents];

		const TrailEntry* pte[NUM_PLAYERS];
		ZERO( pte );

		FOREACH_EnabledPlayer(pn)
		{
			Trail* pTrail = GAMESTATE->m_pCurTrail[pn];
			if( pTrail == NULL )
				continue;	// skip
			if( unsigned(i) < pTrail->m_vEntries.size() )
				pte[pn] = &pTrail->m_vEntries[i];
		}
		display.LoadFromTrailEntry( m_iNumContents+1, pte );
		
		m_iNumContents++;
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

	RectF rectBarSize(-ContentsBarWidth/2, -ContentsBarHeight/2,
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

/*
 * (c) 2001-2004 Chris Danford
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
