/*
-----------------------------------------------------------------------------
 File: ScreenNetworkOptions

 Desc: Select a song.

 Copyright (c) 2004 by the person(s) listed below.  All rights reserved.
	Charles Lohr
-----------------------------------------------------------------------------
*/

#include "ScreenOptions.h"

class ScreenNetworkOptions : public ScreenOptions
{
public:
	ScreenNetworkOptions( CString sName );

	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual void MenuStart( PlayerNumber pn, const InputEventType type );

private:
	void ImportOptions();
	void ExportOptions();

	void GoToNextState();
	void GoToPrevState();
};

