#ifndef SCREEN_RELOAD_SONGS_H
#define SCREEN_RELOAD_SONGS_H

#include "Screen.h"

class ScreenReloadSongs: public Screen
{
	bool m_FirstUpdate;

public:
	ScreenReloadSongs( CString sClassName );
	void Update( float fDeltaTime );
};

#endif
