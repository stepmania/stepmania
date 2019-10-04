#include "global.h"
#include "CourseContentsList.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "Course.h"
#include "Trail.h"
#include "GameState.h"
#include "XmlFile.h"
#include "ActorUtil.h"
#include "RageUtil.h"
#include "Steps.h"

REGISTER_ACTOR_CLASS( CourseContentsList );

CourseContentsList::~CourseContentsList()
{
	for (Actor *d : m_vpDisplay)
		delete d;
	m_vpDisplay.clear();
}

void CourseContentsList::LoadFromNode( const XNode* pNode )
{
	int iMaxSongs = 5;
	pNode->GetAttrValue( "MaxSongs", iMaxSongs );

	const XNode *pDisplayNode = pNode->GetChild( "Display" );
	if( pDisplayNode == nullptr )
	{
		LuaHelpers::ReportScriptErrorFmt("%s: CourseContentsList: missing the Display child", ActorUtil::GetWhere(pNode).c_str());
		return;
	}

	for( int i=0; i<iMaxSongs; i++ )
	{
		Actor *pDisplay = ActorUtil::LoadFromNode( pDisplayNode, this );
		pDisplay->SetUseZBuffer( true );
		m_vpDisplay.push_back( pDisplay );
	}

	ActorScroller::LoadFromNode( pNode );
}

void CourseContentsList::SetFromGameState()
{
	RemoveAllChildren();

	if( GAMESTATE->GetMasterPlayerNumber() == PlayerNumber_Invalid )
		return;
	const Trail *pMasterTrail = GAMESTATE->m_pCurTrail[GAMESTATE->GetMasterPlayerNumber()];
	if( pMasterTrail == nullptr )
		return;
	unsigned uNumEntriesToShow = pMasterTrail->m_vEntries.size(); 
	CLAMP( uNumEntriesToShow, 0, m_vpDisplay.size() );

	for( int i=0; i<(int)uNumEntriesToShow; i++ )
	{
		Actor *pDisplay = m_vpDisplay[i];
		SetItemFromGameState( pDisplay, i );
		this->AddChild( pDisplay );
	}

	bool bLoop = pMasterTrail->m_vEntries.size() > uNumEntriesToShow;

	this->SetLoop( bLoop );
	this->Load2();
	this->SetTransformFromHeight( m_vpDisplay[0]->GetUnzoomedHeight() );
	this->EnableMask( m_vpDisplay[0]->GetUnzoomedWidth(), m_vpDisplay[0]->GetUnzoomedHeight() );

	if( bLoop )
	{
		SetPauseCountdownSeconds( 1.5f );
		this->SetDestinationItem( (float)m_vpDisplay.size()+1 );	// loop forever
	}
}

void CourseContentsList::SetItemFromGameState( Actor *pActor, int iCourseEntryIndex )
{
	const Course *pCourse = GAMESTATE->m_pCurCourse;

	FOREACH_HumanPlayer(pn)
	{
		const Trail *pTrail = GAMESTATE->m_pCurTrail[pn];
		if( pTrail == nullptr
			|| iCourseEntryIndex >= (int) pTrail->m_vEntries.size()
			|| iCourseEntryIndex >= (int) pCourse->m_vEntries.size() )
			continue;

		const TrailEntry *te = &pTrail->m_vEntries[iCourseEntryIndex];
		const CourseEntry *ce = &pCourse->m_vEntries[iCourseEntryIndex];
		if( te == nullptr )
			continue;

		RString s;
		Difficulty dc;
		if( te->bSecret )
		{
			if( ce == nullptr )
				continue;

			int iLow = ce->stepsCriteria.m_iLowMeter;
			int iHigh = ce->stepsCriteria.m_iHighMeter;

			bool bLowIsSet = iLow != -1;
			bool bHighIsSet = iHigh != -1;

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

			dc = te->dc;
			if( dc == Difficulty_Invalid )
				dc = Difficulty_Edit;
		}
		else
		{
			s = ssprintf("%d", te->pSteps->GetMeter());
			dc = te->pSteps->GetDifficulty();
		}

		Message msg("SetSong");
		msg.SetParam( "PlayerNumber", pn );
		msg.SetParam( "Song", te->pSong );
		msg.SetParam( "Steps", te->pSteps );
		msg.SetParam( "Difficulty", dc );
		msg.SetParam( "Meter", s );
		msg.SetParam( "Number", iCourseEntryIndex+1 );
		msg.SetParam( "Modifiers", te->Modifiers );
		msg.SetParam( "Secret", te->bSecret );
		pActor->HandleMessage( msg );
	}
}

// lua start
#include "LuaBinding.h"

/** @brief Allow Lua to have access to the CourseContentsList. */ 
class LunaCourseContentsList: public Luna<CourseContentsList>
{
public:
	static int SetFromGameState( T* p, lua_State *L )			{ p->SetFromGameState(); COMMON_RETURN_SELF; }

	LunaCourseContentsList()
	{
		ADD_METHOD( SetFromGameState );
	}
};

LUA_REGISTER_DERIVED_CLASS( CourseContentsList, ActorScroller )
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
