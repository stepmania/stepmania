#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: TransitionBGAnimation

 Desc: See header

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "TransitionBGAnimation.h"
#include "RageUtil.h"
#include "ScreenManager.h"


TransitionBGAnimation::TransitionBGAnimation()
{
	m_State = waiting,
	m_fSecsIntoTransition = 0.0f;
}

void TransitionBGAnimation::Load( CString sBGAniDir )
{
	m_BGAnimation.LoadFromAniDir( sBGAniDir );

	m_sound.Load( sBGAniDir );
}


void TransitionBGAnimation::Update( float fDeltaTime )
{
	Actor::Update( fDeltaTime );

	if( m_State == transitioning )
	{
		m_BGAnimation.Update( fDeltaTime );
		if( m_fSecsIntoTransition > m_BGAnimation.GetLengthSeconds() )	// over
		{
			m_fSecsIntoTransition = 0.0;
			m_State = finished;
			SCREENMAN->SendMessageToTopScreen( m_MessageToSendWhenDone );
		}

		m_fSecsIntoTransition += fDeltaTime;
	}
}

void TransitionBGAnimation::DrawPrimitives()
{
//	if( m_State == transitioning )
		m_BGAnimation.Draw();
}

void TransitionBGAnimation::StartTransitioning( ScreenMessage send_when_done )
{
	ASSERT( m_State == waiting );	// can't call this more than once
	m_MessageToSendWhenDone = send_when_done;
	m_State = transitioning;
	m_sound.PlayRandom();
	m_fSecsIntoTransition = 0.0;
}

float TransitionBGAnimation::GetLengthSeconds()
{
	return m_BGAnimation.GetLengthSeconds();
}
