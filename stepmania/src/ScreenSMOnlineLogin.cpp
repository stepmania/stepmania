#if !defined(WITHOUT_NETWORKING)
#include "global.h"
#include "ScreenSMOnlineLogin.h"
#include "RageLog.h"
#include "ProfileManager.h"
#include "GameSoundManager.h"
#include "ThemeManager.h"
#include "PrefsManager.h"
#include "ScreenManager.h"
#include "ScreenTextEntry.h"
#include "ScreenPrompt.h"
#include "GameState.h"
#include "NetworkSyncManager.h"
#include "ScreenTextEntry.h"

REGISTER_SCREEN_CLASS(ScreenSMOnlineLogin);

#define PREV_SCREEN		THEME->GetMetric (m_sName,"PrevScreen")
#define NEXT_SCREEN		THEME->GetMetric (m_sName,"NextScreen")

AutoScreenMessage( SM_SMOnlinePack )
AutoScreenMessage( SM_PasswordDone )

OptionRowDefinition g_ProfileLine[1] = {
	OptionRowDefinition("Profile",false)
};

ScreenSMOnlineLogin::ScreenSMOnlineLogin(CString sClassName) : ScreenOptions(sClassName)
{
	LOG->Trace( "ScreenSMOnlineLogin::ScreenSMOnlineLogin()" );
}

void ScreenSMOnlineLogin::Init()
{
	ScreenOptions::Init();

	g_ProfileLine[0].choices.clear();
	PROFILEMAN->GetLocalProfileNames( g_ProfileLine[0].choices );

	if(!g_ProfileLine[0].choices.size())
	{
		SCREENMAN->SystemMessage("You Must Define A Profile!");
		SCREENMAN->SetNewScreen("ScreenProfileOptions");
	}
    else
	{
		FOREACH_PlayerNumber( pn )
			g_ProfileLine[0].m_vEnabledForPlayers.insert( pn );

		vector<OptionRowDefinition> vDefs( &g_ProfileLine[0], &g_ProfileLine[ARRAYSIZE(g_ProfileLine)] );
		vector<OptionRowHandler*> vHands( vDefs.size(), NULL );

		InitMenu( INPUTMODE_SHARE_CURSOR, vDefs, vHands );
  		SOUND->PlayMusic( THEME->GetPathS("ScreenMachineOptions", "music"));
		OptionRow &row = *m_pRows.back();
		row.SetExitText("Login");
	}
}

void ScreenSMOnlineLogin::ImportOptions( int row, const vector<PlayerNumber> &vpns )
{
	switch( row )
	{
	case 0:
		{
			vector<CString> vsProfiles;
			PROFILEMAN->GetLocalProfileIDs( vsProfiles );

			CStringArray::iterator iter;

			FOREACH_PlayerNumber( pn )
			{
				iter = find(vsProfiles.begin(), vsProfiles.end(), PREFSMAN->GetDefaultLocalProfileID(pn).Get() );
				if( iter != vsProfiles.end() )
					m_pRows[0]->SetOneSelection((PlayerNumber) pn, iter - vsProfiles.begin());
			}
		}
		break;
	}
}

void ScreenSMOnlineLogin::ExportOptions( int row, const vector<PlayerNumber> &vpns )
{
	switch( row )
	{
	case 0:
		{
			vector<CString> vsProfiles;
			PROFILEMAN->GetLocalProfileIDs( vsProfiles );

			FOREACH_EnabledPlayer( pn )
				PREFSMAN->GetDefaultLocalProfileID(pn).Set( vsProfiles[m_pRows[0]->GetOneSelection(pn)] );
		}
		break;
	}
}

void ScreenSMOnlineLogin::GoToPrevScreen()
{
	SCREENMAN->SetNewScreen(PREV_SCREEN);
}

