#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: CourseEntryDisplay

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "CourseEntryDisplay.h"
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
#include "ActorUtil.h"
#include <math.h>

#define SEPARATE_COURSE_METERS		THEME->GetMetricB(m_sName,"SeparateCourseMeters")
#define TEXT_BANNER_NAME			THEME->GetMetric (m_sName,"TextBannerName")

void CourseEntryDisplay::Load()
{
	m_sprFrame.SetName( "Bar" );
	m_sprFrame.Load( THEME->GetPathToG("CourseEntryDisplay bar") );
	SET_XY_AND_ON_COMMAND( &m_sprFrame );
	this->AddChild( &m_sprFrame );

	this->m_size.x = (float) m_sprFrame.GetTexture()->GetSourceFrameWidth();
	this->m_size.y = (float) m_sprFrame.GetTexture()->GetSourceFrameHeight();

	m_textNumber.SetName( "Number" );
	m_textNumber.LoadFromFont( THEME->GetPathToF("CourseEntryDisplay number") );
	SET_XY_AND_ON_COMMAND( &m_textNumber );
	this->AddChild( &m_textNumber );

	m_TextBanner.SetName( TEXT_BANNER_NAME, "TextBanner" );
	SET_XY_AND_ON_COMMAND( &m_TextBanner );
	this->AddChild( &m_TextBanner );

	for( int pn = 0; pn < NUM_PLAYERS; ++pn )
	{
		if( !GAMESTATE->IsHumanPlayer((PlayerNumber)pn) )
			continue;	// skip

		if( !SEPARATE_COURSE_METERS && pn != GAMESTATE->m_MasterPlayerNumber )
			continue;	// skip

		m_textFoot[pn].SetName( SEPARATE_COURSE_METERS? ssprintf("FootP%i", pn+1):"Foot" );
		m_textFoot[pn].LoadFromTextureAndChars( THEME->GetPathToG("CourseEntryDisplay difficulty 2x1"),"10" );
		SET_XY_AND_ON_COMMAND( &m_textFoot[pn] );
		this->AddChild( &m_textFoot[pn] );

		m_textDifficultyNumber[pn].SetName( SEPARATE_COURSE_METERS? ssprintf("DifficultyP%i", pn+1):"Difficulty" );
		m_textDifficultyNumber[pn].LoadFromFont( THEME->GetPathToF("Common normal") );
		SET_XY_AND_ON_COMMAND( &m_textDifficultyNumber[pn] );
		this->AddChild( &m_textDifficultyNumber[pn] );
	}

	m_textModifiers.SetName( "Modifiers" );
	m_textModifiers.LoadFromFont( THEME->GetPathToF("Common normal") );
	SET_XY_AND_ON_COMMAND( &m_textModifiers );
	this->AddChild( &m_textModifiers );
}

void CourseEntryDisplay::SetDifficulty( PlayerNumber pn, const CString &text, RageColor c )
{
	if( !GAMESTATE->IsHumanPlayer(pn) )
		return;	// skip
	if( !SEPARATE_COURSE_METERS && pn != GAMESTATE->m_MasterPlayerNumber )
		return;

	m_textDifficultyNumber[pn].SetText( text );
	m_textDifficultyNumber[pn].SetDiffuse( c );

	m_textFoot[pn].SetText( "1" );
	m_textFoot[pn].SetDiffuse( c );
}

void CourseEntryDisplay::LoadFromCourseInfo( int iNum, const Course *pCourse, const Course::Info pci[NUM_PLAYERS] )
{
	const Course::Info &ci = pci[GAMESTATE->m_MasterPlayerNumber];

	if( ci.Mystery )
	{
		for( int pn = 0; pn < NUM_PLAYERS; ++pn )
		{
			Difficulty dc = pCourse->GetDifficulty( pci[pn] );
			if( dc == DIFFICULTY_INVALID )
			{
				int iLow, iHigh;
				pCourse->GetMeterRange(pci[pn], iLow, iHigh);

				SetDifficulty( (PlayerNumber)pn, ssprintf(iLow==iHigh?"%d":"%d-%d", iLow, iHigh), RageColor(1,1,1,1) );
			}
			else
				SetDifficulty( (PlayerNumber)pn, "?", SONGMAN->GetDifficultyColor( dc ) );
		}

		m_TextBanner.LoadFromString( "??????????", "??????????", "", "", "", "" );
		m_TextBanner.SetDiffuse( RageColor(1,1,1,1) ); // TODO: What should this be?
	}
	else
	{
		for( int pn = 0; pn < NUM_PLAYERS; ++pn )
		{
			RageColor colorNotes = SONGMAN->GetDifficultyColor( pci[pn].pNotes->GetDifficulty() );
			SetDifficulty( (PlayerNumber)pn, ssprintf("%d", pci[pn].pNotes->GetMeter()), colorNotes );
		}

		m_TextBanner.LoadFromSong( ci.pSong );
		m_TextBanner.SetDiffuse( SONGMAN->GetSongColor( ci.pSong ) );
	}

	m_textNumber.SetText( ssprintf("%d", iNum) );

	m_textModifiers.SetText( ci.Modifiers );
}
