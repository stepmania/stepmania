#ifndef NOTES_WRITER_SM_H
#define NOTES_WRITER_SM_H

#include "song.h"

class NotesWriterSM
{
	void WriteGlobalTags(FILE *fp, const Song &out);
	void WriteSMNotesTag( const Steps &in, FILE* fp );

public:
	bool Write( CString sPath, const Song &out, bool bSavingCache );
};

#endif
