#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: StyleDef.cpp

 Desc: A data structure that holds the definition of a GameMode.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "StyleDef.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "ErrorCatcher/ErrorCatcher.h"
#include "GameDef.h"
#include "IniFile.h"


StyleDef::StyleDef( GameDef* pGameDef, CString sStyleFilePath )
{
	LOG->WriteLine( "StyleDef::StyleDef( '%s )", sStyleFilePath );

	// extract the game name
	CString sThrowAway;
	splitrelpath( sStyleFilePath, sThrowAway, m_sName, sThrowAway );


	//
	// Parse the .style definition file
	//
	IniFile ini;
	ini.SetPath( sStyleFilePath );
	if( !ini.ReadFile() )
		FatalError( "Error reading style definition file '%s'.", sStyleFilePath );


	if( ini.GetValue( "Style", "Name" )  !=  m_sName )
		FatalError( "Style name in '%s' doesn't match the file name.", sStyleFilePath );

	m_sDescription = ini.GetValue( "Style", "Description" );
	if( m_sDescription == "" )
		FatalError( "Invalid value for Description in '%s'.", sStyleFilePath );

	m_sReadsTag = ini.GetValue( "Style", "ReadsTag" );
	if( m_sReadsTag == "" )
		FatalError( "Invalid value for ReadsTag in '%s'.", sStyleFilePath );

	m_iNumPlayers = ini.GetValueI( "Style", "NumPlayers" );
	if( m_iNumPlayers < 1  ||  m_iNumPlayers > 2 )
		FatalError( "Invalid value for NumPlayers in '%s'.", sStyleFilePath );

	m_iColsPerPlayer = ini.GetValueI( "Style", "ColsPerPlayer" );
	if( m_iColsPerPlayer < 1  ||  m_iColsPerPlayer > MAX_COLS_PER_PLAYER )
		FatalError( "Invalid value for ColsPerPlayer in '%s'.", sStyleFilePath );


	for( int p=0; p<m_iNumPlayers; p++ )
	{
		for( int c=0; c<m_iColsPerPlayer; c++ )
		{
			ColumnInfo &colInfo = m_ColumnInfo[p][c];	// fill this in below

			CString sValueName = ssprintf( "P%01dCol%02d", p+1, c+1 );

			CString sColumnInfo = ini.GetValue( "Style", sValueName );	// should look like "TRACK08,INSTRUMENT02,right,576"
			if( sColumnInfo == "" )
				FatalError( "Value '%s' missing in file '%s'.", sValueName, sStyleFilePath );

			// replace the constants with their corresponding integer literals
			sColumnInfo.Replace( "TRACK01", "0" );
			sColumnInfo.Replace( "TRACK02", "1" );
			sColumnInfo.Replace( "TRACK03", "2" );
			sColumnInfo.Replace( "TRACK04", "3" );
			sColumnInfo.Replace( "TRACK05", "4" );
			sColumnInfo.Replace( "TRACK06", "5" );
			sColumnInfo.Replace( "TRACK07", "6" );
			sColumnInfo.Replace( "TRACK08", "7" );
			sColumnInfo.Replace( "TRACK09", "8" );
			sColumnInfo.Replace( "TRACK00", "9" );
			sColumnInfo.Replace( "TRACK11", "10" );
			sColumnInfo.Replace( "TRACK12", "11" );
			sColumnInfo.Replace( "TRACK13", "12" );
			sColumnInfo.Replace( "TRACK14", "13" );
			sColumnInfo.Replace( "TRACK15", "14" );
			sColumnInfo.Replace( "TRACK16", "15" );
			sColumnInfo.Replace( "INSTRUMENT01", "0" );
			sColumnInfo.Replace( "INSTRUMENT02", "1" );

			// replace button names with their corresponding index
			for( int b=0; b<pGameDef->m_iButtonsPerInstrument; b++ )
				if( 0 != sColumnInfo.Replace( pGameDef->m_sButtonNames[b], ssprintf("%d",b) ) )	// replace was successful
					break;
			if( b == pGameDef->m_iButtonsPerInstrument )	// we didn't replace anything
				FatalError( "Invalid button name in ColumnInfo in '%s': '%s'.", sStyleFilePath, sColumnInfo );


			CStringArray arrayColumnInfo;
			split( sColumnInfo, ",", arrayColumnInfo );
			if( arrayColumnInfo.GetSize() != 4 )
				FatalError( "Invalid ColumnInfo string in '%s'.", sStyleFilePath );


			colInfo.track	= atoi( arrayColumnInfo[0] );
			colInfo.number	= (InstrumentNumber)atoi( arrayColumnInfo[1] );
			colInfo.button	= (InstrumentButton)atoi( arrayColumnInfo[2] );
			colInfo.iX		= atoi( arrayColumnInfo[3] );
		}
	}
	m_iColsPerPlayer = ini.GetValueI( "Style", "ColsPerPlayer" );
	if( m_iColsPerPlayer < 1  ||  m_iColsPerPlayer > MAX_COLS_PER_PLAYER )
		FatalError( "Invalid value for ColsPerPlayer in '%s'.", sStyleFilePath );



	CString sColDrawOrder = ini.GetValue( "Style", "ColDrawOrder" );	// should look like "COL01,COL02,COL03,COL04"
	
	// replace the constants with their corresponding integer literals
	sColDrawOrder.Replace( "COL01", "0" );
	sColDrawOrder.Replace( "COL02", "1" );
	sColDrawOrder.Replace( "COL03", "2" );
	sColDrawOrder.Replace( "COL04", "3" );
	sColDrawOrder.Replace( "COL05", "4" );
	sColDrawOrder.Replace( "COL06", "5" );
	sColDrawOrder.Replace( "COL07", "6" );
	sColDrawOrder.Replace( "COL08", "7" );
	sColDrawOrder.Replace( "COL09", "8" );
	sColDrawOrder.Replace( "COL00", "9" );
	sColDrawOrder.Replace( "COL11", "10" );
	sColDrawOrder.Replace( "COL12", "11" );
	sColDrawOrder.Replace( "COL13", "12" );
	sColDrawOrder.Replace( "COL14", "13" );
	sColDrawOrder.Replace( "COL15", "14" );
	sColDrawOrder.Replace( "COL16", "15" );

	CStringArray arrayColDrawOrder;
	split( sColDrawOrder, ",", arrayColDrawOrder );
	if( arrayColDrawOrder.GetSize() != m_iColsPerPlayer )
		FatalError( "Invalid number of columns in ColDrawOrder in '%s'.", sStyleFilePath );

	for( int c=0; c<m_iColsPerPlayer; c++ )
		m_iColumnDrawOrder[c] = atoi( arrayColDrawOrder[c] );
}

void StyleDef::GetTransformedNoteDataForStyle( PlayerNumber p, NoteData* pOriginal, NoteData &newNoteData )
{
	TrackNumber iNewToOriginalTrack[MAX_COLS_PER_PLAYER];
	for( int col=0; col<m_iColsPerPlayer; col++ )
	{
		ColumnInfo colInfo = m_ColumnInfo[p][col];
		TrackNumber originalTrack = colInfo.track;
		
		iNewToOriginalTrack[col] = originalTrack;
	}
	
	newNoteData.LoadTransformed( pOriginal, m_iColsPerPlayer, iNewToOriginalTrack );
}