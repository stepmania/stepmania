#ifndef COMBINEDLIFEMETER_H
#define COMBINEDLIFEMETER_H

#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"
#include "ActorFrame.h"


class CombinedLifeMeter : public ActorFrame
{
public:
	CombinedLifeMeter() {};
	virtual ~CombinedLifeMeter() {};
	
	virtual void OnSongEnded() {};
	/* Change life after receiving a tap note grade.  This *is* called for
	 * the head of hold notes. */
	virtual void ChangeLife( PlayerNumber pn, TapNoteScore score ) = 0;
	/* Change life after receiving a hold note grade.  tscore is the score
	 * received for the initial tap note. */
	virtual void ChangeLife( PlayerNumber pn, HoldNoteScore score, TapNoteScore tscore ) = 0;
	virtual void ChangeLifeMine( PlayerNumber pn ) = 0;
	virtual void OnDancePointsChange( PlayerNumber pn ) = 0;	// look in GAMESTATE and update the display
	virtual bool IsInDanger( PlayerNumber pn ) = 0;
	virtual bool IsHot( PlayerNumber pn ) = 0;
	virtual bool IsFailing( PlayerNumber pn ) = 0;
	virtual void OnTaunt() {};
};



#endif

/*
 * (c) 2003 Chris Danford
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
