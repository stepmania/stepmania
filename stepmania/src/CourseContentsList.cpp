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
#include "ThemeManager.h"
#include "Steps.h"
#include "GameState.h"
#include "StyleDef.h"
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

	Trail* pTrail[NUM_PLAYERS];
	FOREACH_PlayerNumber(pn)
	{
		pTrail[pn] = GAMESTATE->m_pCurTrail[pn];
	}

	// FIXME: Is there a better way to handle when players don't have 
	// the same number of TrailEntries?
	// They have to have the same number, and of the same songs, or gameplay
	// isn't going to line up.
	Trail* pMasterTrail = pTrail[GAMESTATE->m_MasterPlayerNumber];
	int iNumEntriesToShow = min((int)pMasterTrail->m_vEntries.size(), MAX_TOTAL_CONTENTS);

	m_iNumContents = 0;
	
	for( int i=0; i<iNumEntriesToShow; i++ )
	{
		CourseEntryDisplay& display = m_CourseContentDisplays[m_iNumContents];
	
		const TrailEntry* pte[NUM_PLAYERS];
		FOREACH_EnabledPlayer(pn)
			pte[pn] = &pTrail[pn]->m_vEntries[i];
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
