#include "global.h"
#include "ScreenGameOver.h"
#include "ScreenManager.h"
#include "AnnouncerManager.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"


const ScreenMessage SM_StartFadingOut	=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_PlayAnnouncer	=	ScreenMessage(SM_User + 3);


#define NEXT_SCREEN		THEME->GetMetric("ScreenGameOver","NextScreen")


ScreenGameOver::ScreenGameOver( CString sName ) : Screen( sName )
{
	m_Background.LoadFromAniDir( THEME->GetPathToB("ScreenGameOver background") );
	this->AddChild( &m_Background );

	m_In.Load( THEME->GetPathToB("ScreenGameOver in") );
	this->AddChild( &m_In );

	m_Out.Load( THEME->GetPathToB("ScreenGameOver out") );
	this->AddChild( &m_Out );

	SOUND->PlayMusic( THEME->GetPathToS("ScreenGameOver music") );

	m_In.StartTransitioning( SM_PlayAnnouncer );
	this->PostScreenMessage( SM_StartFadingOut, m_Background.GetLengthSeconds() );
}


void ScreenGameOver::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_PlayAnnouncer:
		SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("game over") );
		break;
	case SM_StartFadingOut:
		m_Out.StartTransitioning( SM_GoToNextScreen );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		break;
	}
}

void ScreenGameOver::MenuStart( PlayerNumber pn )
{
	if( m_In.IsTransitioning() || m_Out.IsTransitioning() )
		return;

	this->ClearMessageQueue();
	this->PostScreenMessage( SM_StartFadingOut, 0 );
}

/*
 * (c) 2002-2003 Chris Danford
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
