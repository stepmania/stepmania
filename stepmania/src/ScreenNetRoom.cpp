#include "global.h"

#if !defined(WITHOUT_NETWORKING)
#include "ScreenNetRoom.h"
#include "ScreenManager.h"
#include "NetworkSyncManager.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "ScreenTextEntry.h"
#include "Command.h"
#include "WheelItemBase.h"
//#include "RageLog.h"
#include "InputEventPlus.h"

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

AutoScreenMessage( SM_SMOnlinePack )
AutoScreenMessage( SM_BackFromRoomName )
AutoScreenMessage( SM_BackFromRoomDesc )

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

	m_RoomWheel.Load("RoomWheel");
	m_RoomWheel.SetName( "RoomWheel" );
	SET_XY( m_RoomWheel );
	m_RoomWheel.TweenOnScreen();
	m_RoomWheel.SetDrawOrder(1);
	this->AddChild( &m_RoomWheel );
	this->MoveToHead( &m_RoomWheel );
	ON_COMMAND( m_RoomWheel );

	NSMAN->ReportNSSOnOff(7);
}

void ScreenNetRoom::Input( const InputEventPlus &input )
{
	if (((input.MenuI.button == MENU_BUTTON_LEFT) || (input.MenuI.button == MENU_BUTTON_RIGHT)) && (input.type == IET_RELEASE))
		m_RoomWheel.Move(0);
		
	ScreenNetSelectBase::Input( input );
}

static LocalizedString ENTER_ROOM_DESCRIPTION ("ScreenNetRoom","Enter a description for the room:");
void ScreenNetRoom::HandleScreenMessage( const ScreenMessage SM )
{
	if( SM == SM_GoToPrevScreen )
	{
		SCREENMAN->SetNewScreen( THEME->GetMetric (m_sName, "PrevScreen") );
	}
	else if( SM == SM_GoToNextScreen )
	{
		SCREENMAN->SetNewScreen( THEME->GetMetric (m_sName, "NextScreen") );
	}
	else if( SM == SM_SMOnlinePack )
	{
		if ( NSMAN->m_SMOnlinePacket.Read1() == 1 )
		{
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
		}
	}
	else if ( SM == SM_BackFromRoomName )
	{
		if ( !ScreenTextEntry::s_bCancelledLast )
		{
			m_newRoomName = ScreenTextEntry::s_sLastAnswer;
			ScreenTextEntry::TextEntry( SM_BackFromRoomDesc, ENTER_ROOM_DESCRIPTION, "", 255 );
		}
	}
	else if( SM == SM_BackFromRoomDesc )
	{
		if ( !ScreenTextEntry::s_bCancelledLast )
		{
			m_newRoomDesc = ScreenTextEntry::s_sLastAnswer;
			CreateNewRoom( m_newRoomName, m_newRoomDesc);
		}
	}
	ScreenNetSelectBase::HandleScreenMessage( SM );
}

void ScreenNetRoom::TweenOffScreen()
{
	OFF_COMMAND( m_textTitle );
	OFF_COMMAND( m_sprTitleBG );

	NSMAN->ReportNSSOnOff(6);
}

/*
void ScreenNetRoom::Update( float fDeltaTime )
{
	ScreenNetSelectBase::Update( fDeltaTime );
}
*/

void ScreenNetRoom::MenuStart( PlayerNumber pn )
{
	NSMAN->m_SMOnlinePacket.ClearPacket();
	NSMAN->m_SMOnlinePacket.Write1( 1 );
	NSMAN->m_SMOnlinePacket.Write1( 1 ); //Type (enter a room)
	m_RoomWheel.Select();
	if (m_RoomWheel.LastSelected() != NULL)
		NSMAN->m_SMOnlinePacket.WriteNT( m_RoomWheel.LastSelected()->m_sText );
	NSMAN->SendSMOnline( );
	ScreenNetSelectBase::MenuStart( pn );
}

void ScreenNetRoom::MenuBack( PlayerNumber pn )
{
	TweenOffScreen();

	Cancel( SM_GoToPrevScreen );

	ScreenNetSelectBase::MenuBack( pn );
}

void ScreenNetRoom::MenuUp( const InputEventPlus &input )
{
	ScreenNetSelectBase::MenuUp( input.MenuI.player );
}

void ScreenNetRoom::MenuDown( const InputEventPlus &input )
{
	ScreenNetSelectBase::MenuDown( input.MenuI.player );
}

void ScreenNetRoom::MenuLeft( const InputEventPlus &input )
{
	if( input.type == IET_FIRST_PRESS )
		m_RoomWheel.Move(-1);

	ScreenNetSelectBase::MenuLeft( input.MenuI.player );
}

void ScreenNetRoom::MenuRight( const InputEventPlus &input )
{
	if( input.type == IET_FIRST_PRESS )
	{
		m_RoomWheel.Move(1);
	}

	ScreenNetSelectBase::MenuRight( input.MenuI.player );
}

void ScreenNetRoom::UpdateRoomsList()
{
	int difference = 0;
	RoomWheelData* itemData = NULL;

	difference = m_RoomWheel.GetNumItems() - m_Rooms.size();

	if (!m_RoomWheel.IsEmpty())
	{
		if (difference > 0)
			for( int x = 0; x < difference; x++ )
				m_RoomWheel.RemoveItem(m_RoomWheel.GetNumItems() - 1);
		else
		{
			difference = abs(difference);
			for( int x = 0; x < difference; x++ )
				m_RoomWheel.AddItem(new RoomWheelData(TYPE_GENERIC, "", "", RageColor(1,1,1,1)));
		}
	}
	else
	{
		for (unsigned int x = 0; x < m_Rooms.size(); x++)
				m_RoomWheel.AddItem(new RoomWheelData(TYPE_GENERIC, "", "", RageColor(1,1,1,1)));
	}

	for (unsigned int i = 0; i < m_Rooms.size(); ++i)
	{
		itemData = m_RoomWheel.GetItem(i);

		itemData->m_sText = m_Rooms[i].Name();
		itemData->m_sDesc = m_Rooms[i].Description();

		switch (m_Rooms[i].State())
		{
		case 0:
			itemData->m_color = THEME->GetMetricC( m_sName, "OpenRoomColor");
			break;
		case 1:
			itemData->m_color = THEME->GetMetricC( m_sName, "PasswdRoomColor");
			break;
		case 2:
			itemData->m_color = THEME->GetMetricC( m_sName, "InGameRoomColor");
			break;
		default:
			itemData->m_color = THEME->GetMetricC( m_sName, "OpenRoomColor");
			break;
		}
	}

	m_RoomWheel.RebuildWheelItems();
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
 * (c) 2004 Charles Lohr, Josh Allen
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
