#ifndef SCREEN_RELOAD_SONGS_H
#define SCREEN_RELOAD_SONGS_H

#include "Screen.h"
#include "BitmapText.h"
class LoadingWindow;

class ScreenReloadSongs: public Screen
{
public:
	ScreenReloadSongs( CString sClassName );
	~ScreenReloadSongs();
	void Update( float fDeltaTime );

private:
	int m_iUpdates;
	LoadingWindow *m_LoadingWindow;
	BitmapText m_Loading;
};

#endif
