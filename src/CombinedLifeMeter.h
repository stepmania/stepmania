#ifndef COMBINEDLIFEMETER_H
#define COMBINEDLIFEMETER_H

#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"
#include "ActorFrame.h"

/** @brief Multiple Players sharing one LifeMeter. */
class CombinedLifeMeter : public ActorFrame
{
public:
	CombinedLifeMeter() {};
	virtual ~CombinedLifeMeter() {};
	
	virtual void OnLoadSong() {};
	/**
	 * @brief Change life after receiving a tap note grade.
	 *
	 * Note that this *is* called for the head of hold/roll notes.
	 * @param pn the PlayerNumber that hit the TapNote.
	 * @param tns the score received from the TapNote. */
	virtual void ChangeLife( PlayerNumber pn, TapNoteScore tns ) = 0;
	/**
	 * @brief Change life after receiving a hold note grade.
	 * @param pn the PlayerNumber that received the hold note score.
	 * @param hns the hold note score recently received.
	 * @param tns the score from the initial TapNote. */
	virtual void ChangeLife( PlayerNumber pn, HoldNoteScore hns, TapNoteScore tns ) = 0;
	virtual void ChangeLife(PlayerNumber pn, float delta) = 0;
	virtual void SetLife(PlayerNumber pn, float value) = 0;
	virtual void HandleTapScoreNone( PlayerNumber pn ) = 0;
};



#endif

/**
 * @file
 * @author Chris Danford (c) 2003
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
 * 
 * (c) 2016- Electromuis, Anton Grootes
 * This branch of https://github.com/stepmania/stepmania
 * will from here on out be released as GPL v3 (wich converts from the previous MIT license)
 */
