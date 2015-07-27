#ifndef WHEEL_ITEM_BASE_H
#define WHEEL_ITEM_BASE_H

#include "ActorFrame.h"
#include "BitmapText.h"
#include "ThemeMetric.h"
#include "GameConstantsAndTypes.h"
#include "AutoActor.h"

struct WheelItemBaseData;
/** @brief The different types of Wheel Items. */
enum WheelItemDataType 
{
	WheelItemDataType_Generic,	/**< A generic item on the Wheel. */
	WheelItemDataType_Section,	/**< A general section on the Wheel. */
	WheelItemDataType_Song,		/**< A Song on the Wheel. */
	WheelItemDataType_Roulette,	/**< The roulette section on the Wheel. */
	WheelItemDataType_Random,	/**< The random section on the Wheel. */
	WheelItemDataType_Portal,	/**< The portal section on the Wheel. */
	WheelItemDataType_Course,	/**< A Course on the Wheel. */
	WheelItemDataType_Sort,		/**< A generic sorting item on the Wheel. */
	WheelItemDataType_Custom,	/**< A custom item on the Wheel. */
	NUM_WheelItemDataType,
	WheelItemDataType_Invalid
};
LuaDeclareType( WheelItemDataType );

struct WheelItemBaseData
{
	WheelItemBaseData() {}
	WheelItemBaseData( WheelItemDataType type, RString sText, RageColor color );
	virtual ~WheelItemBaseData() {}
	WheelItemDataType m_Type;
	RString		m_sText;
	RageColor	m_color;	// either text color or section background color
};
/** @brief An item on the wheel. */
class WheelItemBase : public ActorFrame
{
public:
	WheelItemBase( RString sType );
	WheelItemBase( const WheelItemBase &cpy );
	virtual void DrawPrimitives();
	virtual WheelItemBase *Copy() const { return new WheelItemBase(*this); }

	void Load( RString sType );
	void DrawGrayBar( Actor& bar );
	void SetExpanded( bool bExpanded ) { m_bExpanded = bExpanded; }

	virtual void LoadFromWheelItemData( const WheelItemBaseData* pWID, int iIndex, bool bHasFocus, int iDrawIndex );

	RageColor m_colorLocked;

	const RString GetText(){ ASSERT(m_pData != nullptr); return m_pData->m_sText; }
	const RageColor GetColor(){ ASSERT(m_pData != nullptr); return m_pData->m_color; }
	WheelItemDataType GetType(){ ASSERT(m_pData != nullptr); return m_pData->m_Type; }
	bool IsLoaded(){ return m_pData != nullptr; }

	// Lua
	void PushSelf( lua_State *L );

protected:
	void SetGrayBar( Actor *pBar ) { m_pGrayBar = pBar; }

	const WheelItemBaseData* m_pData;
	bool m_bExpanded; // if TYPE_SECTION whether this section is expanded

	Actor* m_pGrayBar;
};

#endif

/*
 * (c) 2001-2006 Chris Danford, Chris Gomez, Glenn Maynard, Josh Allen
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
