#include "global.h"

#if !defined(WITHOUT_NETWORKING)
#include "ScreenNetRoom.h"
#include "ScreenManager.h"
#include "NetworkSyncManager.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "ScreenTextEntry.h"
#include "ScreenManager.h"
#include "VirtualKeyboard.h"
#include "Command.h"

#define TITLEBG_WIDTH				THEME->GetMetricF(m_sName,"TitleBGWidth")
#define TITLEBG_HEIGHT				THEME->GetMetricF(m_sName,"TitleBGHeight")
#define ROOMSBG_WIDTH				THEME->GetMetricF(m_sName,"RoomsBGWidth")
#define ROOMSBG_HEIGHT				THEME->GetMetricF(m_sName,"RoomsBGHeight")
#define SELECTION_WIDTH				THEME->GetMetricF(m_sName,"SelectionWidth")
#define SELECTION_HEIGHT			THEME->GetMetricF(m_sName,"SelectionHeight")
#define ROOMSPACEX					THEME->GetMetricF(m_sName,"RoomsSpacingX")
#define ROOMSPACEY					THEME->GetMetricF(m_sName,"RoomsSpacingY")
#define ROOMLOWERBOUND				THEME->GetMetricF(m_sName,"RoomsLowerBound")
#define ROOMUPPERBOUND				THEME->GetMetricF(m_sName,"RoomsUpperBound")

const ScreenMessage SM_SMOnlinePack	= ScreenMessage(SM_User+8);	//Unused, but should be known
const ScreenMessage	SM_BackFromRoomName	= ScreenMessage(SM_User+15);
const ScreenMessage	SM_BackFromRoomDesc	= ScreenMessage(SM_User+16);

REGISTER_SCREEN_CLASS( ScreenNetRoom );
ScreenNetRoom::ScreenNetRoom( const CString& sName ) : ScreenNetSelectBase( sName )
{
	GAMESTATE->FinishStage();
}

void ScreenNetRoom::Init()
{
	ScreenNetSelectBase::Init();

	m_soundChangeSel.Load( THEME->GetPathS("ScreenNetRoom","change sel"));

	m_iRoomPlace = 0;
	m_SelectMode = SelectRooms;

	m_sprTitleBG.Load( THEME->GetPathG( m_sName, "TitleBG" ) );
	m_sprTitleBG.SetName( "TitleBG" );
	m_sprTitleBG.SetWidth( TITLEBG_WIDTH );
	m_sprTitleBG.SetHeight( TITLEBG_HEIGHT );
	SET_XY_AND_ON_COMMAND( m_sprTitleBG );
	this->AddChild( &m_sprTitleBG);

	m_textTitle.LoadFromFont( THEME->GetPathF(m_sName,"wheel") );
	m_textTitle.SetShadowLength( 0 );
	m_textTitle.SetName( "Title" );
	m_textTitle.SetMaxWidth( TITLEBG_WIDTH );
	SET_XY_AND_ON_COMMAND( m_textTitle );
	this->AddChild( &m_textTitle);

	m_sprRoomsBG.Load( THEME->GetPathG(m_sName,"List") );
	m_sprRoomsBG.SetName( "RoomsBG" );
	m_sprRoomsBG.SetWidth( ROOMSBG_WIDTH );
	m_sprRoomsBG.SetHeight( ROOMSBG_HEIGHT );
	SET_XY_AND_ON_COMMAND( m_sprRoomsBG );
	this->AddChild( &m_sprRoomsBG);

	m_textRooms.LoadFromFont( THEME->GetPathF(m_sName,"wheel") );
	m_textRooms.SetShadowLength( 0 );
	m_textRooms.SetName( "Rooms" );
	m_textRooms.SetMaxWidth( ROOMSBG_WIDTH );
	SET_XY_AND_ON_COMMAND( m_textRooms );
	this->AddChild( &m_textRooms );

	//SelectionSprite
	m_sprSelection.Load( THEME->GetPathG( m_sName, "Selection" ) );
	m_sprSelection.SetName( "Selection" );
	m_sprSelection.SetWidth( SELECTION_WIDTH );
	m_sprSelection.SetHeight( SELECTION_HEIGHT );
	SET_XY_AND_ON_COMMAND( m_sprSelection );
	this->AddChild( &m_sprSelection );

	//CreateRoom Sprite
	m_sprCreateRoom.Load( THEME->GetPathG( m_sName, "CreateRoom" ) );
	m_sprCreateRoom.SetName( "CreateRoom" );
	SET_XY_AND_ON_COMMAND( m_sprCreateRoom );
	this->AddChild( &m_sprCreateRoom );

	NSMAN->ReportNSSOnOff(7);
}

