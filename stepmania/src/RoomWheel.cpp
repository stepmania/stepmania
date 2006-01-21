#include "global.h"
#include "RoomWheel.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "ScreenTextEntry.h"
#include "LocalizedString.h"
#include "NetworkSyncManager.h"

AutoScreenMessage( SM_BackFromRoomName )

void RoomWheel::Load( CString sType ) 
{
	SetName( sType );
	m_offset = 0;
	LOG->Trace( "RoomWheel::Load('%s')", sType.c_str() );

	LoadFromMetrics( sType );
	LoadVariables();

	FOREACH( WheelItemBase*, m_WheelBaseItems, i )
		SAFE_DELETE( *i );

	m_WheelBaseItems.clear();
	for( int i=0; i<NUM_WHEEL_ITEMS; i++ )
		m_WheelBaseItems.push_back( new RoomWheelItem );

	m_roomInfo.Load("RoomInfoDisplay");
	this->AddChild(&m_roomInfo);

	m_WheelState = STATE_SELECTING;

	AddPerminateItem( new RoomWheelData(TYPE_GENERIC, "Create Room", "Create a new game room", THEME->GetMetricC( m_sName, "CreateRoomColor")) );

	BuildWheelItemsData( m_WheelBaseItemsData );
	RebuildWheelItems();
}

RoomWheelData::RoomWheelData( WheelItemType wit, CString sTitle, CString sDesc, RageColor color ):
	WheelItemBaseData( wit, sTitle, color )
{
	m_sDesc = sDesc;
}

RoomWheelItem::RoomWheelItem( CString sType ):
	WheelItemBase(sType)
{
	SetName( sType );
	Load( sType );
}

void RoomWheelItem::Load(CString sType)
{
	DESC_X				.Load(sType,"DescX");
	DESC_Y				.Load(sType,"DescY");
	DESC_WIDTH			.Load(sType,"DescWidth");
	DESC_ON_COMMAND		.Load(sType,"DescOnCommand");


	m_text.SetHorizAlignString("left");
	TEXT_WIDTH		.Load(sType,"TextWidth");
	m_text.SetMaxWidth(TEXT_WIDTH);

	m_Desc.LoadFromFont( THEME->GetPathF("RoomWheel","text") );
	m_Desc.SetHorizAlignString("left");
	m_Desc.SetShadowLength( 0 );
	m_Desc.SetMaxWidth( DESC_WIDTH );
	m_Desc.SetXY( DESC_X, DESC_Y);
	m_Desc.RunCommands( DESC_ON_COMMAND );
	this->AddChild( &m_Desc );
}

void RoomWheel::BuildWheelItemsData( vector<WheelItemBaseData*> &arrayWheelItemDatas )
{
	if( arrayWheelItemDatas.empty() )
	{
		arrayWheelItemDatas.push_back( new RoomWheelData(TYPE_GENERIC, "- EMPTY -", "", RageColor(1,0,0,1)) );
	}
}

void RoomWheel::AddPerminateItem(RoomWheelData* itemdata)
{
	m_offset++;
	AddItem( itemdata );
}

static LocalizedString ENTER_ROOM_NAME( "RoomWheel", "Enter room name" );
bool RoomWheel::Select()
{
	m_roomInfo.RetractInfoBox();
	if( m_iSelection > 0 )
		return WheelBase::Select();
	else if( m_iSelection == 0 )
	{
		// Since this is not actually an option outside of this wheel, NULL is a good idea.
		m_LastSelection = NULL;
		ScreenTextEntry::TextEntry( SM_BackFromRoomName, ENTER_ROOM_NAME, "", 255 );
	}
	return false;
}

void RoomWheelItem::LoadFromWheelItemBaseData( WheelItemBaseData* pWID )
{
	RoomWheelData* tmpdata = (RoomWheelData*) pWID;
	WheelItemBase::LoadFromWheelItemBaseData( pWID );
	m_Desc.SetText( tmpdata->m_sDesc );
	m_Desc.SetDiffuseColor( pWID->m_color );
	m_text.SetDiffuseColor( pWID->m_color );
}

