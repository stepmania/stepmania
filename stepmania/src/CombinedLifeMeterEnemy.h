#ifndef CombinedLifeMeterEnemy_H
#define CombinedLifeMeterEnemy_H

#include "CombinedLifeMeter.h"
#include "Sprite.h"


class CombinedLifeMeterEnemy : public CombinedLifeMeter
{
public:
	CombinedLifeMeterEnemy();

	virtual void Update( float fDelta );

	enum Face { normal=0, taunt, attack, damage, defeated, NUM_FACES };

	virtual void ChangeLife( PlayerNumber pn, TapNoteScore score ) {};
	virtual void ChangeLife( PlayerNumber pn, HoldNoteScore score, TapNoteScore tscore ) {};
	virtual void OnDancePointsChange( PlayerNumber pn ) {};
	virtual bool IsInDanger( PlayerNumber pn ) { return false; };
	virtual bool IsHot( PlayerNumber pn ) { return false; };
	virtual bool IsFailing( PlayerNumber pn ) { return false; };
	virtual void OnTaunt();

protected:
	void SetFace( Face face );

	Sprite m_sprHealthStream;
	Sprite	m_sprHealthBackground;
	float m_fLastSeenHealthPercent;

	Sprite m_sprFace;
	float m_fSecondsUntilReturnToNormalFace;

	Sprite m_sprFrame;
};


#endif

/*
 * (c) 2001-2003 Chris Danford
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