void ScreenSMOnlineLogin::GoToNextScreen()
{
	vector<PlayerNumber> v;
	v.push_back( GAMESTATE->m_MasterPlayerNumber );
	for( unsigned r=0; r<m_pRows.size(); r++ )
		ExportOptions( r, v );

	PREFSMAN->SaveGlobalPrefsToDisk();
	FOREACH_EnabledPlayer(pn)
	{
		PROFILEMAN->LoadLocalProfileFromMachine((PlayerNumber) pn);
	}

	if(GAMESTATE->IsPlayerEnabled((PlayerNumber) 0) && GAMESTATE->IsPlayerEnabled((PlayerNumber) 1) &&
		(GAMESTATE->GetPlayerDisplayName((PlayerNumber) 0) == GAMESTATE->GetPlayerDisplayName((PlayerNumber) 1)))
	{
		SCREENMAN->SystemMessage("Each Player Needs A Unique Profile!");
		SCREENMAN->SetNewScreen("ScreenSMOnlineLogin");
	}
	else
	{
		m_iPlayer=0;
		while(!GAMESTATE->IsPlayerEnabled((PlayerNumber) m_iPlayer))
			++m_iPlayer;
		ScreenTextEntry::Password(SM_PasswordDone, "You are logging on as:\n" + GAMESTATE->GetPlayerDisplayName((PlayerNumber) m_iPlayer) + "\n\nPlease enter your password.", NULL );
	}
}

void ScreenSMOnlineLogin::HandleScreenMessage(const ScreenMessage SM)
{
	if( SM == SM_PasswordDone )
	{
		if(!ScreenTextEntry::s_bCancelledLast)
			SendLogin(ScreenTextEntry::s_sLastAnswer);
		else
			SCREENMAN->SetNewScreen(PREV_SCREEN);
	}
	else if( SM == SM_SMOnlinePack )
	{
		int ResponceCode = NSMAN->m_SMOnlinePacket.Read1();
		if (ResponceCode == 0)
		{
			int Status = NSMAN->m_SMOnlinePacket.Read1();
			if(Status == 0)
			{
				NSMAN->isSMOLoggedIn[m_iPlayer] = true;
				m_iPlayer++;
				if( GAMESTATE->IsPlayerEnabled((PlayerNumber) m_iPlayer) && m_iPlayer < NUM_PLAYERS )
					ScreenTextEntry::Password(SM_PasswordDone, "You are logging on as:\n" + GAMESTATE->GetPlayerDisplayName((PlayerNumber) m_iPlayer) + "\n\nPlease enter your password.", NULL );
				else
					SCREENMAN->SetNewScreen(NEXT_SCREEN);
			}
			else
			{
				CString Responce = NSMAN->m_SMOnlinePacket.ReadNT();
				ScreenTextEntry::Password( SM_PasswordDone, Responce + "\n\nYou are logging on as:\n" + GAMESTATE->GetPlayerDisplayName((PlayerNumber) m_iPlayer) + "\n\nPlease enter your password.", NULL );
			}
		}
	}
	ScreenOptions::HandleScreenMessage(SM);
}

void ScreenSMOnlineLogin::MenuStart(PlayerNumber pn,const InputEventType type)
{
	ScreenOptions::MenuStart(pn,type);
}

CString ScreenSMOnlineLogin::GetSelectedProfileID()
{
	vector<CString> vsProfiles;
	PROFILEMAN->GetLocalProfileIDs( vsProfiles );

	const OptionRow &row = *m_pRows[GetCurrentRow()];
	const int Selection = row.GetOneSharedSelection();
	if( !Selection )
		return "";
	return vsProfiles[ Selection-1 ];
}

void ScreenSMOnlineLogin::SendLogin(CString sPassword)
{
	CString PlayerName = GAMESTATE->GetPlayerDisplayName((PlayerNumber) m_iPlayer);

	CString HashedName = NSMAN->MD5Hex( sPassword );

	int authMethod = 0;
	if ( NSMAN->GetSMOnlineSalt() != 0 )
	{
		authMethod = 1;
		HashedName = NSMAN->MD5Hex( HashedName + ssprintf( "%d", NSMAN->GetSMOnlineSalt() ) );
	}

	NSMAN->m_SMOnlinePacket.ClearPacket();
	NSMAN->m_SMOnlinePacket.Write1((uint8_t)0);			//Login command
	NSMAN->m_SMOnlinePacket.Write1((uint8_t)m_iPlayer);	//Player
	NSMAN->m_SMOnlinePacket.Write1((uint8_t)authMethod);			//MD5 hash style
	NSMAN->m_SMOnlinePacket.WriteNT(PlayerName);
	NSMAN->m_SMOnlinePacket.WriteNT(HashedName);
	NSMAN->SendSMOnline( );
}

#endif

/*
 * (c) 2004 Charles Lohr Adam Lowman
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