void RoomWheel::Update( float fDeltaTime )
{
	WheelBase::Update(fDeltaTime);
}

void RoomWheel::Move(int n)
{
	if ((n == 0) && (m_iSelection >= m_offset))
	{
		const RoomWheelData* data = GetItem(m_iSelection-m_offset);
		if (data != NULL)
			m_roomInfo.SetRoom( data );
	}
	else
		m_roomInfo.RetractInfoBox();

	WheelBase::Move(n);
}

unsigned int RoomWheel::GetNumItems() const
{
	return m_WheelBaseItemsData.size() - m_offset;
}

void RoomWheel::RemoveItem( int index )
{
	WheelBase::RemoveItem(index + m_offset);
}

RoomInfoDisplay::~RoomInfoDisplay()
{
	for (int i = 0; i < m_playersList.size(); i++)
	{
		this->RemoveChild(m_playersList[i]);
		SAFE_DELETE(m_playersList[i]);
	}
}

void RoomInfoDisplay::DeployInfoBox()
{
	if (m_state == CLOSED)
	{
		LOG->Info("OPEN");
		SET_XY_AND_ON_COMMAND( this );
		m_state = OPEN;
	}
}
	
void RoomInfoDisplay::RetractInfoBox()
{
	if (m_state == OPEN)
		OFF_COMMAND( this );
	
	m_state = LOCKED;
}

void RoomInfoDisplay::Load( CString sType )
{
	SetName(sType);
	DEPLOY_DELAY.Load(sType, "DeployDelay");
	RETRACT_DELAY.Load(sType, "RetractDelay");
	PLAYERLISTX.Load(sType, "PlayerListElementX");
	PLAYERLISTY.Load(sType, "PlayerListElementY");
	PLAYERLISTOFFSETX.Load(sType, "PlayerListElementOffsetX");
	PLAYERLISTOFFSETY.Load(sType, "PlayerListElementOffsetY");

	m_state = LOCKED;
	m_numPlayers = m_maxPlayers = 0;

	m_bg.SetName("Background");
	m_bg.SetWidth( THEME->GetMetricF(sType,"BackgroundWidth") );
	m_bg.SetHeight( THEME->GetMetricF(sType,"BackgroundHeight") );
	this->AddChild(&m_bg);

	m_Title.LoadFromFont( THEME->GetPathF(sType,"text") );
	m_Title.SetName("RoomTitle");
	m_Title.SetShadowLength( 0 );
	m_Title.SetHorizAlign( align_left );
	SET_XY_AND_ON_COMMAND(m_Title);
	this->AddChild(&m_Title);

	m_Desc.LoadFromFont( THEME->GetPathF(sType,"text") );
	m_Desc.SetName("RoomDesc");
	m_Desc.SetShadowLength( 0 );
	m_Desc.SetHorizAlign( align_left );
	SET_XY_AND_ON_COMMAND(m_Desc);
	this->AddChild(&m_Desc);

	m_lastRound.LoadFromFont( THEME->GetPathF(sType,"text") );
	m_lastRound.SetName("LastRound");
	m_lastRound.SetText("Last Round Info:");
	m_lastRound.SetShadowLength( 0 );
	m_lastRound.SetHorizAlign( align_left );
	SET_XY_AND_ON_COMMAND(m_lastRound);
	this->AddChild(&m_lastRound);

	m_songTitle.LoadFromFont( THEME->GetPathF(sType,"text") );
	m_songTitle.SetName("SongTitle");
	m_songTitle.SetShadowLength( 0 );
	m_songTitle.SetHorizAlign( align_left );
	SET_XY_AND_ON_COMMAND(m_songTitle);
	this->AddChild(&m_songTitle);

	m_songSub.LoadFromFont( THEME->GetPathF(sType,"text") );
	m_songSub.SetName("SongSubTitle");
	m_songSub.SetShadowLength( 0 );
	m_songSub.SetHorizAlign( align_left );
	SET_XY_AND_ON_COMMAND(m_songSub);
	this->AddChild(&m_songSub);

	m_songArtist.LoadFromFont( THEME->GetPathF(sType,"text") );
	m_songArtist.SetName("SongArtist");
	m_songArtist.SetShadowLength( 0 );
	m_songArtist.SetHorizAlign( align_left );
	SET_XY_AND_ON_COMMAND(m_songArtist);
	this->AddChild(&m_songArtist);

	m_players.LoadFromFont( THEME->GetPathF(sType,"text") );
	m_players.SetName("Players");
	m_players.SetShadowLength( 0 );
	m_players.SetHorizAlign( align_left );
	SET_XY_AND_ON_COMMAND(m_players);
	this->AddChild(&m_players);

	SET_XY_AND_ON_COMMAND( this );
	OFF_COMMAND(this);
	StopTweening();
}

