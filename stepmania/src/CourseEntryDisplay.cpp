#include "global.h"
#include "CourseEntryDisplay.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "Course.h"
#include "SongManager.h"
#include "ThemeManager.h"
#include "Steps.h"
#include "GameState.h"
#include "Style.h"
#include "ActorUtil.h"
#include "LuaBinding.h"

REGISTER_ACTOR_CLASS( CourseEntryDisplay )

CourseEntryDisplay::CourseEntryDisplay()
{
	m_sName = "CourseEntryDisplay";
}

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

		m_textFoot[pn].SetName( SEPARATE_COURSE_METERS? ssprintf("FootP%i", pn+1):RString("Foot") );
		m_textFoot[pn].LoadFromTextureAndChars( THEME->GetPathF("CourseEntryDisplay","difficulty"),"10" );
		SET_XY_AND_ON_COMMAND( &m_textFoot[pn] );
		this->AddChild( &m_textFoot[pn] );

		m_textDifficultyNumber[pn].SetName( SEPARATE_COURSE_METERS? ssprintf("DifficultyP%i", pn+1):RString("Difficulty") );
		m_textDifficultyNumber[pn].LoadFromFont( THEME->GetPathF("Common","normal") );
		SET_XY_AND_ON_COMMAND( &m_textDifficultyNumber[pn] );
		this->AddChild( &m_textDifficultyNumber[pn] );
	}

	m_textModifiers.SetName( "Modifiers" );
	m_textModifiers.LoadFromFont( THEME->GetPathF("Common","normal") );
	SET_XY_AND_ON_COMMAND( &m_textModifiers );
	this->AddChild( &m_textModifiers );
}

void CourseEntryDisplay::LoadFromNode( const XNode* pNode )
{
	ActorFrame::LoadFromNode( pNode );

	Load();
}

void CourseEntryDisplay::SetDifficulty( PlayerNumber pn, const RString &text, Difficulty dc )
{
	if( !GAMESTATE->IsHumanPlayer(pn) )
		return;	// skip
	if( !SEPARATE_COURSE_METERS && pn != GAMESTATE->m_MasterPlayerNumber )
		return;

	Lua *L = LUA->Get();
	LuaHelpers::Push( L, dc );
	m_textDifficultyNumber[pn].m_pLuaInstance->Set( L, "Difficulty" );
	LuaHelpers::Push( L, dc );
	m_textFoot[pn].m_pLuaInstance->Set( L, "Difficulty" );
	LUA->Release(L);

	m_textDifficultyNumber[pn].SetText( text );
	m_textDifficultyNumber[pn].PlayCommand( "DifficultyChanged" );

	m_textFoot[pn].SetText( "1" );
	m_textFoot[pn].PlayCommand( "DifficultyChanged" );
}

void CourseEntryDisplay::SetFromGameState( int iCourseEntryIndex )
{
	Course *pCourse = GAMESTATE->m_pCurCourse;

	const TrailEntry *tes[NUM_PLAYERS];
	const CourseEntry *ces[NUM_PLAYERS];
	FOREACH_PlayerNumber( p )
	{
		Trail *pTrail = GAMESTATE->m_pCurTrail[p];
		if( pTrail  &&  iCourseEntryIndex < (int) pTrail->m_vEntries.size() )
		{
			tes[p] = &pTrail->m_vEntries[iCourseEntryIndex];
			ces[p] = &pCourse->m_vEntries[iCourseEntryIndex];
		}
		else
		{
			tes[p] = NULL;
			ces[p] = NULL;
		}
	}


	const TrailEntry *te = tes[GAMESTATE->m_MasterPlayerNumber];
	if( te == NULL )
		return;

	if( te->bSecret )
	{
		FOREACH_EnabledPlayer(pn)
		{
			const TrailEntry *te = tes[pn];
			const CourseEntry *ce = ces[pn];
			if( te == NULL || ce == NULL )
				continue;

			int iLow = ce->stepsCriteria.m_iLowMeter;
			int iHigh = ce->stepsCriteria.m_iHighMeter;

			bool bLowIsSet = iLow != -1;
			bool bHighIsSet = iHigh != -1;

			RString s;
			if( !bLowIsSet  &&  !bHighIsSet )
			{
				s = "?";
			}
			if( !bLowIsSet  &&  bHighIsSet )
			{
				s = ssprintf( ">=%d", iHigh );
			}
			else if( bLowIsSet  &&  !bHighIsSet )
			{
				s = ssprintf( "<=%d", iLow );
			}
			else if( bLowIsSet  &&  bHighIsSet )
			{
				if( iLow == iHigh )
					s = ssprintf( "%d", iLow );
				else
					s = ssprintf( "%d-%d", iLow, iHigh );
			}

			Difficulty dc = te->dc;
			if( dc == Difficulty_Invalid )
				dc = DIFFICULTY_EDIT;
			
			SetDifficulty( pn, s, dc );
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
			SetDifficulty( pn, ssprintf("%d", te->GetSteps()->GetMeter()), te->GetSteps()->GetDifficulty() );
		}

		m_TextBanner.LoadFromSong( te->pSong );
		m_TextBanner.SetDiffuse( SONGMAN->GetSongColor( te->pSong ) );
	}

	m_textNumber.SetText( ssprintf("%d", iCourseEntryIndex+1) );

	m_textModifiers.SetText( te->Modifiers );
}


// lua start
#include "LuaBinding.h"

class LunaCourseEntryDisplay: public Luna<CourseEntryDisplay>
{
public:
	static int SetFromGameState( T* p, lua_State *L )	{ p->SetFromGameState(IArg(1)); return 0; }

	LunaCourseEntryDisplay()
	{
		ADD_METHOD( SetFromGameState );
	}
};

LUA_REGISTER_DERIVED_CLASS( CourseEntryDisplay, ActorFrame )
// lua end

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
