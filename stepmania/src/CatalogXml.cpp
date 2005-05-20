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

		int iTotalSongs = 0;
		int iTotalSteps = 0;
		FOREACH_CONST( Song*, SONGMAN->GetAllSongs(), i )
		{
			Song *pSong = *i;
			if( pSong->IsTutorial() )
				continue;	// skip
			if( UNLOCKMAN->SongIsLocked(pSong) )
				continue;	// skip
			iTotalSongs++;

			FOREACH_CONST( StepsType, vStepsTypesToShow, st )
			{
				FOREACH_CONST( Difficulty, DIFFICULTIES_TO_SHOW.GetValue(), dc )
				{
					Steps* pSteps = pSong->GetStepsByDifficulty( *st, *dc, false );	// no autogen
					if( pSteps == NULL )
						continue;	// skip
					if( UNLOCKMAN->StepsIsLocked(pSong,pSteps) )
						continue;	// skip
					iTotalSteps++;
				}
			}
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
			FOREACH_CONST( Difficulty, DIFFICULTIES_TO_SHOW.GetValue(), iter )
			{
				XNode* pNode2 = pNode->AppendChild( "Difficulty", DifficultyToString(*iter) );
				pNode2->AppendAttr( "DisplayAs", DifficultyToThemedString(*iter) );
			}
		}

		{
			FOREACH_CONST( CourseDifficulty, COURSE_DIFFICULTIES_TO_SHOW.GetValue(), iter )
			{
				XNode* pNode2 = pNode->AppendChild( "CourseDifficulty", CourseDifficultyToString(*iter) );
				pNode2->AppendAttr( "DisplayAs", CourseDifficultyToThemedString(*iter) );
			}
		}

		{
			FOREACH_CONST( StepsType, vStepsTypesToShow, iter )
			{
				XNode* pNode2 = pNode->AppendChild( "StepsType", GAMEMAN->StepsTypeToString(*iter) );
				pNode2->AppendAttr( "DisplayAs", GAMEMAN->StepsTypeToThemedString(*iter) );
			}
		}

		{
			FOREACH_PlayMode( pm )
			{
				if( !SHOW_PLAY_MODE(pm) )
					continue;
				XNode* pNode2 = pNode->AppendChild( "PlayMode", PlayModeToString(pm) );
				pNode2->AppendAttr( "DisplayAs", PlayModeToThemedString(pm) );
			}
		}

		{
			vector<const Style*> vpStyle;
			GAMEMAN->GetStylesForGame( GAMESTATE->m_pCurGame, vpStyle );
			FOREACH( const Style*, vpStyle, pStyle )
			{
				if( !SHOW_STYLE(*pStyle) )
					continue;
				StyleID sID;
				sID.FromStyle( (*pStyle) );
				XNode* pNode2 = pNode->AppendChild( sID.CreateNode() );
				pNode2->AppendAttr( "DisplayAs", GAMEMAN->StyleToThemedString(*pStyle) );
			}
		}

		{
			for( int i=MIN_METER; i<=MAX_METER; i++ )
			{
				XNode* pNode2 = pNode->AppendChild( "Meter", ssprintf("Meter%d",i) );
				pNode2->AppendAttr( "DisplayAs", ssprintf("%d",i) );
			}
		}

		{
			FOREACH_UsedGrade( g )
			{
				XNode* pNode2 = pNode->AppendChild( "Grade", GradeToString(g) );
				pNode2->AppendAttr( "DisplayAs", GradeToThemedString(g) );
			}
		}

		{
			FOREACH_TapNoteScore( tns )
			{
				XNode* pNode2 = pNode->AppendChild( "TapNoteScore", TapNoteScoreToString(tns) );
				pNode2->AppendAttr( "DisplayAs", TapNoteScoreToThemedString(tns) );
			}
		}

		{
			FOREACH_HoldNoteScore( hns )
			{
				XNode* pNode2 = pNode->AppendChild( "HoldNoteScore", HoldNoteScoreToString(hns) );
				pNode2->AppendAttr( "DisplayAs", HoldNoteScoreToThemedString(hns) );
			}
		}

		{
			FOREACH_RadarCategory( rc )
			{
				XNode* pNode2 = pNode->AppendChild( "RadarValue", RadarCategoryToString(rc) );
				pNode2->AppendAttr( "DisplayAs", RadarCategoryToThemedString(rc) );
			}
		}

		{
			set<CString> modifiers;
			THEME->GetModifierNames( modifiers );
			for( set<CString>::const_iterator iter = modifiers.begin(); iter != modifiers.end(); iter++ )
			{
				XNode* pNode2 = pNode->AppendChild( "Modifier", *iter );
				PlayerOptions po;
				CString s = *iter;
				po.FromString( s, false );
				vector<CString> v;
				po.GetThemedMods( v );
				if( v.empty() )
					continue;
				pNode2->AppendAttr( "DisplayAs", join(" ",v) );
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
