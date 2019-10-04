#include "global.h"
#if !defined(WITHOUT_NETWORKING)
#include "RoomInfoDisplay.h"
#include "ActorUtil.h"
#include "NetworkSyncManager.h"
#include "LocalizedString.h"

AutoScreenMessage( SM_RoomInfoRetract );
AutoScreenMessage( SM_RoomInfoDeploy );

static LocalizedString LAST_ROUND_INFO ( "RoomInfoDisplay", "Last Round Info:" );
static LocalizedString ROOM_NAME ( "RoomInfoDisplay", "Room Name:" );
static LocalizedString ROOM_DESC ( "RoomInfoDisplay", "Room Description:" );
static LocalizedString SONG_TITLE( "RoomInfoDisplay", "Song Title:" );
static LocalizedString SONG_SUB_TITLE( "RoomInfoDisplay", "Song Subtitle:" );
static LocalizedString SONG_ARTIST( "RoomInfoDisplay", "Song Artist:" );
static LocalizedString PLAYERS( "RoomInfoDisplay", "Players" );

RoomInfoDisplay::RoomInfoDisplay() :
	m_state( OPEN )
{
	//No code
}

RoomInfoDisplay::~RoomInfoDisplay()
{
	for (size_t i = 0; i < m_playerList.size(); i++)
	{
		this->RemoveChild(m_playerList[i]);
		SAFE_DELETE(m_playerList[i]);
	}
}

void RoomInfoDisplay::DeployInfoBox()
{
	if (m_state == CLOSED)
	{
		ON_COMMAND( this );
		m_state = OPEN;
	}
}
	
void RoomInfoDisplay::RetractInfoBox()
{
	if (m_state == OPEN)
		OFF_COMMAND( this );

	m_state = LOCKED;
}

void RoomInfoDisplay::Load( RString sType )
{
	DEPLOY_DELAY.Load( sType, "DeployDelay" );
	RETRACT_DELAY.Load( sType, "RetractDelay" );
	PLAYERLISTX.Load( sType, "PlayerListElementX" );
	PLAYERLISTY.Load( sType, "PlayerListElementY" );
	PLAYERLISTOFFSETX.Load( sType, "PlayerListElementOffsetX" );
	PLAYERLISTOFFSETY.Load( sType, "PlayerListElementOffsetY" );

	m_bg.Load( THEME->GetPathG(m_sName,"Background") );
	m_bg->SetName( "Background" );
	this->AddChild( m_bg );

	m_Title.LoadFromFont( THEME->GetPathF(sType,"text") );
	m_Title.SetName( "RoomTitle" );
	m_Title.SetShadowLength( 0 );
	m_Title.SetHorizAlign( align_left );
	LOAD_ALL_COMMANDS( m_Title );
	ON_COMMAND( m_Title );
	this->AddChild( &m_Title );

	m_Desc.LoadFromFont( THEME->GetPathF(sType,"text") );
	m_Desc.SetName( "RoomDesc" );
	m_Desc.SetShadowLength( 0 );
	m_Desc.SetHorizAlign( align_left );
	LOAD_ALL_COMMANDS( m_Desc );
	ON_COMMAND( m_Desc );
	this->AddChild( &m_Desc );

	m_lastRound.LoadFromFont( THEME->GetPathF(sType,"text") );
	m_lastRound.SetName( "LastRound" );
	m_lastRound.SetText( LAST_ROUND_INFO.GetValue() );
	m_lastRound.SetShadowLength( 0 );
	m_lastRound.SetHorizAlign( align_left );
	LOAD_ALL_COMMANDS( m_lastRound );
	ON_COMMAND( m_lastRound );
	this->AddChild( &m_lastRound );

	m_songTitle.LoadFromFont( THEME->GetPathF(sType,"text") );
	m_songTitle.SetName( "SongTitle" );
	m_songTitle.SetShadowLength( 0 );
	m_songTitle.SetHorizAlign( align_left );
	LOAD_ALL_COMMANDS( m_songTitle );
	ON_COMMAND( m_songTitle );
	this->AddChild( &m_songTitle );

	m_songSub.LoadFromFont( THEME->GetPathF(sType,"text") );
	m_songSub.SetName( "SongSubTitle" );
	m_songSub.SetShadowLength( 0 );
	m_songSub.SetHorizAlign( align_left );
	LOAD_ALL_COMMANDS( m_songSub );
	ON_COMMAND( m_songSub );
	this->AddChild( &m_songSub );

	m_songArtist.LoadFromFont( THEME->GetPathF(sType,"text") );
	m_songArtist.SetName( "SongArtist" );
	m_songArtist.SetShadowLength( 0 );
	m_songArtist.SetHorizAlign( align_left );
	LOAD_ALL_COMMANDS( m_songArtist );
	ON_COMMAND( m_songArtist );
	this->AddChild( &m_songArtist );

	m_players.LoadFromFont( THEME->GetPathF(sType,"text") );
	m_players.SetName( "Players" );
	m_players.SetShadowLength( 0 );
	m_players.SetHorizAlign( align_left );
	LOAD_ALL_COMMANDS( m_players );
	ON_COMMAND( m_players );
	this->AddChild( &m_players );

	LOAD_ALL_COMMANDS( this );

	this->PlayCommand("Off");
	FinishTweening();

	m_state = LOCKED;
}

