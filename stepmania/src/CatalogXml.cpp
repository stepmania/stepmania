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
#include "song.h"
#include "Steps.h"

const CString CATALOG_XML	= "Catalog.xml";

void SaveCatalogXml( CString sDir )
{
	CString fn = sDir + CATALOG_XML;

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

	bool bSaved = xml.SaveToFile(fn);
}
