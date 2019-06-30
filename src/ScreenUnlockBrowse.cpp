#include "global.h"
#include "ScreenUnlockBrowse.h"
#include "UnlockManager.h"


REGISTER_SCREEN_CLASS( ScreenUnlockBrowse );

void ScreenUnlockBrowse::Init()
{
	int index = 0;
	// fill m_aGameCommands before calling Init()
	for (UnlockEntry const &ue : UNLOCKMAN->m_UnlockEntries)
	{
		GameCommand gc;
		UnlockEntryStatus st = ue.GetUnlockEntryStatus();
		switch( st )
		{
		default:
			FAIL_M(ssprintf("Invalid UnlockEntryStatus: %i", st));
		case UnlockEntryStatus_RequirementsMet:
		case UnlockEntryStatus_Unlocked:
			gc.m_bInvalid = false;
			break;
		case UnlockEntryStatus_RequrementsNotMet:
			break;
		}
		gc.m_iIndex = index++;
		// gc.m_sUnlockEntryID = ue->m_sEntryID;
		gc.m_sName = ssprintf("%d",gc.m_iIndex);
		
		m_aGameCommands.push_back( gc );
	}

	ScreenSelectMaster::Init();

	m_banner.SetName( "Banner" );
	LOAD_ALL_COMMANDS_AND_SET_XY( m_banner );
	this->AddChild( &m_banner );

	this->SubscribeToMessage( Message_MenuSelectionChanged );
}

void ScreenUnlockBrowse::BeginScreen()
{
	ScreenSelectMaster::BeginScreen();

	HandleMessage( Message(MessageIDToString(Message_MenuSelectionChanged)) );
}

bool ScreenUnlockBrowse::MenuStart( const InputEventPlus &input )
{
	m_soundStart.Play(true);
	this->PostScreenMessage( SM_BeginFadingOut, 0 );
	return true;
}

void ScreenUnlockBrowse::HandleMessage( const Message &msg )
{
	if( msg == Message_MenuSelectionChanged )
	{
		int iSelection = this->GetSelectionIndex(PLAYER_1);
		const UnlockEntry &ue = UNLOCKMAN->m_UnlockEntries[ iSelection ];
		if( ue.IsLocked() )
			m_banner.LoadFallback();
		else
			m_banner.LoadBannerFromUnlockEntry( &ue );
	}

	ScreenSelectMaster::HandleMessage( msg );
}

/*
 * (c) 2006 Chris Danford
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
