#include "global.h"

#if !defined(WITHOUT_NETWORKING)
#include "ScreenNetRoom.h"
#include "ScreenManager.h"
#include "NetworkSyncManager.h"
#include "GameState.h"
#include "ThemeManager.h"

#define TITLEBG_WIDTH				THEME->GetMetricF(m_sName,"TitleBGWidth")
#define ROOMSBG_WIDTH				THEME->GetMetricF(m_sName,"RoomsBGWidth")

#define	NUM_ROOMS_SHOW				THEME->GetMetricI(m_sName,"NumRoomsShow");

const ScreenMessage SM_SMOnlinePack	= ScreenMessage(SM_User+8);	//Unused, but should be known


ScreenNetRoom::ScreenNetRoom( const CString& sName ) : ScreenNetSelectBase( sName )
{
	GAMESTATE->FinishStage();
	m_soundChangeSel.Load( THEME->GetPathToS("ScreenNetRoom change sel"));

	m_iRoomPlace = 0;

	m_rectTitleBG.SetName( "TitleBG" );
	SET_QUAD_INIT( m_rectTitleBG );
	this->AddChild( &m_rectTitleBG);

	m_textTitle.LoadFromFont( THEME->GetPathF(m_sName,"wheel") );
	m_textTitle.SetShadowLength( 0 );
	m_textTitle.SetName( "Title" );
	m_textTitle.SetMaxWidth( TITLEBG_WIDTH );
	SET_XY_AND_ON_COMMAND( m_textTitle );
	this->AddChild( &m_textTitle);

	m_rectRoomsBG.SetName( "RoomsBG" );
	SET_QUAD_INIT( m_rectRoomsBG );
	this->AddChild( &m_rectRoomsBG);

	m_textRooms.LoadFromFont( THEME->GetPathF(m_sName,"wheel") );
	m_textRooms.SetShadowLength( 0 );
	m_textRooms.SetName( "Rooms" );
	m_textRooms.SetMaxWidth( ROOMSBG_WIDTH );
	SET_XY_AND_ON_COMMAND( m_textRooms );
	this->AddChild( &m_textRooms );

	m_rectRoomsSel.SetName( "Sel" );
	SET_QUAD_INIT( m_rectRoomsSel );
	this->AddChild( &m_rectRoomsSel );
	
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
				m_textTitle.SetText( NSMAN->m_SMOnlinePacket.ReadNT() + '\n' + NSMAN->m_SMOnlinePacket.ReadNT() );
				if ( NSMAN->m_SMOnlinePacket.Read1() != 0 )
				{
					CString SMOnlineSelectScreen;
					THEME->GetMetric( m_sName, "MusicSelectScreen", SMOnlineSelectScreen );
					SCREENMAN->SetNewScreen( SMOnlineSelectScreen );
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
					if (m_iRoomPlace>=m_Rooms.size())
						m_iRoomPlace=m_Rooms.size()-1;
					UpdateRoomsList();
				}
			}
		break;
	}
	ScreenNetSelectBase::HandleScreenMessage( SM );
}

void ScreenNetRoom::TweenOffScreen()
{
	OFF_COMMAND( m_textTitle );
	OFF_COMMAND( m_rectTitleBG );
	OFF_COMMAND( m_rectRoomsBG );
	OFF_COMMAND( m_textRooms );
	OFF_COMMAND( m_rectRoomsSel );

	NSMAN->ReportNSSOnOff(6);
}

void ScreenNetRoom::Update( float fDeltaTime )
{
	ScreenNetSelectBase::Update( fDeltaTime );
}

void ScreenNetRoom::MenuStart( PlayerNumber pn )
{
	NSMAN->m_SMOnlinePacket.ClearPacket();
	NSMAN->m_SMOnlinePacket.Write1( 1 );
	NSMAN->m_SMOnlinePacket.WriteNT( m_Rooms[m_iRoomPlace] );
	NSMAN->SendSMOnline( );
	ScreenNetSelectBase::MenuStart( pn );
}

void ScreenNetRoom::MenuBack( PlayerNumber pn )
{
	TweenOffScreen();

	Back( SM_GoToPrevScreen );

	ScreenNetSelectBase::MenuBack( pn );
}

void ScreenNetRoom::MenuUp( PlayerNumber pn, const InputEventType type )
{
	m_iRoomPlace--;
	if (m_iRoomPlace<0)
		m_iRoomPlace=0;
	if (m_iRoomPlace>=m_Rooms.size())
		m_iRoomPlace=m_Rooms.size()-1;
	UpdateRoomsList();
	ScreenNetSelectBase::MenuUp( pn );
}

void ScreenNetRoom::MenuDown( PlayerNumber pn, const InputEventType type )
{
	m_iRoomPlace++;
	if (m_iRoomPlace<0)
		m_iRoomPlace=0;
	if (m_iRoomPlace>=m_Rooms.size())
		m_iRoomPlace=m_Rooms.size()-1;
	UpdateRoomsList();
	ScreenNetSelectBase::MenuDown( pn );
}

void ScreenNetRoom::UpdateRoomsList()
{
	CString TempText="";
	//It doesn't like this stuff inline
	int min = m_iRoomPlace-NUM_ROOMS_SHOW;
	int max = m_iRoomPlace+NUM_ROOMS_SHOW;
	for (int i=min; i<max; i++ )
	{
		if ( ( i >= 0 ) && ( i < m_Rooms.size() ) )
			TempText += m_Rooms[i] + '\n';
	}
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
