#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: CatalogXml

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "CatalogXml.h"
#include "SongManager.h"
#include "RageLog.h"
#include "song.h"
#include "Steps.h"
#include "XmlFile.h"
#include "Course.h"

const CString CATALOG_XML	= "Catalog.xml";

void SaveCatalogXml( CString sDir )
{
	CString fn = sDir + CATALOG_XML;

	LOG->Trace( "Writing %s ...", fn.c_str() );

	XNode xml;
	xml.name = "Catalog";

	vector<Song*> vpSongs = SONGMAN->GetAllSongs();
	for( unsigned i=0; i<vpSongs.size(); i++ )
	{
		Song* pSong = vpSongs[i];

		SongID songID;
		songID.FromSong( pSong );

		XNode* pSongNode = songID.CreateNode();

		xml.AppendChild( pSongNode );

		pSongNode->AppendChild( "MainTitle", pSong->GetDisplayMainTitle() );
		pSongNode->AppendChild( "SubTitle", pSong->GetDisplaySubTitle() );

		vector<Steps*> vpSteps = pSong->GetAllSteps();
		for( unsigned j=0; j<vpSteps.size(); j++ )
		{
			Steps* pSteps = vpSteps[j];
		
			if( pSteps->IsAutogen() )
				continue;

			StepsID stepsID;
			stepsID.FromSteps( pSteps );

			pSongNode->AppendChild( stepsID.CreateNode() );
		}
	}


	vector<Course*> vpCourses;
	SONGMAN->GetAllCourses( vpCourses, false );
	for( unsigned i=0; i<vpCourses.size(); i++ )
	{
		Course* pCourse = vpCourses[i];

		CourseID courseID;
		courseID.FromCourse( pCourse );

		XNode* pCourseNode = courseID.CreateNode();

		xml.AppendChild( pCourseNode );

		pCourseNode->AppendChild( "Title", pCourse->m_sName );
		pCourseNode->AppendChild( "HasMods", pCourse->HasMods() );

		FOREACH_StepsType( st )
		{
			FOREACH_CourseDifficulty( cd )
			{
				Trail *pTrail = pCourse->GetTrail( st, cd );
				if( pTrail == NULL )
					continue;
				
				TrailID trailID;
				trailID.FromTrail( pTrail );

				pCourseNode->AppendChild( trailID.CreateNode() );
			}
		}
	}

	xml.SaveToFile(fn);
}
