#ifndef NOTES_WRITER_SM_H
#define NOTES_WRITER_SM_H

#include "song.h"

class RageFile;
class NotesWriterSM
{
	void WriteGlobalTags( RageFile &f, const Song &out );
	void WriteSMNotesTag( const Steps &in, RageFile &f, bool bSavingCache );

public:
	bool Write( CString sPath, const Song &out, bool bSavingCache );
};

#endif
