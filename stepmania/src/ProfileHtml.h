#ifndef ProfileHtml_H
#define ProfileHtml_H
/*
-----------------------------------------------------------------------------
 Class: ProfileHtml

 Desc: Helpers for generating an HTML web page based on Profile data.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Profile.h"

void SavePlayerHtmlToDir( CString sDir, const Profile* pProfilePlayer, const Profile* pProfileMachine );
void SaveMachineHtmlToDir( CString sDir, const Profile* pProfileMachine  );


#endif
