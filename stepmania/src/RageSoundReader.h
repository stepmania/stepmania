#ifndef RAGE_SOUND_READER_H
#define RAGE_SOUND_READER_H

/* A sound reader is a source for a RageSound.  It's usually just a convenience
 * wrapper around SDL_Sound. */
class SoundReader {
	mutable string error;

protected:
	void SetError(string e) const { error = e; }

public:
	virtual int GetLength() const = 0; /* ms */
	virtual int GetLength_Fast() const { return GetLength(); } /* ms */
	virtual int SetPosition_Accurate(int ms) = 0;
	virtual int SetPosition_Fast(int ms) { return SetPosition_Accurate(ms); }
	virtual int Read(char *buf, unsigned len) = 0;
	virtual ~SoundReader() { }
	virtual SoundReader *Copy() const = 0;

	bool Error() const { return !error.empty(); }
	string GetError() const { return error; }
};

#endif
