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

#define TITLEBG_WIDTH				THEME->GetMetricF(m_sName,"TitleBGWidth")
#define TITLEBG_HEIGHT				THEME->GetMetricF(m_sName,"TitleBGHeight")
#define ROOMSBG_WIDTH				THEME->GetMetricF(m_sName,"RoomsBGWidth")
#define ROOMSBG_HEIGHT				THEME->GetMetricF(m_sName,"RoomsBGHeight")
#define SELECTION_WIDTH				THEME->GetMetricF(m_sName,"SelectionWidth")
#define SELECTION_HEIGHT			THEME->GetMetricF(m_sName,"SelectionHeight")

#define	NUM_ROOMS_SHOW				THEME->GetMetricI(m_sName,"NumRoomsShow");

const ScreenMessage SM_SMOnlinePack	= ScreenMessage(SM_User+8);	//Unused, but should be known
const ScreenMessage	SM_BackFromRoomName	= ScreenMessage(SM_User+15);
const ScreenMessage	SM_BackFromRoomDesc	= ScreenMessage(SM_User+16);

REGISTER_SCREEN_CLASS( ScreenNetRoom );
ScreenNetRoom::ScreenNetRoom( const CString& sName ) : ScreenNetSelectBase( sName )
{
	GAMESTATE->FinishStage();
	m_soundChangeSel.Load( THEME->GetPathToS("ScreenNetRoom change sel"));

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
						m_Rooms.push_back( NSMAN->m_SMOnlinePacket.ReadNT() );
						NSMAN->m_SMOnlinePacket.ReadNT();
					}
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
	switch (m_SelectMode) {
	case SelectRooms:
		NSMAN->m_SMOnlinePacket.ClearPacket();
		NSMAN->m_SMOnlinePacket.Write1( 1 );
		NSMAN->m_SMOnlinePacket.Write1( 1 ); //Type (enter a room)
		NSMAN->m_SMOnlinePacket.WriteNT( m_Rooms[m_iRoomPlace] );
		NSMAN->SendSMOnline( );
		ScreenNetSelectBase::MenuStart( pn );
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

	Back( SM_GoToPrevScreen );

	ScreenNetSelectBase::MenuBack( pn );
}

void ScreenNetRoom::MenuUp( PlayerNumber pn, const InputEventType type )
{
	switch (m_SelectMode) {
	case SelectRooms:
		m_iRoomPlace--;
		if (m_iRoomPlace<0)
			m_iRoomPlace=0;
		if( m_iRoomPlace >= (int) m_Rooms.size() )
			m_iRoomPlace=m_Rooms.size()-1;
		UpdateRoomsList();
		ScreenNetSelectBase::MenuUp( pn );
		break;
	};
}

void ScreenNetRoom::MenuDown( PlayerNumber pn, const InputEventType type )
{
	switch (m_SelectMode) {
	case SelectRooms:
		m_iRoomPlace++;
		if (m_iRoomPlace<0)
			m_iRoomPlace=0;
		if( m_iRoomPlace >= (int) m_Rooms.size() )
			m_iRoomPlace=m_Rooms.size()-1;
		UpdateRoomsList();
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
	CString TempText="";
	//It doesn't like this stuff inline
	int min = m_iRoomPlace-NUM_ROOMS_SHOW;
	int max = m_iRoomPlace+NUM_ROOMS_SHOW;
	for (int i=min; i<max; i++ )
	{
		if( i >= 0 && i < (int) m_Rooms.size() )
			TempText += m_Rooms[i] + '\n';
		else
			TempText += '\n';
	}
	m_textRooms.SetText( TempText );
}

void ScreenNetRoom::CreateNewRoom( const CString& rName,  const CString& rDesc ) {
	NSMAN->m_SMOnlinePacket.ClearPacket();
	NSMAN->m_SMOnlinePacket.Write1((uint8_t)2); //Create room command
	NSMAN->m_SMOnlinePacket.Write1(1);  //Type game room
	NSMAN->m_SMOnlinePacket.WriteNT(rName);
	NSMAN->m_SMOnlinePacket.WriteNT(rDesc);
	NSMAN->SendSMOnline( );
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
