#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: AnnouncerManager

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "AnnouncerManager.h"
#include "RageLog.h"
#include "ErrorCatcher/ErrorCatcher.h"

AnnouncerManager*	ANNOUNCER = NULL;	// global object accessable from anywhere in the program


const CString DEFAULT_ANNOUNCER_NAME = "default";
const CString ANNOUNER_BASE_DIR  = "Announcers\\";



AnnouncerManager::AnnouncerManager()
{
	CStringArray arrayAnnouncerNames;
	GetAnnouncerNames( arrayAnnouncerNames );
	for( int i=0; i<arrayAnnouncerNames.GetSize(); i++ )
		AssertAnnouncerIsComplete( arrayAnnouncerNames[i] );

	SwitchAnnouncer( DEFAULT_ANNOUNCER_NAME );
}

void AnnouncerManager::GetAnnouncerNames( CStringArray& AddTo )
{
	GetDirListing( DEFAULT_ANNOUNCER_NAME+"\\*", AddTo, true );
	
	// strip out the folder called "CVS"
	for( int i=AddTo.GetSize()-1; i>=0; i-- )
	{
		if( 0 == stricmp( AddTo[i], "cvs" ) )
			AddTo.RemoveAt(i);
	}
}

void AnnouncerManager::SwitchAnnouncer( CString sAnnouncerName )
{
	m_sCurAnnouncerName = sAnnouncerName;
	CString sAnnouncerDir = GetAnnouncerDirFromName( m_sCurAnnouncerName );
	if( !DoesFileExist( sAnnouncerDir ) )
		FatalError( "Error loading the announcer in diretory '%s'.", m_sCurAnnouncerName );
}

void AnnouncerManager::AssertAnnouncerIsComplete( CString sAnnouncerName )
{
	for( int i=0; i<NUM_ANNOUNCER_ELEMENTS; i++ )
	{
		CString sPath = GetPathTo( (AnnouncerElement)i, sAnnouncerName );
		if( !DoesFileExist(sPath) )
			FatalError( "The Announcer element '%s' is missing.", sPath );
	}
}

CString AnnouncerManager::GetAnnouncerDirFromName( CString sAnnouncerName )
{
	return ANNOUNER_BASE_DIR + sAnnouncerName + "\\";
}

CString AnnouncerManager::GetPathTo( AnnouncerElement ae )
{
	return GetPathTo( ae, m_sCurAnnouncerName );
}

