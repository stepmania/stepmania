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


AnnouncerManager*	ANNOUNCER = NULL;	// global object accessable from anywhere in the program


const CString ANNOUNCER_BASE_DIR  = "Announcers\\";



AnnouncerManager::AnnouncerManager()
{
	LOG->WriteLine("AnnouncerManager::AnnouncerManager()");

	CStringArray arrayAnnouncerNames;
	GetAnnouncerNames( arrayAnnouncerNames );
	for( int i=0; i<arrayAnnouncerNames.GetSize(); i++ )
		AssertAnnouncerIsComplete( arrayAnnouncerNames[i] );
}

void AnnouncerManager::GetAnnouncerNames( CStringArray& AddTo )
{
	GetDirListing( ANNOUNCER_BASE_DIR+"*", AddTo, true );
	
	// strip out the folder called "CVS"
	for( int i=AddTo.GetSize()-1; i>=0; i-- )
	{
		if( 0 == stricmp( AddTo[i], "cvs" ) )
			AddTo.RemoveAt(i);
	}
}

bool AnnouncerManager::DoesAnnouncerExist( CString sAnnouncerName )
{
	if( sAnnouncerName == "" )
		return true;

	CStringArray asAnnouncerNames;
	GetAnnouncerNames( asAnnouncerNames );
	for( int i=0; i<asAnnouncerNames.GetSize(); i++ )
		if( 0==stricmp(sAnnouncerName, asAnnouncerNames[i]) )
			return true;
	return false;
}

void AnnouncerManager::AssertAnnouncerIsComplete( CString sAnnouncerName )
{
	for( int i=0; i<NUM_ANNOUNCER_ELEMENTS; i++ )
	{
		CString sPath = GetPathTo( (AnnouncerElement)i, sAnnouncerName );
//		if( !DoesFileExist(sPath) )
//			throw RageException( "The Announcer element '%s' is missing.", sPath );
	}
}

CString AnnouncerManager::GetAnnouncerDirFromName( CString sAnnouncerName )
{
	return ANNOUNCER_BASE_DIR + sAnnouncerName + "\\";
}

void AnnouncerManager::SwitchAnnouncer( CString sNewAnnouncerName )
{
	if( sNewAnnouncerName == "" )
		m_sCurAnnouncerName = "";
	else if( !DoesAnnouncerExist(sNewAnnouncerName) )
		m_sCurAnnouncerName = "";
	else
		m_sCurAnnouncerName = sNewAnnouncerName;
}

CString AnnouncerManager::GetPathTo( AnnouncerElement ae )
{
	return GetPathTo( ae, m_sCurAnnouncerName );
}

