#include "ScreenEvaluation.h"
#include "NetworkSyncManager.h"
#include "Quad.h"
#include "BitmapText.h"
#include "ScreenMessage.h"
#include "DifficultyIcon.h"
#include "StepsDisplay.h"

class ScreenNetEvaluation: public ScreenEvaluation
{
public:
	virtual void Init();

	// sm-ssc:
	int GetNumActivePlayers(){ return m_iActivePlayers; }

	// Lua
	virtual void PushSelf( lua_State *L );

protected:
	virtual bool MenuLeft( const InputEventPlus &input );
	virtual bool MenuUp( const InputEventPlus &input );
	virtual bool MenuRight( const InputEventPlus &input );
	virtual bool MenuDown( const InputEventPlus &input );
	virtual void HandleScreenMessage( const ScreenMessage SM );
	virtual void TweenOffScreen( );

	void UpdateStats( );
private:
	// todo: Make this an AutoActor -aj
	Quad m_rectUsersBG;

	// todo: Make this a StepsDisplay -aj
	DifficultyIcon m_DifficultyIcon[NUM_PLAYERS];
	//StepsDisplay m_StepsDisplays[NUM_PLAYERS];

	vector<BitmapText>	m_textUsers;
	int	 m_iCurrentPlayer;
	int	 m_iActivePlayers;

	PlayerNumber m_pActivePlayer;

	bool m_bHasStats;

	int m_iShowSide;

	void RedoUserTexts();
};

/*
 * (c) 2004-2005 Charles Lohr, Joshua Allen
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

