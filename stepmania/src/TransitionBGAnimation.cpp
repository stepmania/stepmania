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
#include "IniFile.h"


TransitionBGAnimation::TransitionBGAnimation()
{
	m_State = waiting,
	m_fSecsIntoTransition = 0.0f;
}

void TransitionBGAnimation::Load( CString sBGAniDir )
{
	if( !sBGAniDir.empty() && sBGAniDir.Right(1) != "/" )
		sBGAniDir += "/";

	m_BGAnimation.LoadFromAniDir( sBGAniDir );

	// load sound from file specified by ini, or use the first sound in the directory
	IniFile ini;
	ini.SetPath( sBGAniDir+"BGAnimation.ini" );
	ini.ReadFile();

	CString sSoundFileName;
	if( ini.GetValue("BGAnimation","Sound",sSoundFileName) )
		m_sound.Load( sBGAniDir+sSoundFileName );
	else
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
