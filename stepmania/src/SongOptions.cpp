#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: SongOptions

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "SongOptions.h"
#include "RageUtil.h"

void SongOptions::Init() 
{
	m_LifeType = LIFE_BAR;
	m_DrainType = DRAIN_NORMAL;
	m_iBatteryLives = 4;
	m_FailType = FAIL_ARCADE;
	m_bAssistTick = false;
	m_fMusicRate = 1.0f;
	m_bAutoSync = false;
}

CString SongOptions::GetString()
{
	CString sReturn;

	switch( m_LifeType )
	{
	case LIFE_BAR:		
		switch( m_DrainType )
		{
		case DRAIN_NORMAL:										break;
		case DRAIN_NO_RECOVER:		sReturn	+= "NoRecover, ";	break;
		case DRAIN_SUDDEN_DEATH:	sReturn	+= "SuddenDeath, ";	break;
		}
		break;
	case LIFE_BATTERY:
		sReturn	+= ssprintf( "%dLives, ", m_iBatteryLives );
		break;
	}


	switch( m_FailType )
	{
	case FAIL_ARCADE:											break;
	case FAIL_END_OF_SONG:		sReturn	+= "FailEndOfSong, ";	break;
	case FAIL_OFF:				sReturn	+= "FailOff, ";			break;
	}

	if( m_fMusicRate != 1 )
	{
		CString s = ssprintf( "%2.2f", m_fMusicRate );
		if( s[s.GetLength()-1] == '0' )
			s.erase(s.GetLength()-1);
		sReturn += s + "xMusic, ";
	}

	if( m_bAutoSync )
		sReturn += "AutoSync, ";

	if( sReturn.GetLength() > 2 )
		sReturn.erase( sReturn.GetLength()-2 );	// delete the trailing ", "
	return sReturn;
}

/* Options are added to the current settings; call Init() beforehand if
 * you don't want this. */
void SongOptions::FromString( CString sOptions )
{
//	Init();
	sOptions.MakeLower();
	CStringArray asBits;
	split( sOptions, ",", asBits, true );

	for( unsigned i=0; i<asBits.size(); i++ )
	{
		CString& sBit = asBits[i];
		TrimLeft(sBit);
		TrimRight(sBit);
		
		Regex mult("^([0-9]+(\\.[0-9]+)?)xmusic$");
		vector<CString> matches;
		if( mult.Compare(sBit, matches) )
		{
			int ret = sscanf( matches[0], "%f", &m_fMusicRate );
			ASSERT( ret == 1 );
		}

		if(	     sBit == "norecover" )		m_DrainType = DRAIN_NO_RECOVER;
		else if( sBit == "suddendeath" )	m_DrainType = DRAIN_SUDDEN_DEATH;
		else if( sBit == "power-drop" )		m_DrainType = DRAIN_NO_RECOVER;
		else if( sBit == "death" )			m_DrainType = DRAIN_SUDDEN_DEATH;
		else if( sBit == "failendofsong" )	m_FailType = FAIL_END_OF_SONG;
		else if( sBit == "failoff" )		m_FailType = FAIL_OFF;
	}
}
