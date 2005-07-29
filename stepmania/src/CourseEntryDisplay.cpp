#include "global.h"
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
#include "Style.h"
#include "ActorUtil.h"

void CourseEntryDisplay::Load()
{
	SEPARATE_COURSE_METERS	.Load( "CourseEntryDisplay", "SeparateCourseMeters" );
	TEXT_BANNER_TYPE		.Load( "CourseEntryDisplay", "TextBannerType" );

	m_sprFrame.SetName( "Bar" );
	m_sprFrame.Load( THEME->GetPathG("CourseEntryDisplay","bar") );
	SET_XY_AND_ON_COMMAND( &m_sprFrame );
	this->AddChild( &m_sprFrame );

	this->m_size.x = (float) m_sprFrame.GetTexture()->GetSourceFrameWidth();
	this->m_size.y = (float) m_sprFrame.GetTexture()->GetSourceFrameHeight();

	m_textNumber.SetName( "Number" );
	m_textNumber.LoadFromFont( THEME->GetPathF("CourseEntryDisplay","number") );
	SET_XY_AND_ON_COMMAND( &m_textNumber );
	this->AddChild( &m_textNumber );

	m_TextBanner.SetName( "TextBanner" );
	m_TextBanner.Load( TEXT_BANNER_TYPE );
	SET_XY_AND_ON_COMMAND( &m_TextBanner );
	/* Load the m_TextBanner now, so any actor commands sent to us will propagate correctly. */
	m_TextBanner.LoadFromString( "", "", "", "", "", "" );
	this->AddChild( &m_TextBanner );

	FOREACH_HumanPlayer( pn )
	{
		if( !SEPARATE_COURSE_METERS && pn != GAMESTATE->m_MasterPlayerNumber )
			continue;	// skip

		m_textFoot[pn].SetName( SEPARATE_COURSE_METERS? ssprintf("FootP%i", pn+1):CString("Foot") );
		m_textFoot[pn].LoadFromTextureAndChars( THEME->GetPathG("CourseEntryDisplay","difficulty 2x1"),"10" );
		SET_XY_AND_ON_COMMAND( &m_textFoot[pn] );
		this->AddChild( &m_textFoot[pn] );

		m_textDifficultyNumber[pn].SetName( SEPARATE_COURSE_METERS? ssprintf("DifficultyP%i", pn+1):CString("Difficulty") );
		m_textDifficultyNumber[pn].LoadFromFont( THEME->GetPathF("Common","normal") );
		SET_XY_AND_ON_COMMAND( &m_textDifficultyNumber[pn] );
		this->AddChild( &m_textDifficultyNumber[pn] );
	}

	m_textModifiers.SetName( "Modifiers" );
	m_textModifiers.LoadFromFont( THEME->GetPathF("Common","normal") );
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

void CourseEntryDisplay::LoadFromTrailEntry( int iNum, const TrailEntry *tes[NUM_PLAYERS] )
{
	const TrailEntry *te = tes[GAMESTATE->m_MasterPlayerNumber];
	if( te == NULL )
		return;

	if( te->bSecret )
	{
		FOREACH_EnabledPlayer(pn)
		{
			const TrailEntry *te = tes[pn];
			if( te == NULL )
				continue;

			Difficulty dc = te->dc;
			if( dc == DIFFICULTY_INVALID )
			{
				int iLow = te->iLowMeter;
				int iHigh = te->iHighMeter;
				RageColor colorNotes = SONGMAN->GetDifficultyColor( te->pSteps->GetDifficulty() );
				SetDifficulty( pn, ssprintf(iLow==iHigh?"%d":"%d-%d", iLow, iHigh), colorNotes );
			}
			else
			{
				SetDifficulty( pn, "?", SONGMAN->GetDifficultyColor( dc ) );
			}
		}

		m_TextBanner.LoadFromString( "??????????", "??????????", "", "", "", "" );
		m_TextBanner.SetDiffuse( RageColor(1,1,1,1) ); // TODO: What should this be?
	}
	else
	{
		FOREACH_EnabledPlayer(pn)
		{
			const TrailEntry *te = tes[pn];
			if( te == NULL )
				continue;
			RageColor colorNotes = SONGMAN->GetDifficultyColor( te->pSteps->GetDifficulty() );
			SetDifficulty( pn, ssprintf("%d", te->pSteps->GetMeter()), colorNotes );
		}

		m_TextBanner.LoadFromSong( te->pSong );
		m_TextBanner.SetDiffuse( SONGMAN->GetSongColor( te->pSong ) );
	}

	m_textNumber.SetText( ssprintf("%d", iNum) );

	m_textModifiers.SetText( te->Modifiers );
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
