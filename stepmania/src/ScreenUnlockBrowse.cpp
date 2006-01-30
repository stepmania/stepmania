#include "global.h"
#include "ScreenUnlockBrowse.h"
#include "UnlockManager.h"


REGISTER_SCREEN_CLASS( ScreenUnlockBrowse );

void ScreenUnlockBrowse::Init()
{
	// fill m_aGameCommands before calling Init()
	FOREACH_CONST( UnlockEntry, UNLOCKMAN->m_UnlockEntries, ue )
	{
		GameCommand gc;
		if( !ue->IsLocked() )
			gc.m_bInvalid = false;
		gc.m_iIndex = ue - UNLOCKMAN->m_UnlockEntries.begin();
		gc.m_iUnlockEntryID = ue->m_iEntryID;
		gc.m_sName = ssprintf("%d",ue->m_iEntryID);
		
		m_aGameCommands.push_back( gc );
	}

	ScreenSelectMaster::Init();
}

void ScreenUnlockBrowse::MenuStart( PlayerNumber pn )
{
	this->PostScreenMessage( SM_BeginFadingOut, 0 );
}


/*
 * (c) 2006 Chris Danford
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
