#include "stdafx.h"
#include "NotesLoader.h"
#include "NoteTypes.h"
#include "GameManager.h"

bool NotesLoader::Loadable( CString sPath )
{
	CStringArray list;
	GetApplicableFiles( sPath, list );
	return list.GetSize() != 0;
}
