#ifndef NOTES_WRITER_SM_H
#define NOTES_WRITER_SM_H

#include "Song.h"

class NotesWriterSM
{
	void WriteGlobalTags(FILE *fp, const Song &out);
	void WriteSMNotesTag( const Notes &in, FILE* fp );

public:
	bool Write( CString sPath, const Song &out, bool bSavingCache );
};

#endif
