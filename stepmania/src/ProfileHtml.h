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

enum HtmlType { HTML_TYPE_MACHINE, HTML_TYPE_PLAYER };

void SaveStatsWebPage( 
	CString sDir, 
	const Profile *pProfile, 	
	const Profile *pMachineProfile, 	
	HtmlType htmlType
	);


#endif
