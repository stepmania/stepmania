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

#define SHOW_PLAY_MODE(pm)				THEME->GetMetricB("CatalogXml",ssprintf("ShowPlayMode%s",PlayModeToString(pm).c_str()))
#define SHOW_STYLE(ps)					THEME->GetMetricB("CatalogXml",ssprintf("ShowStyle%s",Capitalize((ps)->m_szName).c_str()))
#define SHOW_STEPS_TYPE(st)				THEME->GetMetricB("CatalogXml",ssprintf("ShowStepsType%s",Capitalize(GAMEMAN->StepsTypeToString(st)).c_str()))
#define INTERNET_RANKING_HOME_URL		THEME->GetMetric ("CatalogXml","InternetRankingHomeUrl")
#define INTERNET_RANKING_UPLOAD_URL		THEME->GetMetric ("CatalogXml","InternetRankingUploadUrl")
#define INTERNET_RANKING_VIEW_GUID_URL	THEME->GetMetric ("CatalogXml","InternetRankingViewGuidUrl")
#define PRODUCT_TITLE					THEME->GetMetric ("CatalogXml","ProductTitle")
#define FOOTER_TEXT						THEME->GetMetric ("CatalogXml","FooterText")
#define FOOTER_LINK						THEME->GetMetric ("CatalogXml","FooterLink")

void SaveCatalogXml()
{
	CString fn = CATALOG_XML_FILE;

	LOG->Trace( "Writing %s ...", fn.c_str() );

	XNode xml;
	xml.name = "Catalog";

	{
		bool ShowStepsType[NUM_STEPS_TYPES];
		FOREACH_StepsType( st )
			ShowStepsType[st] = SHOW_STEPS_TYPE( st );
	
		XNode* pNode = xml.AppendChild( "Songs" );

		vector<Song*> vpSongs = SONGMAN->GetAllSongs();
		for( unsigned i=0; i<vpSongs.size(); i++ )
		{
			Song* pSong = vpSongs[i];
			
			if( pSong->IsTutorial() )
				continue;	// skip

			SongID songID;
			songID.FromSong( pSong );

			XNode* pSongNode = songID.CreateNode();

			pNode->AppendChild( pSongNode );

			pSongNode->AppendChild( "MainTitle", pSong->GetDisplayMainTitle() );
			pSongNode->AppendChild( "SubTitle", pSong->GetDisplaySubTitle() );

			set<Difficulty> vDiffs;
			GAMESTATE->GetDifficultiesToShow( vDiffs );

			FOREACH_StepsType( st )
			{
				if( !ShowStepsType[st] )
					continue;	// skip

				for( set<Difficulty>::const_iterator iter = vDiffs.begin(); iter != vDiffs.end(); iter++ )
				{
					Steps* pSteps = pSong->GetStepsByDifficulty( st, *iter, false );	// no autogen
					if( pSteps == NULL )
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

			CourseID courseID;
			courseID.FromCourse( pCourse );

			XNode* pCourseNode = courseID.CreateNode();

			pNode->AppendChild( pCourseNode );

			pCourseNode->AppendChild( "MainTitle", pCourse->GetDisplayMainTitle() );
			pCourseNode->AppendChild( "SubTitle", pCourse->GetDisplaySubTitle() );
			pCourseNode->AppendChild( "HasMods", pCourse->HasMods() );

			set<CourseDifficulty> vDiffs;
			GAMESTATE->GetCourseDifficultiesToShow( vDiffs );

			FOREACH_StepsType( st )
			{
				if( !SHOW_STEPS_TYPE(st) )
					continue;	// skip

				for( set<CourseDifficulty>::const_iterator iter = vDiffs.begin(); iter != vDiffs.end(); iter++ )
				{
					Trail *pTrail = pCourse->GetTrail( st, *iter );
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
			set<Difficulty> vDiffs;
			GAMESTATE->GetDifficultiesToShow( vDiffs );
			for( set<Difficulty>::const_iterator iter = vDiffs.begin(); iter != vDiffs.end(); iter++ )
			{
				XNode* pNode2 = pNode->AppendChild( "Difficulty", DifficultyToString(*iter) );
				pNode2->AppendAttr( "DisplayAs", DifficultyToThemedString(*iter) );
			}
		}

		{
			set<CourseDifficulty> vDiffs;
			GAMESTATE->GetCourseDifficultiesToShow( vDiffs );
			for( set<CourseDifficulty>::const_iterator iter = vDiffs.begin(); iter != vDiffs.end(); iter++ )
			{
				XNode* pNode2 = pNode->AppendChild( "CourseDifficulty", CourseDifficultyToString(*iter) );
				pNode2->AppendAttr( "DisplayAs", CourseDifficultyToThemedString(*iter) );
			}
		}

		{
			vector<StepsType> vStepsTypes;
			GAMEMAN->GetStepsTypesForGame( GAMESTATE->m_pCurGame, vStepsTypes );
			FOREACH_CONST( StepsType, vStepsTypes, iter )
			{
				if( !SHOW_STEPS_TYPE(*iter) )
					continue;
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
				pNode2->AppendAttr( "DisplayAs", PlayerOptions::ThemeMod(*iter) );
			}
		}
	}

	xml.AppendChild( "InternetRankingHomeUrl", INTERNET_RANKING_HOME_URL );
	xml.AppendChild( "InternetRankingUploadUrl", INTERNET_RANKING_UPLOAD_URL );
	xml.AppendChild( "InternetRankingViewGuidUrl", INTERNET_RANKING_VIEW_GUID_URL );
	xml.AppendChild( "ProductTitle", PRODUCT_TITLE );
	xml.AppendChild( "FooterText", FOOTER_TEXT );
	xml.AppendChild( "FooterLink", FOOTER_LINK );

	DISP_OPT opts = optDefault;
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