void ScreenNetRoom::Input( const DeviceInput& DeviceI, const InputEventType type,
								  const GameInput& GameI, const MenuInput& MenuI,
								  const StyleInput& StyleI )
{
	ScreenNetSelectBase::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenNetRoom::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( THEME->GetMetric (m_sName, "PrevScreen") );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( THEME->GetMetric (m_sName, "NextScreen") );
		break;
	case SM_SMOnlinePack:
		if ( NSMAN->m_SMOnlinePacket.Read1() == 1 )
			switch ( NSMAN->m_SMOnlinePacket.Read1() )
			{
			case 0: //Room title Change
				{
					CString titleSub;
					titleSub = NSMAN->m_SMOnlinePacket.ReadNT() + "\n";
					titleSub += NSMAN->m_SMOnlinePacket.ReadNT();
					m_textTitle.SetText( titleSub );
					if ( NSMAN->m_SMOnlinePacket.Read1() != 0 )
					{
						CString SMOnlineSelectScreen;
						THEME->GetMetric( m_sName, "MusicSelectScreen", SMOnlineSelectScreen );
						SCREENMAN->SetNewScreen( SMOnlineSelectScreen );
					}
				}
			case 1: //Rooms list change
				{
					int numRooms = NSMAN->m_SMOnlinePacket.Read1();
					m_Rooms.clear();
					for (int i=0;i<numRooms;i++)
					{
						RoomData tmpRoomData;
						tmpRoomData.SetName(NSMAN->m_SMOnlinePacket.ReadNT());
						tmpRoomData.SetDescription(NSMAN->m_SMOnlinePacket.ReadNT());
						m_Rooms.push_back( tmpRoomData );
					}
					//Abide by protocol and read room status
					for (int i=0;i<numRooms;i++)
						m_Rooms[i].SetState(NSMAN->m_SMOnlinePacket.Read1());

					if (m_iRoomPlace<0)
						m_iRoomPlace=0;
					if( m_iRoomPlace >= (int) m_Rooms.size() )
						m_iRoomPlace=m_Rooms.size()-1;
					UpdateRoomsList();
				}
			}
		break;
	case SM_BackFromRoomName:
		if ( !ScreenTextEntry::s_bCancelledLast ) {
			m_newRoomName = ScreenTextEntry::s_sLastAnswer;
			VIRTUALKB.Reset(VKMODE_PROFILE); // allow all characters
			SCREENMAN->TextEntry( SM_BackFromRoomDesc, "Enter Room Description:", "");
		}
		break;
	case SM_BackFromRoomDesc:
		if ( !ScreenTextEntry::s_bCancelledLast ) {
			m_newRoomDesc = ScreenTextEntry::s_sLastAnswer;
			CreateNewRoom( m_newRoomName, m_newRoomDesc);
		}
		break;
	}
	ScreenNetSelectBase::HandleScreenMessage( SM );
}

void ScreenNetRoom::TweenOffScreen()
{
	OFF_COMMAND( m_textTitle );
	OFF_COMMAND( m_sprTitleBG );
	OFF_COMMAND( m_sprRoomsBG );
	OFF_COMMAND( m_textRooms );
	OFF_COMMAND( m_sprSelection );

	NSMAN->ReportNSSOnOff(6);
}

void ScreenNetRoom::Update( float fDeltaTime )
{
	ScreenNetSelectBase::Update( fDeltaTime );
}

void ScreenNetRoom::MenuStart( PlayerNumber pn )
{
	switch( m_SelectMode )
	{
	case SelectRooms:
		if( m_iRoomPlace < (int) m_RoomList.size() && m_iRoomPlace >= 0 )
		{
			NSMAN->m_SMOnlinePacket.ClearPacket();
			NSMAN->m_SMOnlinePacket.Write1( 1 );
			NSMAN->m_SMOnlinePacket.Write1( 1 ); //Type (enter a room)
			NSMAN->m_SMOnlinePacket.WriteNT( m_RoomList[m_iRoomPlace].GetText() );
			NSMAN->SendSMOnline( );
			ScreenNetSelectBase::MenuStart( pn );
		}
		break;
	case SelectMakeRoom:
		VIRTUALKB.Reset(VKMODE_PROFILE); // allow all characters
		SCREENMAN->TextEntry( SM_BackFromRoomName, "Enter Room Name:", "");
		break;
	};
}

void ScreenNetRoom::MenuBack( PlayerNumber pn )
{
	TweenOffScreen();

	Cancel( SM_GoToPrevScreen );

	ScreenNetSelectBase::MenuBack( pn );
}

void ScreenNetRoom::MenuUp( PlayerNumber pn, const InputEventType type )
{
	switch (m_SelectMode) {
	case SelectRooms:
		if (m_iRoomPlace > 0) {
			m_iRoomPlace--;
			UpdateRoomPos();
		}
		ScreenNetSelectBase::MenuUp( pn );
		break;
	};
}

