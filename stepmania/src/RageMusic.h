#ifndef RAGE_MUSIC_H

class RageSound;
class RageMusic
{
	RageSound *music;

public:
	RageMusic();
	~RageMusic();

	void Play(CString file, bool force_loop = false, float start_sec = -1, float length_sec = -1, float fade_len = 0);
	void Stop() { Play(""); }
	CString GetPath() const;
};

extern RageMusic *MUSIC;
#endif
