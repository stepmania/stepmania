#include "global.h"
#include "ScreenCaution.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "AnnouncerManager.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"


#define NEXT_SCREEN				THEME->GetMetric(m_sName,"NextScreen")


const ScreenMessage SM_DoneOpening		= ScreenMessage(SM_User-7);
const ScreenMessage SM_StartClosing		= ScreenMessage(SM_User-8);


ScreenCaution::ScreenCaution( CString sName ) : Screen( sName )
{
	if(!PREFSMAN->m_bShowDontDie)
	{
		this->PostScreenMessage( SM_GoToNextScreen, 0.f );
		return;
	}

	m_Background.LoadFromAniDir( THEME->GetPathB(m_sName,"background") );
	this->AddChild( &m_Background );
	
	m_In.Load( THEME->GetPathB(m_sName,"in") );
	m_In.StartTransitioning( SM_DoneOpening );
	this->AddChild( &m_In );

	m_Out.Load( THEME->GetPathB(m_sName,"out") );
	this->AddChild( &m_Out );

	m_Back.Load( THEME->GetPathToB("Common back") );
	this->AddChild( &m_Back );

	float fCloseInSeconds = m_Background.GetLengthSeconds()-m_Out.GetLengthSeconds();
	fCloseInSeconds = max( 0, fCloseInSeconds );
	this->PostScreenMessage( SM_StartClosing, fCloseInSeconds );

	SOUND->PlayMusic( THEME->GetPathS(m_sName,"music") );
}


void ScreenCaution::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( Screen::JoinInput( DeviceI, type, GameI, MenuI, StyleI ) )
		return;	// handled

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}


void ScreenCaution::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_StartClosing:
		m_Background.PlayCommand("Off");
		m_Out.StartTransitioning( SM_GoToNextScreen );
		break;
	case SM_DoneOpening:
		SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("caution") );
		break;
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		break;
	}
}

void ScreenCaution::MenuStart( PlayerNumber pn )
{
	if( m_In.IsTransitioning() || m_Out.IsTransitioning() || m_Back.IsTransitioning() )
		return;
	m_Background.PlayCommand("Off");
	m_Out.StartTransitioning( SM_GoToNextScreen );
}

void ScreenCaution::MenuBack( PlayerNumber pn )
{
	if( m_In.IsTransitioning() || m_Out.IsTransitioning() || m_Back.IsTransitioning() )
		return;
	this->ClearMessageQueue();
	m_Back.StartTransitioning( SM_GoToPrevScreen );
	SCREENMAN->PlayBackSound();
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
