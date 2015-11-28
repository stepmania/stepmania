/* ScreenNetSelectMusic - A method for Online/Net song selection */

#ifndef SCREEN_NET_ROOM_H
#define SCREEN_NET_ROOM_H

#include "ScreenWithMenuElements.h"
#include "ScreenNetSelectBase.h"
#include <vector>
#include "RoomWheel.h"
#include "RoomInfoDisplay.h"

class RoomData {
public:
	void SetName( const std::string& name ) { m_name = name; }
	void SetDescription( const std::string& desc ) { m_description = desc; }
	void SetState(unsigned int state) { m_state = state; }
	void SetFlags( unsigned int iFlags ) { m_iFlags = iFlags; }
	inline std::string Name() { return m_name; }
	inline std::string Description() { return m_description; }
	inline unsigned int State() { return m_state; }
	inline unsigned int GetFlags() { return m_iFlags; }
private:
	std::string m_name;
	std::string m_description;
	unsigned int m_state;
	unsigned int m_iFlags;
};

class ScreenNetRoom : public ScreenNetSelectBase
{
public:
	virtual void Init();
	virtual bool Input( const InputEventPlus &input );
	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:
	virtual bool MenuStart( const InputEventPlus &input );
	virtual bool MenuBack( const InputEventPlus &input );

	virtual void TweenOffScreen( );

private:
	void UpdateRoomsList();
	bool MenuLeft( const InputEventPlus &input );
	bool MenuRight( const InputEventPlus &input );
	void CreateNewRoom( const std::string& rName,  const std::string& rDesc, const std::string& rPass );

	RageSound m_soundChangeSel;

	std::vector<BitmapText> m_RoomList;
	std::vector<RoomData> m_Rooms;
	int m_iRoomPlace;

	std::string m_sLastPickedRoom;

	std::string m_newRoomName, m_newRoomDesc, m_newRoomPass;

	RoomWheel m_RoomWheel;
	RoomInfoDisplay m_roomInfo;
};
#endif

/*
 * (c) 2004 Charles Lohr, Joshua Allen
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
