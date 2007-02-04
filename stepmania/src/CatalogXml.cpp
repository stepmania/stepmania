#include "global.h"
#include "CatalogXml.h"
#include "SongManager.h"
#include "RageLog.h"
#include "song.h"
#include "Steps.h"
#include "XmlFile.h"
#include "XmlFileUtil.h"
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
#include "Style.h"
#include "CommonMetrics.h"
#include "UnlockManager.h"
#include "arch/LoadingWindow/LoadingWindow.h"
#include "LocalizedString.h"

#define SHOW_PLAY_MODE(pm)		THEME->GetMetricB("CatalogXml",ssprintf("ShowPlayMode%s",PlayModeToString(pm).c_str()))
#define SHOW_STYLE(ps)			THEME->GetMetricB("CatalogXml",ssprintf("ShowStyle%s",Capitalize((ps)->m_szName).c_str()))
#define INTERNET_RANKING_HOME_URL	THEME->GetMetric ("CatalogXml","InternetRankingHomeUrl")
#define INTERNET_RANKING_UPLOAD_URL	THEME->GetMetric ("CatalogXml","InternetRankingUploadUrl")
#define INTERNET_RANKING_VIEW_GUID_URL	THEME->GetMetric ("CatalogXml","InternetRankingViewGuidUrl")
#define PRODUCT_TITLE			THEME->GetMetric ("CatalogXml","ProductTitle")
#define FOOTER_TEXT			THEME->GetMetric ("CatalogXml","FooterText")
#define FOOTER_LINK			THEME->GetMetric ("CatalogXml","FooterLink")

// Catalog file paths.
const RString CATALOG_XML       = "Catalog.xml";
const RString CATALOG_XSL       = "Catalog.xsl";
const RString CATALOG_XML_FILE  = "Save/" + CATALOG_XML;

