#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ProfileManager

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ProfileManager.h"
#include "RageUtil.h"

CString Profile::GetDisplayName()
{
	if( !m_sName.empty() )
		return m_sName;
	else if( !m_sLastUsedHighScoreName.empty() )
		return m_sLastUsedHighScoreName;
	else
		return "NO NAME";
}

CString Profile::GetDisplayCaloriesBurned()
{
	if( m_fWeightPounds == 0 )	// weight not entered
		return "N/A";
	else 
		return ssprintf("%iCal",m_fCaloriesBurned);
}

int Profile::GetTotalNumSongsPlayed()
{
	int iTotal = 0;
	for( int i=0; i<NUM_PLAY_MODES; i++ )
		iTotal += m_iNumSongsPlayedByPlayMode[i];
	return iTotal;
}


