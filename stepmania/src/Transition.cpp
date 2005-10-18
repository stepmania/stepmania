#include "global.h"
#include "Transition.h"
#include "ScreenManager.h"


/*
 * XXX: We send OnCommand on load, then we send no updates until we get
 * StartTransitioning.  Historically, this was fine, but it's incorrect now
 * that commands can have arbitrary side-effects.  We should only send OnCommand
 * when we get StartTransitioning, but we need to run it early to set m_fLengthSeconds.
 * Eventually, phase out GetLengthSeconds (ScreenGameplay is somewhat tricky).  For
 * now, play a "StartTransitioning" command; put anything that has side-effects in
 * there, instead.
 */
Transition::Transition()
{
	m_State = waiting;
}

void Transition::Load( CString sBGAniDir )
{
	this->RemoveAllChildren();

	m_sprTransition.Load( sBGAniDir );
	m_sprTransition->PlayCommand( "On" );
	this->AddChild( m_sprTransition );
	m_fLengthSeconds = m_sprTransition->GetTweenTimeLeft();

	m_State = waiting;
}


void Transition::UpdateInternal( float fDeltaTime )
{
	if( m_State != transitioning )
		return;

	// Check this before running Update, so we draw the last frame of the finished
	// transition before sending m_MessageToSendWhenDone.
	if( m_sprTransition->GetTweenTimeLeft() == 0 )	// over
	{
		m_State = finished;
		SCREENMAN->SendMessageToTopScreen( m_MessageToSendWhenDone );
	}

	ActorFrame::UpdateInternal( fDeltaTime );
}

void Transition::Reset()
{
	m_State = waiting;
	m_bFirstUpdate = true;

	if( m_sprTransition.IsLoaded() )
	{
		m_sprTransition->FinishTweening();
		m_sprTransition->PlayCommand( "On" );
	}
}

bool Transition::EarlyAbortDraw() const
{
	return m_State == waiting;
}

/* Our parent might send us OnCommand.  We do that ourself, because
 * we sometimes want to know GetLengthSeconds before StartTransitioning.
 * Make sure we don't process OnCommand twice. */
void Transition::PlayCommand( const CString &sCommandName, Actor *pParent )
{
	if( sCommandName == "On" )
		return;

	ActorFrame::PlayCommand( sCommandName, pParent );
}

void Transition::StartTransitioning( ScreenMessage send_when_done )
{
	if( m_State != waiting )
		return;	// ignore
	
	// If transition isn't loaded don't set state to transitioning.
	// We assume elsewhere that m_sprTransition is loaded.
	if( !m_sprTransition.IsLoaded() )
		return;
	
	m_sprTransition->PlayCommand( "StartTransitioning" );

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
