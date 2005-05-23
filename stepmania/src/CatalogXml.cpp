#include "global.h"
#include "CatalogXml.h"
#include "SongManager.h"
#include "RageLog.h"
#include "song.h"
#include "Steps.h"
#include "XmlFile.h"
#include "Course.h"
#include "SongUtil.h"
#include "StepsUtil.h"
#include "CourseUtil.h"
#include "TrailUtil.h"
#include "GameState.h"
#include <set>
#include "Foreach.h"
#include "GameManager.h"
#include "StyleUtil.h"
#include "ThemeManager.h"
#include "PrefsManager.h"
#include "Style.h"
#include "CommonMetrics.h"
#include "UnlockManager.h"
#include "arch/LoadingWindow/LoadingWindow.h"

#define SHOW_PLAY_MODE(pm)				THEME->GetMetricB("CatalogXml",ssprintf("ShowPlayMode%s",PlayModeToString(pm).c_str()))
#define SHOW_STYLE(ps)					THEME->GetMetricB("CatalogXml",ssprintf("ShowStyle%s",Capitalize((ps)->m_szName).c_str()))
#define INTERNET_RANKING_HOME_URL		THEME->GetMetric ("CatalogXml","InternetRankingHomeUrl")
#define INTERNET_RANKING_UPLOAD_URL		THEME->GetMetric ("CatalogXml","InternetRankingUploadUrl")
#define INTERNET_RANKING_VIEW_GUID_URL	THEME->GetMetric ("CatalogXml","InternetRankingViewGuidUrl")
#define PRODUCT_TITLE					THEME->GetMetric ("CatalogXml","ProductTitle")
#define FOOTER_TEXT						THEME->GetMetric ("CatalogXml","FooterText")
#define FOOTER_LINK						THEME->GetMetric ("CatalogXml","FooterLink")

