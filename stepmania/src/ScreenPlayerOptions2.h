#ifndef ScreenPlayerOptions2_H
#define ScreenPlayerOptions2_H
/*
-----------------------------------------------------------------------------
 Class: ScreenPlayerOptions2

 Desc: Select a song.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenOptions.h"
#include "PrefsManager.h"


class ScreenPlayerOptions2 : public ScreenOptions
{
public:
	ScreenPlayerOptions2();

	virtual void Update( float fDelta );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

private:
	void ImportOptions();
	void ExportOptions();

	void GoToNextState();
	void GoToPrevState();

	bool            m_bAcceptedChoices;
	bool            m_bGoToOptions;
	Sprite          m_sprOptionsMessage;
};



#endif
