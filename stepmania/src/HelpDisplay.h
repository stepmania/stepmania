#ifndef HelpDisplay_H
#define HelpDisplay_H
/*
-----------------------------------------------------------------------------
 Class: HelpDisplay

 Desc: A BitmapText that cycles through messages.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ActorFrame.h"
#include "BitmapText.h"


class HelpDisplay : public ActorFrame
{
public:
	HelpDisplay();
	void Load();
	void SetTips( const CStringArray &arrayTips ) { SetTips( arrayTips, arrayTips ); }
	void SetTips( const CStringArray &arrayTips, const CStringArray &arrayTipsAlt );

	virtual void Update( float fDeltaTime );

protected:
	BitmapText m_textTip;

	CStringArray m_arrayTips, m_arrayTipsAlt;
	int m_iCurTipIndex;
	
	float m_fSecsUntilSwitch;
};

#endif
