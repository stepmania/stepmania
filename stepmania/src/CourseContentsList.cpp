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

REGISTER_ACTOR_CLASS( CourseContentsList )

CourseContentsList::CourseContentsList()
{
	m_iVisibleItems = 5;
}

CourseContentsList::~CourseContentsList()
{
	FOREACH( Actor *, m_vpDisplay, d )
		delete *d;
	m_vpDisplay.clear();
}

void CourseContentsList::LoadFromNode( const XNode* pNode )
{
	pNode->GetAttrValue( "VisibleItems", m_iVisibleItems );

	ActorScroller::LoadFromNode( pNode );

	const XNode *pDisplayNode = pNode->GetChild( "Display" );
	if( pDisplayNode == NULL )
		RageException::Throw( "%s: CourseContentsList: missing the Display child", ActorUtil::GetWhere(pNode).c_str() );

	for( int i=0; i<m_iVisibleItems+2; i++ )
	{
		Actor *pDisplay = ActorUtil::LoadFromNode( pDisplayNode, this );
		pDisplay->SetUseZBuffer( true );
		m_vpDisplay.push_back( pDisplay );
	}

	this->SetNumItemsToDraw( (float)m_iVisibleItems );
}

void CourseContentsList::SetFromGameState()
{
	RemoveAllChildren();

	// FIXME: Is there a better way to handle when players don't have 
	// the same number of TrailEntries?
	// They have to have the same number, and of the same songs, or gameplay
	// isn't going to line up.
	const Trail *pMasterTrail = GAMESTATE->m_pCurTrail[GAMESTATE->m_MasterPlayerNumber];
	if( pMasterTrail == NULL )
		return;
	unsigned uNumEntriesToShow = min( pMasterTrail->m_vEntries.size(), m_vpDisplay.size() );

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
	this->SetSecondsPerItem( 0.7f );
	this->EnableMask( m_vpDisplay[0]->GetUnzoomedWidth(), m_vpDisplay[0]->GetUnzoomedHeight() );
	this->SetSecondsPauseBetweenItems( 0.7f );
	this->ScrollThroughAllItems();

	this->SetCurrentAndDestinationItem( (m_iVisibleItems-1)/2 );
	if( bLoop )
	{
		SetPauseCountdownSeconds( 1.5f );
		this->SetDestinationItem( m_vpDisplay.size()+1 );	// loop forever
	}
}

void CourseContentsList::SetItemFromGameState( Actor *pActor, int iCourseEntryIndex )
{
	const Course *pCourse = GAMESTATE->m_pCurCourse;

	const TrailEntry *tes[NUM_PLAYERS];
	const CourseEntry *ces[NUM_PLAYERS];
	FOREACH_PlayerNumber( p )
	{
		const Trail *pTrail = GAMESTATE->m_pCurTrail[p];
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

	FOREACH_HumanPlayer(pn)
	{
		const TrailEntry *te = tes[pn];
		if( te == NULL )
			continue;

		RString s;
		Difficulty dc;
		if( te->bSecret )
		{
			const CourseEntry *ce = ces[pn];
			if( ce == NULL )
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
		msg.SetParam( "Difficulty", dc );
		msg.SetParam( "Meter", s );
		msg.SetParam( "Number", iCourseEntryIndex+1 );
		msg.SetParam( "Modifiers", te->Modifiers );
		if( !te->bSecret )
			msg.SetParam( "Song", te->pSong );
		pActor->HandleMessage( msg );
	}
}

// lua start
#include "LuaBinding.h"

class LunaCourseContentsList: public Luna<CourseContentsList>
{
public:
	static int SetFromGameState( T* p, lua_State *L )			{ p->SetFromGameState(); return 0; }

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
