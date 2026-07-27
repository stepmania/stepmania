#ifndef SCREEN_SELECT_H
#define SCREEN_SELECT_H

#include "ScreenWithMenuElements.h"
#include "GameCommand.h"
#include "ThemeMetric.h"

#include <vector>


/** @brief Base class for Style, Difficulty, and Mode selection screens. */
class ScreenSelect : public ScreenWithMenuElements
{
public:
	virtual void Init();
	virtual void BeginScreen();
	virtual ~ScreenSelect();

	virtual void Update( float fDelta );
	virtual bool Input( const InputEventPlus &input );
	virtual void HandleScreenMessage( const ScreenMessage SM );
	virtual void HandleMessage( const Message &msg );

	virtual bool MenuBack( const InputEventPlus &input );

protected:
	virtual int GetSelectionIndex( PlayerNumber pn ) = 0;
	virtual void UpdateSelectableChoices() = 0; // derived screens must handle this

	/**
	 * @brief The game commands available.
	 *
	 * Derived classes should look here for the choices. */
	std::vector<GameCommand>	m_aGameCommands;

	std::vector<RString>		m_asSubscribedMessages;

	/** @brief Count up to the time between idle comment announcer sounds. */
	RageTimer		m_timerIdleComment;
	/** @brief Count up to go to the timeout screen. */
	RageTimer		m_timerIdleTimeout;

	ThemeMetric<float> IDLE_COMMENT_SECONDS;
	ThemeMetric<float> IDLE_TIMEOUT_SECONDS;
	ThemeMetric<bool> ALLOW_DISABLED_PLAYER_INPUT;
};

#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2004
 * @section LICENSE
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
