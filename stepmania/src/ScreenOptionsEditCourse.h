#ifndef ScreenOptionsEditCourse_H
#define ScreenOptionsEditCourse_H

#include "ScreenOptionsManageCourses.h"
#include "Course.h"

class ScreenOptionsEditCourse : public ScreenOptionsEditCourseSubMenu
{
public:
	ScreenOptionsEditCourse( CString sName );

	void Init();
	virtual void BeginScreen();

	virtual void HandleScreenMessage( const ScreenMessage SM );

protected:
	virtual void ImportOptions( int iRow, const vector<PlayerNumber> &vpns );
	virtual void ExportOptions( int iRow, const vector<PlayerNumber> &vpns );
	
	virtual void AfterChangeRow( PlayerNumber pn );
	virtual void AfterChangeValueInRow( int iRow, PlayerNumber pn );
	virtual void ProcessMenuStart( PlayerNumber pn, const InputEventType type );

	int GetCourseEntryIndexWithFocus() const;

	Course m_Original;

	vector<Song*>	m_vpDisplayedSongs;	// corresponds with the choices in the Entry row
};

#endif

/*
 * (c) 2003-2004 Chris Danford
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
