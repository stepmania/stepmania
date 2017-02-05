/* RoomWheel - A wheel containing data about rooms. */

#ifndef ROOM_WHEEL_H
#define ROOM_WHEEL_H

#include "WheelBase.h"
#include "WheelItemBase.h"
#include "ThemeMetric.h"

struct RoomWheelItemData : public WheelItemBaseData
{
	RoomWheelItemData() : m_iFlags(0) { }
	RoomWheelItemData( WheelItemDataType type, const RString& sTitle, const RString& sDesc, RageColor color ):
		WheelItemBaseData( type, sTitle, color ), m_sDesc(sDesc), m_iFlags(0) { };

	RString		m_sDesc;
	unsigned int	m_iFlags;
};

class RoomWheelItem : public WheelItemBase
{
public:
	RoomWheelItem( RString sType = "RoomWheelItem" );
	RoomWheelItem( const RoomWheelItem &cpy );

	void Load( RString sType );
	virtual void LoadFromWheelItemData( const WheelItemBaseData* pWID, int iIndex, bool bHasFocus, int iDrawIndex );
	virtual RoomWheelItem *Copy() const { return new RoomWheelItem(*this); }

private:
	AutoActor	m_sprNormalPart;
	AutoActor	m_sprColorPart;
	AutoActor	m_sprOverPart;
	BitmapText	m_text;
	BitmapText	m_Desc;
};

struct RoomInfo
{
	RString songTitle;
	RString songSubTitle;
	RString songArtist;
	int numPlayers;
	int maxPlayers;
	vector<RString> players;
};

class RoomWheel : public WheelBase
{
public:
	virtual ~RoomWheel();
	virtual void Load( RString sType );
	virtual void BuildWheelItemsData( vector<WheelItemBaseData*> &arrayWheelItemDatas );
	virtual unsigned int GetNumItems() const;
	virtual bool Select();
	virtual void Move( int n );

	inline RoomWheelItemData *GetItem( unsigned int i ) { return dynamic_cast<RoomWheelItemData*>( WheelBase::GetItem(i + m_offset) ); }
	void AddPermanentItem( RoomWheelItemData *itemdata );
	int GetCurrentIndex() const { return m_iSelection; }
	int GetPerminateOffset() const { return m_offset; }
	void AddItem( WheelItemBaseData *itemdata );
	void RemoveItem( int index );

	// Lua
	void PushSelf(lua_State *L);

private:
	virtual WheelItemBase *MakeItem();
	int m_offset;
};

#endif

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