void RoomInfoDisplay::SetRoom( const RoomWheelData* roomData )
{
	m_state = CLOSED;
	m_deployDelay.Touch();

	m_Title.SetText(ssprintf("Name: %s", roomData->m_sText.c_str()));
	m_Desc.SetText(ssprintf("Description: %s", roomData->m_sDesc.c_str()));
	m_songTitle.SetText(ssprintf("Title: %s", NULL));
	m_songSub.SetText(ssprintf("Subtitle: %s", NULL));
	m_songArtist.SetText(ssprintf("Artist: %s", NULL));
	m_players.SetText(ssprintf("Players (%d/%d):", m_numPlayers, m_maxPlayers));
}

void RoomInfoDisplay::Update( float fDeltaTime )
{
	if ((m_deployDelay.PeekDeltaTime() >= DEPLOY_DELAY) && (m_deployDelay.PeekDeltaTime() < (DEPLOY_DELAY + RETRACT_DELAY)))
		DeployInfoBox();
	else if (m_deployDelay.PeekDeltaTime() >= DEPLOY_DELAY + RETRACT_DELAY)
		RetractInfoBox();

	ActorFrame::Update(fDeltaTime);
}

void RoomInfoDisplay::RequestRoomInfo()
{
	NSMAN->m_SMOnlinePacket.ClearPacket();
	NSMAN->m_SMOnlinePacket.Write1((uint8_t)3); //Request Room Info
	NSMAN->m_SMOnlinePacket.WriteNT(m_Title.GetText());
	NSMAN->SendSMOnline( );
}

void RoomInfoDisplay::SetRoomInfo( const RoomInfo& info)
{
	m_songTitle.SetText(info.songTitle);
	m_songSub.SetText(info.songSubTitle);
	m_songArtist.SetText(info.songArtist);
	m_numPlayers = info.numPlayers;
	m_maxPlayers = info.maxPlayers;
	vector<CString> players;

	if (m_playersList.size() > info.players.size())
	{
		for (int i = info.players.size(); i < m_playersList.size(); i++)
		{
			//if our old list is larger remove some elements
			this->RemoveChild(m_playersList[i]);
			SAFE_DELETE(m_playersList[i]);
		}
		m_playersList.resize(info.players.size());
	}
	else if (m_playersList.size() < info.players.size())
	{
		//add elements if our old list is smaller
		int oldsize = m_playersList.size();
		m_playersList.resize(info.players.size());
		for (int i = oldsize; i < m_playersList.size(); i++)
		{
			m_playersList[i] = new BitmapText;
			m_playersList[i]->LoadFromFont( THEME->GetPathF(GetName(),"text") );
			m_playersList[i]->SetName("PlayersListElement");
			m_playersList[i]->SetShadowLength( 0 );
			m_playersList[i]->SetHorizAlign( align_left );
			m_playersList[i]->SetX(PLAYERLISTX + (i * PLAYERLISTOFFSETX));
			m_playersList[i]->SetY(PLAYERLISTY + (i * PLAYERLISTOFFSETY));
			ON_COMMAND(m_playersList[i]);
			this->AddChild(&m_players);
		}

	}

	for (int i = 0; i < m_playersList.size(); i++)
		m_playersList[i]->SetText(info.players[i]);
}

/*
 * (c) 2004 Josh Allen
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
