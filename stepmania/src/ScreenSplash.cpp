#include "global.h"
#include "ScreenSplash.h"
#include "ThemeManager.h"
#include "RageUtil.h"

AutoScreenMessage( SM_PrepScreen )


REGISTER_SCREEN_CLASS( ScreenSplash );
ScreenSplash::ScreenSplash( CString sClassName ) : ScreenWithMenuElements( sClassName ),
	ALLOW_START_TO_SKIP			(m_sName,"AllowStartToSkip"),
	NEXT_SCREEN					(m_sName,"NextScreen"),
	MINIMUM_LOAD_DELAY_SECONDS	(m_sName,"MinimumLoadDelaySeconds"),
	PREPARE_SCREEN				(m_sName,"PrepareScreen")
{
}

void ScreenSplash::Init()
{
	ScreenWithMenuElements::Init();

	/* Prep the new screen once m_sprOverlay is complete. */ 	 
	this->PostScreenMessage( SM_PrepScreen, m_In.GetTweenTimeLeft() );
}

void ScreenSplash::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_PrepScreen )
	{
		RageTimer length;
		if( PREPARE_SCREEN )
			SCREENMAN->PrepareScreen( NEXT_SCREEN );
		float fScreenLoadSeconds = length.GetDeltaTime();

		/* The screen load took fScreenLoadSeconds.  Move on to the next screen after
		 * no less than MINIMUM_DELAY seconds. */
		this->PostScreenMessage( SM_GoToNextScreen, max( 0, MINIMUM_LOAD_DELAY_SECONDS-fScreenLoadSeconds) );
		return;
	}
	else if( SM == SM_BeginFadingOut )
	{
		StartTransitioning( SM_GoToNextScreen );
		return;
	}

	ScreenWithMenuElements::HandleScreenMessage( SM );
}

void ScreenSplash::MenuBack( PlayerNumber pn )
{
	if( IsTransitioning() )
		return;
	Cancel( SM_GoToPrevScreen );
}

void ScreenSplash::MenuStart( PlayerNumber pn )
{
	if( IsTransitioning() )
		return;
	if( !ALLOW_START_TO_SKIP )
		return;
	this->ClearMessageQueue( SM_PrepScreen );
	HandleScreenMessage( SM_PrepScreen );
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
