#include "global.h"
#include "Transition.h"
#include "RageUtil.h"
#include "ScreenManager.h"
#include "IniFile.h"
#include "RageFile.h"


Transition::Transition()
{
	m_State = waiting;
}

void Transition::Load( CString sBGAniDir )
{
	if( !sBGAniDir.empty() && sBGAniDir.Right(1) != "/" )
		sBGAniDir += "/";

	bool bWasLoaded = m_sprTransition.IsLoaded();
	m_sprTransition.Load( sBGAniDir );
	m_sprTransition->PlayCommand( "On" );
	m_fLengthSeconds = m_sprTransition->GetTweenTimeLeft();

	if( !bWasLoaded )
		this->AddChild( m_sprTransition );

	m_State = waiting;

	// load sound from file specified by ini, or use the first sound in the directory
	IniFile ini;
	ini.ReadFile( sBGAniDir+"BGAnimation.ini" );

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
	if( m_State != transitioning )
		return;

	/* Start the transition on the first update, not in the ctor, so
	 * we don't play a sound while our parent is still loading. */
	if( m_bFirstUpdate )
		m_sound.PlayRandom();

	// Check this before running Update, so we draw the last frame of the finished
	// transition before sending m_MessageToSendWhenDone.
	if( m_sprTransition->GetTweenTimeLeft() == 0 )	// over
	{
		m_State = finished;
		SCREENMAN->SendMessageToTopScreen( m_MessageToSendWhenDone );
	}

	ActorFrame::Update( fDeltaTime );
}

void Transition::Reset()
{
	ASSERT( m_State == finished );
	m_State = waiting;
}

bool Transition::EarlyAbortDraw()
{
	return m_State != transitioning;
}

void Transition::StartTransitioning( ScreenMessage send_when_done )
{
	if( m_State != waiting )
		return;	// ignore

	m_MessageToSendWhenDone = send_when_done;
	m_State = transitioning;
}

float Transition::GetLengthSeconds() const
{
	return m_fLengthSeconds;
}

float Transition::GetTweenTimeLeft() const
{
	if( m_State != transitioning )
		return 0;

	return m_sprTransition->GetTweenTimeLeft();
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
