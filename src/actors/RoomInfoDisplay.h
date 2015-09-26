/* RoomInfoDisplay: Shows information about an online game room. */

#ifndef ROOM_INFO_DISPLAY_H
#define ROOM_INFO_DISPLAY_H

#include "RoomWheel.h"
#include "ActorFrame.h"

class RoomInfoDisplay : public ActorFrame
{
public:
	RoomInfoDisplay();
	~RoomInfoDisplay();
	virtual void Load( RString sType );
	virtual void Update( float fDeltaTime );
	void SetRoom( const RoomWheelItemData* roomData );
	void SetRoomInfo( const RoomInfo& info);
	void DeployInfoBox();
	void RetractInfoBox();
private:
	void RequestRoomInfo(const RString& name);
	enum RoomInfoDisplayState
	{
		OPEN = 0,
		CLOSED,
		LOCKED
	};

	RoomInfoDisplayState m_state;
	AutoActor m_bg;
	BitmapText m_Title;
	BitmapText m_Desc;

	BitmapText m_lastRound;
	BitmapText m_songTitle;
	BitmapText m_songSub;
	BitmapText m_songArtist;

	BitmapText m_players;
	vector<BitmapText*> m_playerList;

	RageTimer m_deployDelay;

	ThemeMetric<float>	X;
	ThemeMetric<float>	Y;
	ThemeMetric<float>	DEPLOY_DELAY;
	ThemeMetric<float>	RETRACT_DELAY;
	ThemeMetric<float>	PLAYERLISTX;
	ThemeMetric<float>	PLAYERLISTY;
	ThemeMetric<float>	PLAYERLISTOFFSETX;
	ThemeMetric<float>	PLAYERLISTOFFSETY;
};

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