CString AnnouncerManager::GetPathTo( AnnouncerElement ae, CString sAnnouncerName ) 
{
	if( sAnnouncerName == "" )
		return "";	// hopefully there are no sound files here

	CString sAssetDir;

	switch( ae )
	{
		case ANNOUNCER_CAUTION:						 	sAssetDir = "caution";							break;
		case ANNOUNCER_GAMEPLAY_100_COMBO:				sAssetDir = "gameplay 100 combo";				break;
		case ANNOUNCER_GAMEPLAY_1000_COMBO:				sAssetDir = "gameplay 1000 combo";				break;
		case ANNOUNCER_GAMEPLAY_200_COMBO:				sAssetDir = "gameplay 200 combo";				break;
		case ANNOUNCER_GAMEPLAY_300_COMBO:				sAssetDir = "gameplay 300 combo";				break;
		case ANNOUNCER_GAMEPLAY_400_COMBO:				sAssetDir = "gameplay 400 combo";				break;
		case ANNOUNCER_GAMEPLAY_500_COMBO:				sAssetDir = "gameplay 500 combo";				break;
		case ANNOUNCER_GAMEPLAY_600_COMBO:				sAssetDir = "gameplay 600 combo";				break;
		case ANNOUNCER_GAMEPLAY_700_COMBO:				sAssetDir = "gameplay 700 combo";				break;
		case ANNOUNCER_GAMEPLAY_800_COMBO:				sAssetDir = "gameplay 800 combo";				break;
		case ANNOUNCER_GAMEPLAY_900_COMBO:				sAssetDir = "gameplay 900 combo";				break;
		case ANNOUNCER_GAMEPLAY_CLEARED:				sAssetDir = "gameplay cleared";					break;
		case ANNOUNCER_GAMEPLAY_COMBO_STOPPED:			sAssetDir = "gameplay combo stopped";			break;
		case ANNOUNCER_GAMEPLAY_COMMENT_DANGER:			sAssetDir = "gameplay comment danger";			break;
		case ANNOUNCER_GAMEPLAY_COMMENT_GOOD:			sAssetDir = "gameplay comment good";			break;	
		case ANNOUNCER_GAMEPLAY_COMMENT_HOT:			sAssetDir = "gameplay comment hot";				break;
		case ANNOUNCER_GAMEPLAY_FAILED:					sAssetDir = "gameplay failed";					break;
		case ANNOUNCER_GAMEPLAY_HERE_WE_GO_EXTRA:		sAssetDir = "gameplay here we go extra";		break;
		case ANNOUNCER_GAMEPLAY_HERE_WE_GO_FINAL:		sAssetDir = "gameplay here we go final";		break;
		case ANNOUNCER_GAMEPLAY_HERE_WE_GO_NORMAL:		sAssetDir = "gameplay here we go normal";		break;
		case ANNOUNCER_GAMEPLAY_READY:					sAssetDir = "gameplay ready";					break;
		case ANNOUNCER_EVALUATION_FINAL_A:				sAssetDir = "evaluation final a";				break;
		case ANNOUNCER_EVALUATION_FINAL_AA:				sAssetDir = "evaluation final aa";				break;
		case ANNOUNCER_EVALUATION_FINAL_AAA:			sAssetDir = "evaluation final aaa";				break;
		case ANNOUNCER_EVALUATION_FINAL_B:				sAssetDir = "evaluation final b";				break;
		case ANNOUNCER_EVALUATION_FINAL_C:				sAssetDir = "evaluation final c";				break;
		case ANNOUNCER_EVALUATION_FINAL_D:				sAssetDir = "evaluation final d";				break;
		case ANNOUNCER_EVALUATION_FINAL_E:				sAssetDir = "evaluation final e";				break;
		case ANNOUNCER_GAME_OVER:						sAssetDir = "game over";						break;
		case ANNOUNCER_MENU_HURRY_UP:					sAssetDir = "menu hurry up";					break;
		case ANNOUNCER_MUSIC_SCROLL:					sAssetDir = "music scroll";						break;
		case ANNOUNCER_EVALUATION_A:					sAssetDir = "evaluation a";						break;
		case ANNOUNCER_EVALUATION_AA:					sAssetDir = "evaluation aa";					break;
		case ANNOUNCER_EVALUATION_AAA:					sAssetDir = "evaluation aaa";					break;
		case ANNOUNCER_EVALUATION_B:					sAssetDir = "evaluation b";						break;
		case ANNOUNCER_EVALUATION_C:					sAssetDir = "evaluation c";						break;
		case ANNOUNCER_EVALUATION_D:					sAssetDir = "evaluation d";						break;
		case ANNOUNCER_EVALUATION_E:					sAssetDir = "evaluation e";						break;
		case ANNOUNCER_SELECT_COURSE_INTRO:				sAssetDir = "select course intro";				break;
		case ANNOUNCER_SELECT_DIFFICULTY_COMMENT_EASY:	sAssetDir = "select difficulty comment easy";	break;
		case ANNOUNCER_SELECT_DIFFICULTY_COMMENT_HARD:	sAssetDir = "select difficulty comment hard";	break;
		case ANNOUNCER_SELECT_DIFFICULTY_COMMENT_MEDIUM:sAssetDir = "select difficulty comment medium";	break;
		case ANNOUNCER_SELECT_DIFFICULTY_COMMENT_ONI:	sAssetDir = "select difficulty comment oni";	break;
		case ANNOUNCER_SELECT_DIFFICULTY_CHALLENGE:		sAssetDir = "select difficulty challenge";		break;
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
		case ANNOUNCER_STAGE_1:							sAssetDir = "stage 1";							break;
		case ANNOUNCER_STAGE_2:							sAssetDir = "stage 2";							break;
		case ANNOUNCER_STAGE_3:							sAssetDir = "stage 3";							break;
		case ANNOUNCER_STAGE_4:							sAssetDir = "stage 4";							break;
		case ANNOUNCER_STAGE_5:							sAssetDir = "stage 5";							break;
		case ANNOUNCER_STAGE_FINAL:						sAssetDir = "stage final";						break;
		case ANNOUNCER_STAGE_EXTRA:						sAssetDir = "stage extra";						break;
		case ANNOUNCER_STAGE_ANOTHER_EXTRA:				sAssetDir = "stage another extra";				break;
		case ANNOUNCER_STAGE_CHALLENGE:					sAssetDir = "stage challenge";					break;
		case ANNOUNCER_TITLE_MENU_ATTRACT:				sAssetDir = "title menu attract";				break;
		case ANNOUNCER_TITLE_MENU_GAME_NAME:			sAssetDir = "title menu game name";				break;

		default:	ASSERT(0);  // Unhandled Announcer element
	}

	return GetAnnouncerDirFromName( sAnnouncerName ) + sAssetDir;
}
