#ifndef SCREENPLAYEROPTIONS_H
#define SCREENPLAYEROPTIONS_H
/*
-----------------------------------------------------------------------------
 Class: ScreenPlayerOptions

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenOptionsMaster.h"

class ScreenPlayerOptions : public ScreenOptionsMaster
{
public:
	ScreenPlayerOptions( CString sName );

	virtual void Update( float fDelta );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

private:
	void GoToNextState();
	void GoToPrevState();

	virtual void ExportOptions();

	void UpdateDisqualified();

	bool            m_bAcceptedChoices;
	bool            m_bGoToOptions;
	bool            m_bAskOptionsMessage;
	Sprite          m_sprOptionsMessage;
};



#endif