void ScreenNetRoom::MenuDown( PlayerNumber pn, const InputEventType type )
{
	switch( m_SelectMode )
	{
	case SelectRooms:
		if( m_iRoomPlace+1 < (int) m_RoomList.size() )
		{
			m_iRoomPlace++;
			UpdateRoomPos();
		}
		ScreenNetSelectBase::MenuDown( pn );
		break;
	};
}

void ScreenNetRoom::MenuLeft( PlayerNumber pn, const InputEventType type )
{
	m_soundChangeSel.Play();
	m_SelectMode = (NetSelectModes) ( ( (int)m_SelectMode - 1) % (int)SelectModes);
	if ( (int) m_SelectMode < 0) 
		m_SelectMode = (NetSelectModes) (SelectModes - 1);
	m_sprSelection.StopTweening( );
	COMMAND( m_sprSelection,  ssprintf("To%d", m_SelectMode+1 ) );
}

void ScreenNetRoom::MenuRight( PlayerNumber pn, const InputEventType type )
{
	m_soundChangeSel.Play();
	m_SelectMode = (NetSelectModes) ( ( (int)m_SelectMode + 1) % (int)SelectModes);
	m_sprSelection.StopTweening( );
	COMMAND( m_sprSelection,  ssprintf("To%d", m_SelectMode+1 ) );
}

void ScreenNetRoom::UpdateRoomsList()
{
	float cx = THEME->GetMetricF(m_sName, "RoomsX") - ROOMSPACEX * ( m_iRoomPlace );
	float cy = THEME->GetMetricF(m_sName, "RoomsY") - ROOMSPACEY * ( m_iRoomPlace );

	for (unsigned int i = 0; i < m_RoomList.size(); ++i)
		this->RemoveChild(&m_RoomList[i]);

	m_RoomList.clear();
	m_RoomList.resize(m_Rooms.size());

	for (unsigned int i = 0; i < m_Rooms.size(); ++i) {
		m_RoomList[i].LoadFromFont( THEME->GetPathF(m_sName,"Rooms") );
		m_RoomList[i].SetName( "RoomListEliment" );
		m_RoomList[i].SetShadowLength( 1 );
		m_RoomList[i].SetXY( cx, cy );
		m_RoomList[i].SetText(m_Rooms[i].Name());
		switch (m_Rooms[i].State()) {
		case 0:
			m_RoomList[i].SetDiffuseColor (THEME->GetMetricC( m_sName, "OpenRoomColor"));
			break;
		case 1:
			m_RoomList[i].SetDiffuseColor (THEME->GetMetricC( m_sName, "PasswdRoomColor"));
			break;
		case 2:
			m_RoomList[i].SetDiffuseColor (THEME->GetMetricC( m_sName, "InGameRoomColor"));
			break;
		default:
			m_RoomList[i].SetDiffuseColor (THEME->GetMetricC( m_sName, "OpenRoomColor"));
			break;
		}
		this->AddChild( &m_RoomList[i] );
		if (cy > ROOMLOWERBOUND) {
			m_RoomList[i].StopTweening();
			COMMAND(m_RoomList[i], "RoomsOff");
		}
		cx+=ROOMSPACEX;
		cy+=ROOMSPACEY;
	}
}

void ScreenNetRoom::CreateNewRoom( const CString& rName,  const CString& rDesc ) {
	NSMAN->m_SMOnlinePacket.ClearPacket();
	NSMAN->m_SMOnlinePacket.Write1((uint8_t)2); //Create room command
	NSMAN->m_SMOnlinePacket.Write1(1);  //Type game room
	NSMAN->m_SMOnlinePacket.WriteNT(rName);
	NSMAN->m_SMOnlinePacket.WriteNT(rDesc);
	NSMAN->SendSMOnline( );
}

void ScreenNetRoom::UpdateRoomPos() {
	float cx = THEME->GetMetricF(m_sName, "RoomsX") - ROOMSPACEX * ( m_iRoomPlace );
	float cy = THEME->GetMetricF(m_sName, "RoomsY") - ROOMSPACEY * ( m_iRoomPlace );
	for (unsigned int x = 0; x < m_RoomList.size(); ++x) 
	{
		m_RoomList[x].StopTweening();
		CString Command = ssprintf( "linear,0.2;y,%f;x,%f;", cy, cx );

		if ( ( cy > ROOMLOWERBOUND ) || ( cy < ROOMUPPERBOUND ) )
			Command += THEME->GetMetric( m_sName, "RoomsOffCommand" );
		else
			Command += THEME->GetMetric( m_sName, "RoomsOnCommand" );

		ActorCommands cmds( Command );
		m_RoomList[x].RunCommands( cmds );
		cx += ROOMSPACEX;
		cy += ROOMSPACEY;
	}
}

#endif

/*
 * (c) 2004 Charles Lohr
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
