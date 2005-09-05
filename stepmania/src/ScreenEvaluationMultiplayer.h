#ifndef ScreenEvaluationMultiplayer_H
#define ScreenEvaluationMultiplayer_H

#include "ScreenWithMenuElements.h"

class MultiplayerEvalScoreRow;

enum JudgeLine
{
	JudgeLine_Marvelous,
	JudgeLine_Perfect,
	JudgeLine_Great,
	JudgeLine_Good,
	JudgeLine_Boo,
	JudgeLine_Miss,
	JudgeLine_Ok,
	NUM_JudgeLine,
	JudgeLine_INVALID
};
#define FOREACH_JudgeLine( i ) FOREACH_ENUM( JudgeLine, NUM_JudgeLine, i )
const CString& JudgeLineToString( JudgeLine i );

class ScreenEvaluationMultiplayer : public ScreenWithMenuElements
{
public:
	ScreenEvaluationMultiplayer( CString sName );
	~ScreenEvaluationMultiplayer();
	virtual void Init();

	virtual void Input( const InputEventPlus &input );
	virtual void HandleScreenMessage( const ScreenMessage SM );
	virtual void Update(float f);
	virtual void DrawPrimitives();

protected:
	void MenuBack( PlayerNumber pn );
	void MenuStart( PlayerNumber pn );

	ActorFrame m_frameLabels;
	AutoActor m_sprJudgmentLabel[NUM_JudgeLine];

	vector<AutoActor>		m_vsprBullet;
	vector<MultiplayerEvalScoreRow*>	m_vpMultiplayerEvalScoreRow;
};

#endif

/*
 * (c) 2005 Chris Danford
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

