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
#include "VirtualKeyboard.h"
#include "GameState.h"
#include "NetworkSyncManager.h"
#include "ScreenTextEntry.h"
#include "crypto/CryptMD5.h"

REGISTER_SCREEN_CLASS(ScreenSMOnlineLogin);

#define PREV_SCREEN		THEME->GetMetric (m_sName,"PrevScreen")
#define NEXT_SCREEN		THEME->GetMetric (m_sName,"NextScreen")
const ScreenMessage SM_SMOnlinePack	    = ScreenMessage(SM_User+8);
const ScreenMessage SM_PasswordDone	    = ScreenMessage(SM_User+9);
OptionRowData g_ProfileLine[1] = {
	OptionRowData("Profile",false)
};

  ScreenSMOnlineLogin::ScreenSMOnlineLogin(CString sClassName) : ScreenOptions(sClassName) {
	LOG->Trace( "ScreenSMOnlineLogin::ScreenSMOnlineLogin()" );

	g_ProfileLine[0].choices.clear();
	PROFILEMAN->GetLocalProfileNames( g_ProfileLine[0].choices );

	if(!g_ProfileLine[0].choices.size()) {
      SCREENMAN->SystemMessage("You Must Define A Profile!");
      SCREENMAN->SetNewScreen("ScreenProfileOptions");
	}
    else {
	  InitMenu(INPUTMODE_SHARE_CURSOR, g_ProfileLine, 1);
  	  SOUND->PlayMusic( THEME->GetPathToS("ScreenMachineOptions music"));
	  Row &row = *m_Rows.back();
	  BitmapText *bt = row.m_textItems.back();
	  bt->SetText("Login");		//Change "Exit" Text
	}
  }

  void ScreenSMOnlineLogin::ImportOptions() {
	vector<CString> vsProfiles;
	PROFILEMAN->GetLocalProfileIDs( vsProfiles );

	CStringArray::iterator iter;

	iter = find( 
		vsProfiles.begin(),
		vsProfiles.end(),
		PREFSMAN->m_sDefaultLocalProfileID[0] );
	if( iter != vsProfiles.end() )
		m_Rows[0]->SetOneSelection((PlayerNumber) 0, iter - vsProfiles.begin());

	iter = find( 
		vsProfiles.begin(),
		vsProfiles.end(),
		PREFSMAN->m_sDefaultLocalProfileID[1] );
	if( iter != vsProfiles.end() )
	  m_Rows[0]->SetOneSelection((PlayerNumber) 1, iter - vsProfiles.begin());
  }

  void ScreenSMOnlineLogin::ExportOptions() {
	vector<CString> vsProfiles;
	PROFILEMAN->GetLocalProfileIDs( vsProfiles );

	if(GAMESTATE->IsPlayerEnabled((PlayerNumber) 0))
	  PREFSMAN->m_sDefaultLocalProfileID[0] = vsProfiles[m_Rows[0]->GetOneSelection((PlayerNumber) 0)];

	if(GAMESTATE->IsPlayerEnabled((PlayerNumber) 1))
	  PREFSMAN->m_sDefaultLocalProfileID[0] = vsProfiles[m_Rows[0]->GetOneSelection((PlayerNumber) 1)];
  }

  void ScreenSMOnlineLogin::GoToPrevScreen() {
	SCREENMAN->SetNewScreen(PREV_SCREEN);
  }

  void ScreenSMOnlineLogin::GoToNextScreen() {
    PREFSMAN->SaveGlobalPrefsToDisk();
	PROFILEMAN->LoadLocalProfileFromMachine((PlayerNumber) 0);
	PROFILEMAN->LoadLocalProfileFromMachine((PlayerNumber) 1);	//Update Profiles

	if(GAMESTATE->IsPlayerEnabled((PlayerNumber) 0) && GAMESTATE->IsPlayerEnabled((PlayerNumber) 1) &&
	  (GAMESTATE->GetPlayerDisplayName((PlayerNumber) 0) == GAMESTATE->GetPlayerDisplayName((PlayerNumber) 1))) {
	  SCREENMAN->SystemMessage("Each Player Needs A Unique Profile!");
	  SCREENMAN->SetNewScreen("ScreenSMOnlineLogin");
	}
	else {
	  m_iPlayer=0;
	  while(!GAMESTATE->IsPlayerEnabled((PlayerNumber) m_iPlayer))
	 	++m_iPlayer;
      SCREENMAN->Password(SM_PasswordDone, "You are logging on as:\n" + GAMESTATE->GetPlayerDisplayName((PlayerNumber) m_iPlayer) + "\n\nPlease enter your password.", NULL );
	}
  }

  void ScreenSMOnlineLogin::HandleScreenMessage(const ScreenMessage SM) {
    switch(SM) {
      case SM_PasswordDone:
		if(!ScreenTextEntry::s_bCancelledLast)
	      SendLogin(ScreenTextEntry::s_sLastAnswer);
		else
          SCREENMAN->SetNewScreen(PREV_SCREEN);
	  break;

	  case SM_SMOnlinePack:
		int ResponceCode = NSMAN->m_SMOnlinePacket.Read1();
		if (ResponceCode == 0) {
		  int Status = NSMAN->m_SMOnlinePacket.Read1();
		  if(Status == 0) {
			NSMAN->isSMOLoggedIn[m_iPlayer++] = true;
		    if(GAMESTATE->IsPlayerEnabled((PlayerNumber) m_iPlayer))
	          SCREENMAN->Password(SM_PasswordDone, "You are logging on as:\n" + GAMESTATE->GetPlayerDisplayName((PlayerNumber) m_iPlayer) + "\n\nPlease enter you password.", NULL );
			else
			  SCREENMAN->SetNewScreen(NEXT_SCREEN);
		  } 
		  else {
			CString Responce = NSMAN->m_SMOnlinePacket.ReadNT();
			SCREENMAN->Password( SM_PasswordDone, Responce + "\n\nYou are logging on as:\n" + GAMESTATE->GetPlayerDisplayName((PlayerNumber) m_iPlayer) + "\n\nPlease enter you password.", NULL );
		  }
		}
 	  break;
	}
    ScreenOptions::HandleScreenMessage(SM);
  }

  void ScreenSMOnlineLogin::MenuStart(PlayerNumber pn,const InputEventType type) {
    ScreenOptions::MenuStart(pn,type);
  }

  CString ScreenSMOnlineLogin::GetSelectedProfileID() {
	vector<CString> vsProfiles;
	PROFILEMAN->GetLocalProfileIDs( vsProfiles );

	const Row &row = *m_Rows[GetCurrentRow()];
	const int Selection = row.GetOneSharedSelection();
	if( !Selection )
		return "";
	return vsProfiles[ Selection-1 ];
  }

  void ScreenSMOnlineLogin::SendLogin(CString sPassword) {
	CString PlayerName = GAMESTATE->GetPlayerDisplayName((PlayerNumber) m_iPlayer);
	CString HashedName;
	CString PreHashedName;

	unsigned char Output[16];
	const unsigned char *Input = (unsigned char *)sPassword.c_str();

	MD5Context BASE;

	memset(&BASE,0,sizeof(MD5Context));
	memset(&Output,0,16);

	MD5Init( &BASE );


	MD5Update( &BASE, Input, sPassword.length());

	MD5Final( Output, &BASE );

	for (int i = 0; i < 16; i++)
		PreHashedName += ssprintf( "%2X", Output[i] );

	//XXX: Yuck. Convert spaces to 0's better. (will fix soon)
	for (int i = 0; i < 32; i++)
		if ( PreHashedName.c_str()[i] == ' ' )
			HashedName += '0';
		else
			if ( PreHashedName.c_str()[i]=='\0' )
				HashedName += ' ';
			else
				HashedName += PreHashedName.c_str()[i];

	NSMAN->m_SMOnlinePacket.ClearPacket();
	NSMAN->m_SMOnlinePacket.Write1((uint8_t)0);			//Login command
	NSMAN->m_SMOnlinePacket.Write1((uint8_t)m_iPlayer);	//Player
	NSMAN->m_SMOnlinePacket.Write1((uint8_t)0);			//MD5 hash style
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