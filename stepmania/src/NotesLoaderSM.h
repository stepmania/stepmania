#ifndef NOTES_LOADER_SM_H
#define NOTES_LOADER_SM_H

#include "song.h"
#include "Steps.h"
#include "NotesLoader.h"

class MsdFile;

class SMLoader: public NotesLoader  {
	static void LoadFromSMTokens( 
		CString sNotesType, 
		CString sDescription,
		CString sDifficulty,
		CString sMeter,
		CString sRadarValues,
		CString sNoteData,		
		CString sAttackData,		
		Steps &out);

	bool FromCache;

public:
	SMLoader() { FromCache = false; }
	bool LoadFromSMFile( CString sPath, Song &out );
	bool LoadFromSMFile( CString sPath, Song &out, bool cache )
	{
		FromCache=cache;
		return LoadFromSMFile( sPath, out );
	}

	void GetApplicableFiles( CString sPath, CStringArray &out );
	bool LoadFromDir( CString sPath, Song &out );
	static bool LoadTimingFromFile( const CString &fn, TimingData &out );
	static void LoadTimingFromSMFile( const MsdFile &msd, TimingData &out );
	static bool LoadEdit( CString sEditFilePath, ProfileSlot slot );
};

#endif
