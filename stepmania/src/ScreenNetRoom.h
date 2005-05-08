/* ScreenNetSelectMusic - A method for Online/Net song selection */

#ifndef SCREENNETROOM_H
#define SCREENNETROOM_H

#include "ScreenWithMenuElements.h"
#include "ScreenNetSelectBase.h"
#include <vector>
#include "RoomWheel.h"

class RoomData {
public:
	void SetName( const CString& name ) { m_name = name; }
	void SetDescription( const CString& desc ) { m_description = desc; }
	void SetState(unsigned int state) { m_state = state; }
	inline CString Name() { return m_name; }
	inline CString Description() { return m_description; }
	inline unsigned int State() { return m_state; }
private:
	CString m_name;
	CString m_description;
	unsigned int m_state;
};

class ScreenNetRoom : public ScreenNetSelectBase
{
public:
	ScreenNetRoom( const CString& sName );
	virtual void Init();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type,
						const GameInput& GameI, const MenuInput& MenuI,
						const StyleInput& StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:
	virtual void MenuStart( PlayerNumber pn );
	virtual void MenuUp( PlayerNumber pn, const InputEventType type );
	virtual void MenuDown( PlayerNumber pn, const InputEventType type );
	virtual void MenuBack( PlayerNumber pn );

	virtual void TweenOffScreen( );
	virtual void Update( float fDeltaTime );

private:
	void UpdateRoomsList();
	void MenuLeft( PlayerNumber pn, const InputEventType type );
	void MenuRight( PlayerNumber pn, const InputEventType type );
	void CreateNewRoom( const CString& rName,  const CString& rDesc );
		
	RageSound m_soundChangeSel;
	
	BitmapText m_textTitle;
	Sprite m_sprTitleBG;

	vector < BitmapText > m_RoomList;
	vector < RoomData > m_Rooms;
	int m_iRoomPlace;

	CString m_newRoomName, m_newRoomDesc;

	RoomWheel m_RoomWheel;
};
#endif

/*
 * (c) 2004 Charles Lohr
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