static LocalizedString SAVING_CATALOG_XML( "CatalogXml", "Saving %s ..." );
void CatalogXml::Save( LoadingWindow *loading_window )
{
	ASSERT( SONGMAN );
	ASSERT( UNLOCKMAN );

	if( loading_window )
		loading_window->SetText( ssprintf(SAVING_CATALOG_XML.GetValue(),CATALOG_XML.c_str()) );

	RString fn = CATALOG_XML_FILE;

	LOG->Trace( "Writing %s ...", fn.c_str() );
	XNode xml( "Catalog" );

	const vector<StepsType> &vStepsTypesToShow = CommonMetrics::STEPS_TYPES_TO_SHOW.GetValue();
	const vector<Difficulty> &vDifficultiesToShow = CommonMetrics::DIFFICULTIES_TO_SHOW.GetValue();
	const vector<CourseDifficulty> &vCourseDifficultiesToShow = CommonMetrics::COURSE_DIFFICULTIES_TO_SHOW.GetValue();
	const bool bWriteSimpleValues = THEME->GetMetricB( "RadarValues", "WriteSimpleValues" );
	const bool bWriteComplexValues = THEME->GetMetricB( "RadarValues", "WriteComplexValues" );

	{
		XNode* pNode = xml.AppendChild( "Totals" );
		XNode* pNumSongsByGroup = pNode->AppendChild( "NumSongsByGroup" );
		XNode* pNumStepsByGroup = pNode->AppendChild( "NumStepsByGroup" );

		int iTotalSongs = 0;
		int iTotalSteps = 0;
		vector<RString> vsGroups;
		SONGMAN->GetSongGroupNames( vsGroups );
		FOREACH_CONST( RString, vsGroups, sGroup )
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
				
				int iNumStepsInGroupAndStepsType[NUM_Difficulty];
				ZERO( iNumStepsInGroupAndStepsType );
				FOREACH_CONST( Song*, vpSongsInGroup, i )
				{
					Song *pSong = *i;
				
				/*
				 * Not all songs should be stored in Catalog.xml.  Tutorial songs
				 * are meant to learn how to play the game, not for playing for
				 * a grade.  Locked songs aren't shown to keep them a surprise
				 * until such a time when they are unlocked.
				 */
				
					if( pSong->IsTutorial() )
						continue;	// skip: Tutorial song.
					if( UNLOCKMAN->SongIsLocked(pSong) )
						continue;	// skip: Locked song.
					iTotalSongs++;
					iNumSongsInGroup++;
					FOREACH_CONST( Difficulty, vDifficultiesToShow, dc )
					{
						Steps* pSteps = SongUtil::GetStepsByDifficulty( pSong, *st, *dc, false );	// no autogen
						
						/*
						 * There is no point in storing the steps if either there
						 * are no steps, or the song is locked.
						 */
						
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
			if( !pCourse->AllSongsAreFixed() )
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
			case UnlockRewardType_Song:	iNumUnlockedSongs++;	break;
			case UnlockRewardType_Steps:	iNumUnlockedSteps++;	break;
			case UnlockRewardType_Course:	iNumUnlockedCourses++;	break;
			}
		}

		pNode->AppendChild( "TotalSongs",		iTotalSongs );
		pNode->AppendChild( "TotalSteps",		iTotalSteps );
		pNode->AppendChild( "TotalCourses",		iTotalCourses );
		pNode->AppendChild( "NumUnlockedSongs",		iNumUnlockedSongs );
		pNode->AppendChild( "NumUnlockedSteps",		iNumUnlockedSteps );
		pNode->AppendChild( "NumUnlockedCourses",	iNumUnlockedCourses );
	}

	{
		XNode* pNode = xml.AppendChild( "Songs" );

		const vector<Song*>& vpSongs = SONGMAN->GetAllSongs();

		for( unsigned i=0; i<vpSongs.size(); i++ )
		{
			const Song* pSong = vpSongs[i];
			
			/*
			 * Not all songs should be stored in Catalog.xml.  Tutorial songs
			 * are meant to learn how to play the game, not for playing for
			 * a grade.  Locked songs aren't shown to keep them a surprise
			 * until such a time when they are unlocked.
			 */
			
			if( pSong->IsTutorial() )
				continue;	// skip: Tutorial song.
			if( UNLOCKMAN->SongIsLocked(pSong) )
				continue;	// skip: Locked song.

			SongID songID;
			songID.FromSong( pSong );

			XNode* pSongNode = songID.CreateNode();

			pNode->AppendChild( pSongNode );

			pSongNode->AppendChild( "MainTitle", pSong->GetDisplayMainTitle() );
			pSongNode->AppendChild( "SubTitle", pSong->GetDisplaySubTitle() );

			FOREACH_CONST( StepsType, vStepsTypesToShow, st )
			{
				FOREACH_CONST( Difficulty, vDifficultiesToShow, dc )
				{
					Steps* pSteps = SongUtil::GetStepsByDifficulty( pSong, *st, *dc, false );	// no autogen
					if( pSteps == NULL )
						continue;	// skip
					if( UNLOCKMAN->StepsIsLocked(pSong,pSteps) )
						continue;	// skip

					StepsID stepsID;
					stepsID.FromSteps( pSteps );

					XNode* pStepsIDNode = stepsID.CreateNode();
					pSongNode->AppendChild( pStepsIDNode );
					
					pStepsIDNode->AppendChild( "Meter", pSteps->GetMeter() );
					pStepsIDNode->AppendChild( pSteps->GetRadarValues(PLAYER_1).CreateNode(bWriteSimpleValues, bWriteComplexValues) );
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
			if( !pCourse->AllSongsAreFixed() )
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


			FOREACH_CONST( StepsType, vStepsTypesToShow, st )
			{
				FOREACH_CONST( CourseDifficulty, vCourseDifficultiesToShow, dc )
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
					pTrailIDNode->AppendChild( pTrail->GetRadarValues().CreateNode(bWriteSimpleValues, bWriteComplexValues) );
				}
			}
		}
	}

	{
		XNode* pNode = xml.AppendChild( "Types" );

		{
			XNode* pNode2 = pNode->AppendChild( "Difficulty" );
			FOREACH_CONST( Difficulty, vDifficultiesToShow, iter )
			{
				XNode* pNode3 = pNode2->AppendChild( "Difficulty", DifficultyToString(*iter) );
				pNode3->AppendAttr( "DisplayAs", DifficultyToLocalizedString(*iter) );
			}
		}

		{
			XNode* pNode2 = pNode->AppendChild( "CourseDifficulty" );
			FOREACH_CONST( CourseDifficulty, vCourseDifficultiesToShow, iter )
			{
				XNode* pNode3 = pNode2->AppendChild( "CourseDifficulty", DifficultyToString(*iter) );
				pNode3->AppendAttr( "DisplayAs", CourseDifficultyToLocalizedString(*iter) );
			}
		}

		{
			XNode* pNode2 = pNode->AppendChild( "StepsType" );
			FOREACH_CONST( StepsType, vStepsTypesToShow, iter )
			{
				XNode* pNode3 = pNode2->AppendChild( "StepsType", GAMEMAN->StepsTypeToString(*iter) );
				pNode3->AppendAttr( "DisplayAs", GAMEMAN->StepsTypeToLocalizedString(*iter) );
			}
		}

		{
			XNode* pNode2 = pNode->AppendChild( "PlayMode" );
			FOREACH_PlayMode( pm )
			{
				if( !SHOW_PLAY_MODE(pm) )
					continue;
				XNode* pNode3 = pNode2->AppendChild( "PlayMode", PlayModeToString(pm) );
				pNode3->AppendAttr( "DisplayAs", PlayModeToLocalizedString(pm) );
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
				pNode3->AppendAttr( "DisplayAs", GAMEMAN->StyleToLocalizedString(*pStyle) );
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
				pNode3->AppendAttr( "DisplayAs", GradeToLocalizedString(g) );
			}
		}

		{
			XNode* pNode2 = pNode->AppendChild( "TapNoteScore" );
			FOREACH_TapNoteScore( tns )
			{
				XNode* pNode3 = pNode2->AppendChild( "TapNoteScore", TapNoteScoreToString(tns) );
				pNode3->AppendAttr( "DisplayAs", TapNoteScoreToLocalizedString(tns) );
			}
		}

		{
			XNode* pNode2 = pNode->AppendChild( "HoldNoteScore" );
			FOREACH_HoldNoteScore( hns )
			{
				XNode* pNode3 = pNode2->AppendChild( "HoldNoteScore", HoldNoteScoreToString(hns) );
				pNode3->AppendAttr( "DisplayAs", HoldNoteScoreToLocalizedString(hns) );
			}
		}

		{
			XNode* pNode2 = pNode->AppendChild( "RadarValue" );
			FOREACH_RadarCategory( rc )
			{
				XNode* pNode3 = pNode2->AppendChild( "RadarValue", RadarCategoryToString(rc) );
				pNode3->AppendAttr( "DisplayAs", RadarCategoryToLocalizedString(rc) );
			}
		}

		{
			XNode* pNode2 = pNode->AppendChild( "Modifier" );
			vector<RString> modifiers;
			THEME->GetOptionNames( modifiers );
			FOREACH_CONST( RString, modifiers, iter )
			{
				PlayerOptions po;
				RString s = *iter;
				po.FromString( s, false );
				vector<RString> v;
				po.GetLocalizedMods( v );
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

	XmlFileUtil::SaveToFile( &xml, fn, CATALOG_XSL, false );

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
