/* WheelBase - A wheel with data eliments. */

#ifndef WHEELBASE_H
#define WHEELBASE_H

#include "AutoActor.h"
#include "ActorFrame.h"
#include "RageSound.h"
#include "GameConstantsAndTypes.h"
#include "ScreenMessage.h"
#include "ScrollBar.h"
#include "RageTimer.h"
#include "WheelItemBase.h"
#include "ThemeMetric.h"

//AutoScreenMessage( SM_SongChanged );		// TODO: Replace this with a Message and MESSAGEMAN

//struct CompareSongPointerArrayBySectionName;

class WheelBase : public ActorFrame
{
public:
	WheelBase();
	virtual ~WheelBase();
	virtual void Load( CString sType );

	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	void DrawItem( int index );

	virtual void TweenOnScreen(bool changing_sort);
	virtual void TweenOffScreen(bool changing_sort);
	virtual void TweenOnScreen() { TweenOnScreen(false); }
	virtual void TweenOffScreen() { TweenOffScreen(false); }

	void Move(int n);
	virtual void ChangeMusic(int dist); /* +1 or -1 */

	/* Return true if we're moving fast automatically. */
	int IsMoving() const;
	bool IsSettled() const;

	void GetItemPosition( float fPosOffsetsFromMiddle, float& fX_out, float& fY_out, float& fZ_out, float& fRotationX_out );
	void SetItemPosition( Actor &item, float fPosOffsetsFromMiddle );

	bool Select();	// return true if this selection ends the screen

	bool WheelIsLocked() { return (m_WheelState == STATE_LOCKED ? true : false); }
	void RebuildWheelItems( int dist = -999999 );	// -999999 = refresh all

	void AddItem( WheelItemBaseData* itemdata );
	void RemoveItem( int index );
	WheelItemBaseData* LastSelected();

protected:
	virtual void LoadFromMetrics( CString sType );
	void LoadVariables();
	void CommonUpdateProcedure(float fDeltaTime);

	virtual void BuildWheelItemsData( vector<WheelItemBaseData *> &arrayWheelItemDatas );
//	bool SelectModeMenuItem();
	int FirstVisibleIndex();

	ScrollBar			m_ScrollBar;
	AutoActor			m_sprHighlight;

	vector<WheelItemBaseData *> m_WheelBaseItemsData;
	vector<WheelItemBase *> m_WheelBaseItems;
	WheelItemBaseData* m_LastSelection;
	
	bool				m_isEmpty;
	int					m_iSelection;		// index into m_CurWheelItemBaseData
	CString				m_sExpandedSectionName;


	int					m_iSwitchesLeftInSpinDown;		
	float				m_fLockedWheelVelocity;
	/* 0 = none; -1 or 1 = up/down */
	int					m_Moving;
	RageTimer			m_MovingSoundTimer;
	float				m_TimeBeforeMovingBegins;
	float				m_SpinSpeed;
	enum WheelState { 
		STATE_SELECTING_GENERIC,
		STATE_SELECTING_MUSIC,
		STATE_FLYING_OFF_BEFORE_NEXT_SORT, 
		STATE_FLYING_ON_AFTER_NEXT_SORT, 
		STATE_TWEENING_ON_SCREEN, 
		STATE_TWEENING_OFF_SCREEN, 
		STATE_WAITING_OFF_SCREEN,
		STATE_ROULETTE_SPINNING,
		STATE_ROULETTE_SLOWING_DOWN,
		STATE_RANDOM_SPINNING,
		STATE_LOCKED,
	};
	WheelState			m_WheelState;
	float				m_fTimeLeftInState;
	float				m_fPositionOffsetFromSelection;

	RageSound m_soundChangeMusic;
	RageSound m_soundExpand;
	RageSound m_soundLocked;

//	bool WheelItemIsVisible(int n);
	void UpdateScrollbar();

	ThemeMetric<float>	SWITCH_SECONDS;
	ThemeMetric<float> LOCKED_INITIAL_VELOCITY;
	ThemeMetric<float> SCROLL_BAR_X;
	ThemeMetric<int> SCROLL_BAR_HEIGHT;
	ThemeMetric<float>	ITEM_CURVE_X;
	ThemeMetric<bool> USE_LINEAR_WHEEL;
	ThemeMetric<float>	ITEM_SPACING_Y;
	ThemeMetric<float>	WHEEL_3D_RADIUS;
	ThemeMetric<float>	CIRCLE_PERCENT;
	ThemeMetric<bool>	USE_3D;
	ThemeMetric<float>	NUM_WHEEL_ITEMS_TO_DRAW;
	ThemeMetric<float> WHEEL_ITEM_ON_DELAY_CENTER;
	ThemeMetric<float> WHEEL_ITEM_ON_DELAY_OFFSET;
	ThemeMetric<float> WHEEL_ITEM_OFF_DELAY_CENTER;
	ThemeMetric<float> WHEEL_ITEM_OFF_DELAY_OFFSET;
//	ThemeMetric1D<RageColor> SECTION_COLORS;
};

#endif

/*
 * (c) 2001-2004 Chris Danford, Chris Gomez, Glenn Maynard, Josh Allen
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
