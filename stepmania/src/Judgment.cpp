#include "global.h"
#include "Judgment.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "ThemeMetric.h"
#include "ActorUtil.h"
#include "StatsManager.h"
#include "XmlFile.h"

REGISTER_ACTOR_CLASS( Judgment )

Judgment::Judgment()
{
	m_mpToTrack = MultiPlayer_Invalid;
}

void Judgment::LoadFromNode( const XNode* pNode )
{
	RString sFile;
	if( ActorUtil::GetAttrPath(pNode, "File", sFile) )
	{
		CollapsePath( sFile );
		LoadNormal( sFile );
	}
	else
	{
		LoadNormal();
	}

	ActorFrame::LoadFromNode( pNode );
}

void Judgment::LoadNormal()
{
	LoadNormal( THEME->GetPathG("Judgment","label") );
}

void Judgment::LoadNormal( const RString &sPath )
{
	m_sprJudgment.Load( sPath );
	ASSERT( m_sprJudgment.GetNumStates() == 6  ||  m_sprJudgment.GetNumStates() == 12 );
	m_sprJudgment.StopAnimating();
	m_sprJudgment.SetName( "Judgment" );
	ActorUtil::LoadAllCommands( m_sprJudgment, "Judgment" );
	Reset();
	this->AddChild( &m_sprJudgment );
}

void Judgment::Reset()
{
	m_sprJudgment.FinishTweening();
	m_sprJudgment.SetXY( 0, 0 );
	m_sprJudgment.StopEffect();
	m_sprJudgment.SetHidden( true );
}

void Judgment::SetJudgment( TapNoteScore score, bool bEarly )
{
	//LOG->Trace( "Judgment::SetJudgment()" );

	Reset();

	m_sprJudgment.SetHidden( false );

	int iStateMult = (m_sprJudgment.GetNumStates()==12) ? 2 : 1;
	int iStateAdd = ( bEarly || ( iStateMult == 1 ) ) ? 0 : 1;

	switch( score )
	{
	case TNS_W1:
		m_sprJudgment.SetState( 0 * iStateMult + iStateAdd );
		m_sprJudgment.PlayCommand( "W1" );
		break;
	case TNS_W2:
		m_sprJudgment.SetState( 1 * iStateMult + iStateAdd );
		m_sprJudgment.PlayCommand( "W2" );
		break;
	case TNS_W3:
		m_sprJudgment.SetState( 2 * iStateMult + iStateAdd );
		m_sprJudgment.PlayCommand( "W3" );
		break;
	case TNS_W4:
		m_sprJudgment.SetState( 3 * iStateMult + iStateAdd );
		m_sprJudgment.PlayCommand( "W4" );
		break;
	case TNS_W5:
		m_sprJudgment.SetState( 4 * iStateMult + iStateAdd );
		m_sprJudgment.PlayCommand( "W5" );
		break;
	case TNS_Miss:
		m_sprJudgment.SetState( 5 * iStateMult + iStateAdd );
		m_sprJudgment.PlayCommand( "Miss" );
		break;
	default:
		ASSERT(0);
	}
}

void Judgment::LoadFromMultiPlayer( MultiPlayer mp )
{
	ASSERT( m_mpToTrack == MultiPlayer_Invalid );	// assert only load once
	m_mpToTrack = mp;
	this->SubscribeToMessage( enum_add2(Message_ShowJudgmentMuliPlayerP1,m_mpToTrack) );
}

void Judgment::HandleMessage( const Message &msg )
{
	if( m_mpToTrack != MultiPlayer_Invalid  &&  msg == enum_add2(Message_ShowJudgmentMuliPlayerP1,m_mpToTrack) )
		SetJudgment( STATSMAN->m_CurStageStats.m_multiPlayer[m_mpToTrack].tnsLast, false );	// FIXME: save and pass early bool?

	ActorFrame::HandleMessage( msg );
}


// lua start
#include "LuaBinding.h"

class LunaJudgment: public Luna<Judgment>
{
public:
	static int LoadFromMultiPlayer( T* p, lua_State *L ) { p->LoadFromMultiPlayer( Enum::Check<MultiPlayer>(L, 1) ); return 0; }

	LunaJudgment()
	{
		ADD_METHOD( LoadFromMultiPlayer );
	}
};

LUA_REGISTER_DERIVED_CLASS( Judgment, ActorFrame )
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
