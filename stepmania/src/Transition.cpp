#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: Transition

 Desc: See header

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Transition.h"
#include "RageUtil.h"
#include "ScreenManager.h"
#include "IniFile.h"
#include "RageFile.h"


Transition::Transition()
{
	m_State = waiting;
	m_fSecsIntoTransition = 0.0f;
}

void Transition::Load( CString sBGAniDir )
{
	if( !sBGAniDir.empty() && sBGAniDir.Right(1) != SLASH )
		sBGAniDir += SLASH;

	m_BGAnimation.LoadFromAniDir( sBGAniDir );

	m_State = waiting;
	m_fSecsIntoTransition = 0.0f;

	// load sound from file specified by ini, or use the first sound in the directory
	IniFile ini;
	ini.SetPath( sBGAniDir+"BGAnimation.ini" );
	ini.ReadFile();

	CString sSoundFileName;
	if( ini.GetValue("BGAnimation","Sound",sSoundFileName) )
	{
		FixSlashesInPlace( sSoundFileName );
		CString sPath = sBGAniDir+sSoundFileName;
		CollapsePath( sPath );
		m_sound.Load( sPath );
	}
	else
	{
		m_sound.Load( sBGAniDir );
	}
}


void Transition::Update( float fDeltaTime )
{
	Actor::Update( fDeltaTime );

	if( m_State == transitioning )
	{
		/* Start the transition on the first update, not in the ctor, so
		 * we don't play a sound while our parent is still loading. */
		if( m_fSecsIntoTransition==0 )	// first update
		{
			m_sound.PlayRandom();

			// stop the sound from playing multiple times on an Update(0)
			m_fSecsIntoTransition+=0.000001f;	
		}

		m_BGAnimation.Update( fDeltaTime );
		if( m_fSecsIntoTransition > m_BGAnimation.GetLengthSeconds() )	// over
		{
			SCREENMAN->SendMessageToTopScreen( m_MessageToSendWhenDone );
			m_fSecsIntoTransition = 0.0;
			m_State = finished;
		}
		else
			m_fSecsIntoTransition += fDeltaTime;
	}
}

void Transition::DrawPrimitives()
{
	// Unless we're transitioning, don't draw because we'll waste resources drawing things
	// that aren't visible.  -Chris
	switch( m_State )
	{
	case waiting:
		return;
	case finished:
		return;
	}

	m_BGAnimation.Draw();
}

void Transition::StartTransitioning( ScreenMessage send_when_done )
{
	if( m_State != waiting )
	{
		// This assert will trigger in some cases if start is pressed
		// right as a transition is ending.  We want to know about these
		// problems in debug builds, but they're not fatal.
		DEBUG_ASSERT( m_State == waiting );	// can't call this more than once
		return;	// ignore
	}
	m_MessageToSendWhenDone = send_when_done;
	m_State = transitioning;
	m_fSecsIntoTransition = 0.0;
}

float Transition::GetLengthSeconds()
{
	return m_BGAnimation.GetLengthSeconds();
}
