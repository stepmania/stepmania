/* ScreenMapControllers - Maps device input to instrument buttons. */

#ifndef SCREEN_MAP_CONTROLLERS_H
#define SCREEN_MAP_CONTROLLERS_H

#include "ScreenWithMenuElements.h"
#include "BitmapText.h"
#include "InputMapper.h"
#include "ActorScroller.h"
#include "RageSound.h"

#include <vector>


class ScreenMapControllers : public ScreenWithMenuElements
{
public:
	ScreenMapControllers();
	~ScreenMapControllers();
	virtual void Init();
	virtual void BeginScreen();

	virtual void Update( float fDeltaTime );
	virtual bool Input( const InputEventPlus &input );
	virtual void HandleMessage( const Message &msg );
	virtual void HandleScreenMessage( const ScreenMessage SM );

private:

	Actor *GetActorWithFocus();
	void BeforeChangeFocus();
	void AfterChangeFocus();
	void Refresh();
	void DismissWarning();
	bool CursorOnAction();
	bool CursorOnHeader();
	bool CursorOnKey();
	bool CursorCanGoUp();
	bool CursorCanGoDown();
	bool CursorCanGoLeft();
	bool CursorCanGoRight();
	int CurKeyIndex();
	int CurActionIndex();
	void SetCursorFromSetListCurrent();
	void StartWaitingForPress();

	unsigned int m_CurController;
	unsigned int m_CurButton;
	unsigned int m_CurSlot;
	unsigned int m_MaxDestItem;

	bool m_ChangeOccurred;

	RageTimer m_WaitingForPress;
	DeviceInput m_DeviceIToMap;

	struct KeyToMap
	{
		GameButton m_GameButton;

		// owned by m_Line
		BitmapText	*m_textMappedTo[NUM_GameController][NUM_SHOWN_GAME_TO_DEVICE_SLOTS];
	};
	std::vector<KeyToMap> m_KeysToMap;

	BitmapText m_textDevices;

	BitmapText m_textLabel[NUM_GameController];
	BitmapText m_ListHeaderCenter;
	BitmapText m_ListHeaderLabels[NUM_GameController][NUM_SHOWN_GAME_TO_DEVICE_SLOTS];

	float m_AutoDismissWarningSecs;
	AutoActor m_Warning;

	float m_AutoDismissNoSetListPromptSecs;
	AutoActor m_NoSetListPrompt;

	float m_AutoDismissSanitySecs;
	AutoActor m_SanityMessage;

	struct SetListEntry
	{
		int m_button;
		int m_controller;
		int m_slot;
		SetListEntry(int b, int c, int s)
			:m_button(b), m_controller(c), m_slot(s) {}
		bool operator<(SetListEntry const& rhs) const
		{
			if(m_controller != rhs.m_controller)
			{
				return m_controller < rhs.m_controller;
			}
			if(m_button != rhs.m_button)
			{
				return m_button < rhs.m_button;
			}
			return m_slot < rhs.m_slot;
		}
	};
	std::set<SetListEntry> m_SetList;
	std::set<SetListEntry>::iterator m_SetListCurrent;
	bool m_InSetListMode;

	typedef void (ScreenMapControllers::* action_fun_t)();
	struct ActionRow
	{
		RString m_name;
		AutoActor m_actor;
		action_fun_t m_action;
		void Load(RString const& scr_name, RString const& name,
			ScreenMapControllers::action_fun_t action, ActorFrame* line,
			ActorScroller* scroller);
	};
	void ClearToDefault();
	void ReloadFromDisk();
	void SaveToDisk();
	void SetListMode();
	void ExitAction();
	bool SanityCheckWrapper();

	std::vector<ActionRow> m_Actions;

	std::vector<ActorFrame*> m_Line;
	ActorScroller m_LineScroller;

	RageSound m_soundChange;
	RageSound m_soundDelete;
};

#endif

/*
 * (c) 2001-2004 Chris Danford
 * 2014 Eric Reese
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