void RoomInfoDisplay::SetRoom( const RoomWheelItemData* roomData )
{
	RequestRoomInfo(roomData->m_sText);

	m_Title.SetText( ROOM_NAME.GetValue() + roomData->m_sText );
	m_Desc.SetText( ROOM_DESC.GetValue() + roomData->m_sDesc );
}

void RoomInfoDisplay::Update( float fDeltaTime )
{
	if ((m_deployDelay.PeekDeltaTime() >= DEPLOY_DELAY) && (m_deployDelay.PeekDeltaTime() < (DEPLOY_DELAY + RETRACT_DELAY)))
		DeployInfoBox();
	else if (m_deployDelay.PeekDeltaTime() >= DEPLOY_DELAY + RETRACT_DELAY)
		RetractInfoBox();

	ActorFrame::Update(fDeltaTime);
}

void RoomInfoDisplay::RequestRoomInfo(const RString& name)
{
	NSMAN->m_SMOnlinePacket.ClearPacket();
	NSMAN->m_SMOnlinePacket.Write1((uint8_t)3); //Request Room Info
	NSMAN->m_SMOnlinePacket.WriteNT(name);
	NSMAN->SendSMOnline( );
}

void RoomInfoDisplay::SetRoomInfo( const RoomInfo& info)
{
	m_songTitle.SetText( SONG_TITLE.GetValue() + info.songTitle );
	m_songSub.SetText( SONG_SUB_TITLE.GetValue() + info.songSubTitle );
	m_songArtist.SetText( SONG_ARTIST.GetValue() + info.songArtist );
	m_players.SetText(ssprintf("%s (%d/%d):", PLAYERS.GetValue().c_str(), info.numPlayers, info.maxPlayers));

	if (m_playerList.size() > info.players.size())
	{
		for (size_t i = info.players.size(); i < m_playerList.size(); i++)
		{
			//if our old list is larger remove some elements
			this->RemoveChild(m_playerList[i]);
			SAFE_DELETE(m_playerList[i]);
		}
		m_playerList.resize(info.players.size());
	}
	else if (m_playerList.size() < info.players.size())
	{
		//add elements if our old list is smaller
		int oldsize = m_playerList.size();
		m_playerList.resize(info.players.size());
		for (size_t i = oldsize; i < m_playerList.size(); i++)
		{
			m_playerList[i] = new BitmapText;
			m_playerList[i]->LoadFromFont( THEME->GetPathF(GetName(),"text") );
			m_playerList[i]->SetName("PlayerListElement");
			m_playerList[i]->SetHorizAlign( align_left );
			m_playerList[i]->SetX(PLAYERLISTX + (i * PLAYERLISTOFFSETX));
			m_playerList[i]->SetY(PLAYERLISTY + (i * PLAYERLISTOFFSETY));
			LOAD_ALL_COMMANDS(m_playerList[i]);
			ON_COMMAND(m_playerList[i]);
			this->AddChild(m_playerList[i]);
		}

	}

	for (size_t i = 0; i < m_playerList.size(); i++)
		m_playerList[i]->SetText(info.players[i]);

	m_state = CLOSED;
	m_deployDelay.Touch();
}
#endif

/*
 * (c) 2006 Josh Allen
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
