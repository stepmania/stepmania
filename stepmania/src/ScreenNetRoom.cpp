#include "global.h"

//XXX: THIS IS JUST A STUB!
//XXX: THIS CLASS IS NOT FUNCTIONAL

#if !defined(WITHOUT_NETWORKING)
#include "ScreenNetRoom.h"
#include "ScreenManager.h"
#include "NetworkSyncManager.h"
#include "GameState.h"
#include "ThemeManager.h"

const ScreenMessage SM_SMOnlinePack	= ScreenMessage(SM_User+8);	//Unused, but should be known


ScreenNetRoom::ScreenNetRoom( const CString& sName ) : ScreenNetSelectBase( sName )
{
	GAMESTATE->FinishStage();
	m_soundChangeSel.Load( THEME->GetPathToS("ScreenNetRoom change sel"));

	NSMAN->ReportNSSOnOff(7);
	return;
}

void ScreenNetRoom::Input( const DeviceInput& DeviceI, const InputEventType type,
								  const GameInput& GameI, const MenuInput& MenuI,
								  const StyleInput& StyleI )
{
	ScreenNetSelectBase::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenNetRoom::HandleScreenMessage( const ScreenMessage SM )
{
	ScreenNetSelectBase::HandleScreenMessage( SM );
}

void ScreenNetRoom::TweenOffScreen()
{
	NSMAN->ReportNSSOnOff(6);
}

void ScreenNetRoom::Update( float fDeltaTime )
{
	ScreenNetSelectBase::Update( fDeltaTime );
}

void ScreenNetRoom::MenuStart( PlayerNumber pn )
{
	ScreenNetSelectBase::MenuStart( pn );
}

void ScreenNetRoom::MenuBack( PlayerNumber pn )
{
	ScreenNetSelectBase::MenuBack( pn );
}

void ScreenNetRoom::MenuUp( PlayerNumber pn, const InputEventType type )
{
	ScreenNetSelectBase::MenuUp( pn );
}

void ScreenNetRoom::MenuDown( PlayerNumber pn, const InputEventType type )
{
	ScreenNetSelectBase::MenuDown( pn );
}

#endif

/*
 * (c) 2004 Charles Lohr
 * All rights reserved.
 *      Elements from ScreenTextEntry
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