void SaveCatalogXml( LoadingWindow *loading_window )
{
	ASSERT( SONGMAN );
	ASSERT( UNLOCKMAN );

	if( loading_window )
		loading_window->SetText( "Saving Catalog.xml ..." );

	CString fn = CATALOG_XML_FILE;

	LOG->Trace( "Writing %s ...", fn.c_str() );

	XNode xml;
	xml.m_sName = "Catalog";

	const vector<StepsType> &vStepsTypesToShow = STEPS_TYPES_TO_SHOW.GetValue();
	
	{
		XNode* pNode = xml.AppendChild( "Totals" );
		XNode* pNumSongsByGroup = pNode->AppendChild( "NumSongsByGroup" );
		XNode* pNumStepsByGroup = pNode->AppendChild( "NumStepsByGroup" );

		int iTotalSongs = 0;
		int iTotalSteps = 0;
		vector<CString> vsGroups;
		SONGMAN->GetGroupNames( vsGroups );
		FOREACH_CONST( CString, vsGroups, sGroup )
		{
			XNode* p1 = pNumSongsByGroup->AppendChild( "Group" );
			p1->AppendAttr( "Name", *sGroup );
			XNode* p2 = pNumStepsByGroup->AppendChild( "Group" );
			p2->AppendAttr( "Name", *sGroup );

			int iNumSongsInGroup = 0;
			vector<Song*> vpSongsInGroup;
			SONGMAN->GetSongs( vpSongsInGroup, *sGroup );
			FOREACH_CONST( StepsType, vStepsTypesToShow, st )
			{
				XNode* p3 = p2->AppendChild( "StepsType" );
				p3->AppendAttr( "StepsType", GAMEMAN->StepsTypeToString(*st) );

				int iNumStepsInGroupAndStepsType[NUM_DIFFICULTIES];
				ZERO( iNumStepsInGroupAndStepsType );
				FOREACH_CONST( Song*, vpSongsInGroup, i )
				{
					Song *pSong = *i;
					if( pSong->IsTutorial() )
						continue;	// skip
					if( UNLOCKMAN->SongIsLocked(pSong) )
						continue;	// skip
					iTotalSongs++;
					iNumSongsInGroup++;

					FOREACH_CONST( Difficulty, DIFFICULTIES_TO_SHOW.GetValue(), dc )
					{
						Steps* pSteps = pSong->GetStepsByDifficulty( *st, *dc, false );	// no autogen
						if( pSteps == NULL )
							continue;	// skip
						if( UNLOCKMAN->StepsIsLocked(pSong,pSteps) )
							continue;	// skip
						iTotalSteps++;
						iNumStepsInGroupAndStepsType[pSteps->GetDifficulty()]++;
					}
				}
				FOREACH_Difficulty( dc )
					p3->AppendChild( DifficultyToString(dc), iNumStepsInGroupAndStepsType[dc] );
			}

			p1->AppendAttr( "Num", iNumSongsInGroup );
		}

		int iTotalCourses = 0;
		vector<Course*> vpCourses;
		SONGMAN->GetAllCourses( vpCourses, false );
		for( unsigned i=0; i<vpCourses.size(); i++ )
		{
			Course* pCourse = vpCourses[i];
			// skip non-fixed courses.  We don't have any stable data for them other than 
			// the title.
			if( !pCourse->IsFixed() )
				continue;
			if( UNLOCKMAN->CourseIsLocked(pCourse) )
				continue;
			iTotalCourses++;
		}

		int iNumUnlockedSongs = 0;
		int iNumUnlockedSteps = 0;
		int iNumUnlockedCourses = 0;
		FOREACH_CONST( UnlockEntry, UNLOCKMAN->m_UnlockEntries, e )
		{
			if( e->IsLocked() )
				continue;
			switch( e->m_Type )
			{
			case UnlockEntry::TYPE_SONG:	iNumUnlockedSongs++;	break;
			case UnlockEntry::TYPE_STEPS:	iNumUnlockedSteps++;	break;
			case UnlockEntry::TYPE_COURSE:	iNumUnlockedCourses++;	break;
			}
		}

		pNode->AppendChild( "TotalSongs",			iTotalSongs );
		pNode->AppendChild( "TotalSteps",			iTotalSteps );
		pNode->AppendChild( "TotalCourses",			iTotalCourses );
		pNode->AppendChild( "NumUnlockedSongs",		iNumUnlockedSongs );
		pNode->AppendChild( "NumUnlockedSteps",		iNumUnlockedSteps );
		pNode->AppendChild( "NumUnlockedCourses",	iNumUnlockedCourses );
	}

	{
		XNode* pNode = xml.AppendChild( "Songs" );

		vector<Song*> vpSongs = SONGMAN->GetAllSongs();
		for( unsigned i=0; i<vpSongs.size(); i++ )
		{
			Song* pSong = vpSongs[i];
			
			if( pSong->IsTutorial() )
				continue;	// skip
			if( UNLOCKMAN->SongIsLocked(pSong) )
				continue;	// skip

			SongID songID;
			songID.FromSong( pSong );

			XNode* pSongNode = songID.CreateNode();

			pNode->AppendChild( pSongNode );

			pSongNode->AppendChild( "MainTitle", pSong->GetDisplayMainTitle() );
			pSongNode->AppendChild( "SubTitle", pSong->GetDisplaySubTitle() );

			FOREACH_CONST( StepsType, vStepsTypesToShow, st )
			{
				FOREACH_CONST( Difficulty, DIFFICULTIES_TO_SHOW.GetValue(), dc )
				{
					Steps* pSteps = pSong->GetStepsByDifficulty( *st, *dc, false );	// no autogen
					if( pSteps == NULL )
						continue;	// skip
					if( UNLOCKMAN->StepsIsLocked(pSong,pSteps) )
						continue;	// skip

					StepsID stepsID;
					stepsID.FromSteps( pSteps );

					XNode* pStepsIDNode = stepsID.CreateNode();
					pSongNode->AppendChild( pStepsIDNode );
					
					pStepsIDNode->AppendChild( "Meter", pSteps->GetMeter() );
					pStepsIDNode->AppendChild( pSteps->GetRadarValues().CreateNode() );
				}
			}
		}
	}


	{
		XNode* pNode = xml.AppendChild( "Courses" );

		vector<Course*> vpCourses;
		SONGMAN->GetAllCourses( vpCourses, false );
		for( unsigned i=0; i<vpCourses.size(); i++ )
		{
			Course* pCourse = vpCourses[i];

			// skip non-fixed courses.  We don't have any stable data for them other than 
			// the title.
			if( !pCourse->IsFixed() )
				continue;
			if( UNLOCKMAN->CourseIsLocked(pCourse) )
				continue;

			CourseID courseID;
			courseID.FromCourse( pCourse );

			XNode* pCourseNode = courseID.CreateNode();

			pNode->AppendChild( pCourseNode );

			pCourseNode->AppendChild( "MainTitle", pCourse->GetDisplayMainTitle() );
			pCourseNode->AppendChild( "SubTitle", pCourse->GetDisplaySubTitle() );
			pCourseNode->AppendChild( "HasMods", pCourse->HasMods() );

			const vector<CourseDifficulty> &vDiffs = DIFFICULTIES_TO_SHOW.GetValue();

			FOREACH_CONST( StepsType, vStepsTypesToShow, st )
			{
				FOREACH_CONST( CourseDifficulty, vDiffs, dc )
				{
					Trail *pTrail = pCourse->GetTrail( *st, *dc );
					if( pTrail == NULL )
						continue;
					if( !pTrail->m_vEntries.size() )
						continue;
					
					TrailID trailID;
					trailID.FromTrail( pTrail );

					XNode* pTrailIDNode = trailID.CreateNode();
					pCourseNode->AppendChild( pTrailIDNode );
					
					pTrailIDNode->AppendChild( "Meter", pTrail->GetMeter() );
					pTrailIDNode->AppendChild( pTrail->GetRadarValues().CreateNode() );
				}
			}
		}
	}

	{
		XNode* pNode = xml.AppendChild( "Types" );

		{
			XNode* pNode2 = pNode->AppendChild( "Difficulty" );
			FOREACH_CONST( Difficulty, DIFFICULTIES_TO_SHOW.GetValue(), iter )
			{
				XNode* pNode3 = pNode2->AppendChild( "Difficulty", DifficultyToString(*iter) );
				pNode3->AppendAttr( "DisplayAs", DifficultyToThemedString(*iter) );
			}
		}

		{
			XNode* pNode2 = pNode->AppendChild( "CourseDifficulty" );
			FOREACH_CONST( CourseDifficulty, COURSE_DIFFICULTIES_TO_SHOW.GetValue(), iter )
			{
				XNode* pNode3 = pNode2->AppendChild( "CourseDifficulty", CourseDifficultyToString(*iter) );
				pNode3->AppendAttr( "DisplayAs", CourseDifficultyToThemedString(*iter) );
			}
		}

		{
			XNode* pNode2 = pNode->AppendChild( "StepsType" );
			FOREACH_CONST( StepsType, vStepsTypesToShow, iter )
			{
				XNode* pNode3 = pNode2->AppendChild( "StepsType", GAMEMAN->StepsTypeToString(*iter) );
				pNode3->AppendAttr( "DisplayAs", GAMEMAN->StepsTypeToThemedString(*iter) );
			}
		}

		{
			XNode* pNode2 = pNode->AppendChild( "PlayMode" );
			FOREACH_PlayMode( pm )
			{
				if( !SHOW_PLAY_MODE(pm) )
					continue;
				XNode* pNode3 = pNode2->AppendChild( "PlayMode", PlayModeToString(pm) );
				pNode3->AppendAttr( "DisplayAs", PlayModeToThemedString(pm) );
			}
		}

		{
			XNode* pNode2 = pNode->AppendChild( "Style" );
			vector<const Style*> vpStyle;
			GAMEMAN->GetStylesForGame( GAMESTATE->m_pCurGame, vpStyle );
			FOREACH( const Style*, vpStyle, pStyle )
			{
				if( !SHOW_STYLE(*pStyle) )
					continue;
				StyleID sID;
				sID.FromStyle( (*pStyle) );
				XNode* pNode3 = pNode2->AppendChild( sID.CreateNode() );
				pNode3->AppendAttr( "DisplayAs", GAMEMAN->StyleToThemedString(*pStyle) );
			}
		}

		{
			XNode* pNode2 = pNode->AppendChild( "Meter" );
			for( int i=MIN_METER; i<=MAX_METER; i++ )
			{
				XNode* pNode3 = pNode2->AppendChild( "Meter", ssprintf("Meter%d",i) );
				pNode3->AppendAttr( "DisplayAs", ssprintf("%d",i) );
			}
		}

		{
			XNode* pNode2 = pNode->AppendChild( "Grade" );
			FOREACH_UsedGrade( g )
			{
				XNode* pNode3 = pNode2->AppendChild( "Grade", GradeToString(g) );
				pNode3->AppendAttr( "DisplayAs", GradeToThemedString(g) );
			}
		}

		{
			XNode* pNode2 = pNode->AppendChild( "TapNoteScore" );
			FOREACH_TapNoteScore( tns )
			{
				XNode* pNode3 = pNode2->AppendChild( "TapNoteScore", TapNoteScoreToString(tns) );
				pNode3->AppendAttr( "DisplayAs", TapNoteScoreToThemedString(tns) );
			}
		}

		{
			XNode* pNode2 = pNode->AppendChild( "HoldNoteScore" );
			FOREACH_HoldNoteScore( hns )
			{
				XNode* pNode3 = pNode2->AppendChild( "HoldNoteScore", HoldNoteScoreToString(hns) );
				pNode3->AppendAttr( "DisplayAs", HoldNoteScoreToThemedString(hns) );
			}
		}

		{
			XNode* pNode2 = pNode->AppendChild( "RadarValue" );
			FOREACH_RadarCategory( rc )
			{
				XNode* pNode3 = pNode2->AppendChild( "RadarValue", RadarCategoryToString(rc) );
				pNode3->AppendAttr( "DisplayAs", RadarCategoryToThemedString(rc) );
			}
		}

		{
			XNode* pNode2 = pNode->AppendChild( "Modifier" );
			vector<CString> modifiers;
			THEME->GetModifierNames( modifiers );
			FOREACH_CONST( CString, modifiers, iter )
			{
				PlayerOptions po;
				CString s = *iter;
				po.FromString( s, false );
				vector<CString> v;
				po.GetThemedMods( v );
				if( v.empty() )
					continue;
				XNode* pNode3 = pNode2->AppendChild( "Modifier", *iter );
				pNode3->AppendAttr( "DisplayAs", join(" ",v) );
			}
		}
	}

	xml.AppendChild( "InternetRankingHomeUrl", INTERNET_RANKING_HOME_URL );
	xml.AppendChild( "InternetRankingUploadUrl", INTERNET_RANKING_UPLOAD_URL );
	xml.AppendChild( "InternetRankingViewGuidUrl", INTERNET_RANKING_VIEW_GUID_URL );
	xml.AppendChild( "ProductTitle", PRODUCT_TITLE );
	xml.AppendChild( "FooterText", FOOTER_TEXT );
	xml.AppendChild( "FooterLink", FOOTER_LINK );

	DISP_OPT opts;
	opts.stylesheet = CATALOG_XSL;
	opts.write_tabs = false;
	xml.SaveToFile(fn, &opts);

	LOG->Trace( "Done." );
}

/*
 * (c) 2004 Chris Danford
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