CString AnnouncerManager::GetPathTo( AnnouncerElement ae, CString sAnnouncerName ) 
{
	CString sAssetDir;

	switch( ae )
	{
		case ANNOUNCER_CAUTION:						 	sAssetDir = "caution";					break;
		case ANNOUNCER_GAMEPLAY_100_COMBO:				sAssetDir = "gameplay 100 combo";		break;
		case ANNOUNCER_GAMEPLAY_1000_COMBO:				sAssetDir = "gameplay 1000 combo";		break;
		case ANNOUNCER_GAMEPLAY_200_COMBO:				sAssetDir = "gameplay 200 combo";		break;
		case ANNOUNCER_GAMEPLAY_300_COMBO:				sAssetDir = "gameplay 300 combo";		break;
		case ANNOUNCER_GAMEPLAY_400_COMBO:				sAssetDir = "gameplay 400 combo";		break;
		case ANNOUNCER_GAMEPLAY_500_COMBO:				sAssetDir = "gameplay 500 combo";		break;
		case ANNOUNCER_GAMEPLAY_600_COMBO:				sAssetDir = "gameplay 600 combo";		break;
		case ANNOUNCER_GAMEPLAY_700_COMBO:				sAssetDir = "gameplay 700 combo";		break;
		case ANNOUNCER_GAMEPLAY_800_COMBO:				sAssetDir = "gameplay 800 combo";		break;
		case ANNOUNCER_GAMEPLAY_900_COMBO:				sAssetDir = "gameplay 900 combo";		break;
		case ANNOUNCER_GAMEPLAY_CLEARED:				sAssetDir = "gameplay cleared";			break;
		case ANNOUNCER_GAMEPLAY_COMBO_STOPPED:			sAssetDir = "gameplay combo stopped";	break;
		case ANNOUNCER_GAMEPLAY_COMMENT_BAD:			sAssetDir = "gameplay comment bad";		break;
		case ANNOUNCER_GAMEPLAY_COMMENT_GOOD:			sAssetDir = "gameplay comment good";	break;	
		case ANNOUNCER_GAMEPLAY_COMMENT_OK:				sAssetDir = "gameplay comment ok";		break;
		case ANNOUNCER_GAMEPLAY_COMMENT_VERY_BAD:		sAssetDir = "gameplay comment very bad";		break;
		case ANNOUNCER_GAMEPLAY_COMMENT_VERY_GOOD_BOTH:	sAssetDir = "gameplay comment very good both";	break;
		case ANNOUNCER_GAMEPLAY_COMMENT_VERY_GOOD_P1:	sAssetDir = "gameplay comment very good p1";	break;
		case ANNOUNCER_GAMEPLAY_COMMENT_VERY_GOOD_P2:	sAssetDir = "gameplay comment very good p2";	break;
		case ANNOUNCER_GAMEPLAY_FAILED:					sAssetDir = "gameplay failed";					break;
		case ANNOUNCER_GAMEPLAY_HERE_WE_GO_EXTRA:		sAssetDir = "gameplay here we go extra";		break;
		case ANNOUNCER_GAMEPLAY_HERE_WE_GO_FINAL:		sAssetDir = "gameplay here we go final";		break;
		case ANNOUNCER_GAMEPLAY_HERE_WE_GO_NORMAL:		sAssetDir = "gameplay here we go normal";		break;
		case ANNOUNCER_GAMEPLAY_READY:					sAssetDir = "gameplay ready";			break;
		case ANNOUNCER_FINAL_RESULT_A:					sAssetDir = "final result a";			break;
		case ANNOUNCER_FINAL_RESULT_AA:					sAssetDir = "final result aa";			break;
		case ANNOUNCER_FINAL_RESULT_AAA:				sAssetDir = "final result aaa";			break;
		case ANNOUNCER_FINAL_RESULT_B:					sAssetDir = "final result b";			break;
		case ANNOUNCER_FINAL_RESULT_C:					sAssetDir = "final result c";			break;
		case ANNOUNCER_FINAL_RESULT_D:					sAssetDir = "final result d";			break;
		case ANNOUNCER_FINAL_RESULT_E:					sAssetDir = "final result e";			break;
		case ANNOUNCER_GAME_OVER:						sAssetDir = "game over";				break;
		case ANNOUNCER_MUSIC_SCROLL:					sAssetDir = "music scroll";				break;
		case ANNOUNCER_RESULT_A:						sAssetDir = "result a";					break;
		case ANNOUNCER_RESULT_AA:						sAssetDir = "result aa";				break;
		case ANNOUNCER_RESULT_AAA:						sAssetDir = "result aaa";				break;
		case ANNOUNCER_RESULT_B:						sAssetDir = "result b";					break;
		case ANNOUNCER_RESULT_C:						sAssetDir = "result c";					break;
		case ANNOUNCER_RESULT_D:						sAssetDir = "result d";					break;
		case ANNOUNCER_RESULT_E:						sAssetDir = "result e";					break;
		case ANNOUNCER_SELECT_DIFFICULTY_COMMENT_EASY:	sAssetDir = "select difficulty comment easy";	break;
		case ANNOUNCER_SELECT_DIFFICULTY_COMMENT_HARD:	sAssetDir = "select difficulty comment hard";	break;
		case ANNOUNCER_SELECT_DIFFICULTY_COMMENT_MEDIUM:sAssetDir = "select difficulty comment medium";	break;
		case ANNOUNCER_SELECT_DIFFICULTY_INTRO:			sAssetDir = "select difficulty intro";			break;
		case ANNOUNCER_SELECT_GROUP_COMMENT_ALL_MUSIC:	sAssetDir = "select group comment all music";	break;
		case ANNOUNCER_SELECT_GROUP_COMMENT_GENERAL:	sAssetDir = "select group comment general";		break;
		case ANNOUNCER_SELECT_GROUP_INTRO:				sAssetDir = "select group intro";				break;
		case ANNOUNCER_SELECT_MUSIC_COMMENT_GENERAL:	sAssetDir = "select music comment general";		break;
		case ANNOUNCER_SELECT_MUSIC_COMMENT_HARD:		sAssetDir = "select music comment hard";		break;
		case ANNOUNCER_SELECT_MUSIC_COMMENT_NEW:		sAssetDir = "select music comment new";			break;
		case ANNOUNCER_SELECT_MUSIC_INTRO:				sAssetDir = "select music intro";				break;
		case ANNOUNCER_SELECT_STYLE_COMMENT_COUPLE:		sAssetDir = "select style comment couple";		break;
		case ANNOUNCER_SELECT_STYLE_COMMENT_DOUBLE:		sAssetDir = "select style comment double";		break;
		case ANNOUNCER_SELECT_STYLE_COMMENT_SINGLE:		sAssetDir = "select style comment single";		break;
		case ANNOUNCER_SELECT_STYLE_COMMENT_SOLO:		sAssetDir = "select style comment solo";		break;
		case ANNOUNCER_SELECT_STYLE_COMMENT_VERSUS:		sAssetDir = "select style comment versus";		break;
		case ANNOUNCER_SELECT_STYLE_INTRO:				sAssetDir = "select style intro";				break;
		case ANNOUNCER_STAGE_1:							sAssetDir = "stage 1";						break;
		case ANNOUNCER_STAGE_2:							sAssetDir = "stage 2";						break;
		case ANNOUNCER_STAGE_3:							sAssetDir = "stage 3";						break;
		case ANNOUNCER_STAGE_4:							sAssetDir = "stage 4";						break;
		case ANNOUNCER_STAGE_5:							sAssetDir = "stage 5";						break;
		case ANNOUNCER_STAGE_EXTRA:						sAssetDir = "stage extra";					break;
		case ANNOUNCER_STAGE_FINAL:						sAssetDir = "stage final";					break;
		case ANNOUNCER_TITLE_MENU_ATTRACT:				sAssetDir = "title menu attract";			break;
		case ANNOUNCER_TITLE_MENU_GAME_NAME:			sAssetDir = "title menu game name";			break;

		default:	ASSERT(0);  // Unhandled Announcer element
	}

	return GetAnnouncerDirFromName( sAnnouncerName ) + sAssetDir;
}
