#include "stdafx.h"
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

	switch( m_AutoAdjust )
	{
	case ADJUST_OFF:											break;
	case ADJUST_ON:				sReturn += "AutoAdjust, ";		break;
	}

	if( sReturn.GetLength() > 2 )
		sReturn.erase( sReturn.GetLength()-2 );	// delete the trailing ", "
	return sReturn;
}

void SongOptions::FromString( CString sOptions )
{
	Init();
	sOptions.MakeLower();
	CStringArray asBits;
	split( sOptions, ",", asBits, true );

	for( unsigned i=0; i<asBits.size(); i++ )
	{
		CString& sBit = asBits[i];
		TrimLeft(sBit);
		TrimRight(sBit);
		
		if(	     sBit == "norecover" )		m_DrainType = DRAIN_NO_RECOVER;
		else if( sBit == "suddendeath" )	m_DrainType = DRAIN_SUDDEN_DEATH;
		else if( sBit == "power-drop" )		m_DrainType = DRAIN_NO_RECOVER;
		else if( sBit == "death" )			m_DrainType = DRAIN_SUDDEN_DEATH;
		else if( sBit == "0.7xmusic" )		m_fMusicRate = 0.7f;
		else if( sBit == "0.8xmusic" )		m_fMusicRate = 0.8f;
		else if( sBit == "0.9xmusic" )		m_fMusicRate = 0.9f;
		else if( sBit == "1.0xmusic" )		m_fMusicRate = 1.0f;
		else if( sBit == "1.1xmusic" )		m_fMusicRate = 1.1f;
		else if( sBit == "1.2xmusic" )		m_fMusicRate = 1.2f;
		else if( sBit == "1.3xmusic" )		m_fMusicRate = 1.3f;
		else if( sBit == "1.4xmusic" )		m_fMusicRate = 1.4f;
		else if( sBit == "1.5xmusic" )		m_fMusicRate = 1.5f;
	}
}
