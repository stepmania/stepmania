#ifndef RAGE_SOUND_MANAGER_H
#define RAGE_SOUND_MANAGER_H

#include <set>
#include <map>
#include "RageUtil_CircularBuffer.h"

class RageSound;
class RageSoundBase;
class RageSoundDriver;
struct RageSoundParams;

/* This is a temporary hack, to try to track down an obscure crash. */
#if defined(_WINDOWS) && _MSC_VER >= 1300 
#include <windows.h>

extern set<void *> g_ProtectedPages;
template<typename T>
class ProtAllocator
{
public:
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    template<typename U> struct rebind
	{
        typedef ProtAllocator<U> other;
    };

	explicit ProtAllocator() {}
	~ProtAllocator() {}
	ProtAllocator( ProtAllocator const & ) {}
	template<typename U> ProtAllocator( ProtAllocator<U> const& ) {}

	pointer address( reference r ) { return &r; }
	const_pointer address( const_reference r ) { return &r; }

	pointer allocate( size_type cnt, typename std::allocator<void>::const_pointer = 0 )
	{ 
		cnt *= sizeof (T);
		void *p = VirtualAlloc( NULL, cnt, MEM_COMMIT, PAGE_READWRITE );
		g_ProtectedPages.insert( p );

		return reinterpret_cast<pointer>( p );
	}
	void deallocate( pointer p, size_type s )
	{
		VirtualFree( p, 0, MEM_RELEASE );
		g_ProtectedPages.erase( p );
	}

	size_type max_size() const
	{
		return 2147483648 / sizeof(T);
	}

	void construct( pointer p, const T& t ) { new(p) T(t); }
	void destroy( pointer p ) { p->~T(); }

	bool operator==( ProtAllocator const& ) { return true; }
	bool operator!=( ProtAllocator const& a ) { return !operator==(a); }
};

#else
#define ProtAllocator allocator
#endif


class RageSoundManager
{
	/* Set of sounds that we've taken over (and are responsible for deleting
	 * when they're finished playing): */
	set<RageSound *> owned_sounds;
	set<RageSound *> playing_sounds;

	/* A list of all sounds that currently exist. */
	typedef set<RageSound *, less<RageSound*>, ProtAllocator<RageSound*> > all_sounds_type;
	all_sounds_type all_sounds;
	
	RageSoundDriver *driver;

	/* Prefs: */
	float MixVolume;
	struct queued_pos_map_t
	{
		int ID, pos, got_frames;
		int64_t frameno;
	};

	CircBuf<queued_pos_map_t> pos_map_queue;

public:
	RageSoundManager();
	~RageSoundManager();
	void Init( CString drivers );

	float GetMixVolume() const { return MixVolume; }
	void SetPrefs(float MixVol);

	void Update(float delta);
	void StartMixing( RageSoundBase *snd );	/* used by RageSound */
	void StopMixing( RageSoundBase *snd );	/* used by RageSound */
	int64_t GetPosition( const RageSoundBase *snd ) const;	/* used by RageSound */
	void RegisterSound( RageSound *p );		/* used by RageSound */
	void UnregisterSound( RageSound *p );	/* used by RageSound */
	int GetUniqueID();						/* used by RageSound */
	void RegisterPlayingSound( RageSound *p );	/* used by RageSound */
	void UnregisterPlayingSound( RageSound *p );	/* used by RageSound */
	void CommitPlayingPosition( int ID, int64_t frameno, int pos, int got_bytes );	/* used by drivers */
	float GetPlayLatency() const;
	int GetDriverSampleRate( int rate ) const;
	set<RageSound *> GetPlayingSounds() const;

	/* When deleting a sound from any thread except the one calling Update(), this
	 * must be used to prevent race conditions. */
	void DeleteSound( RageSound *p );

	void PlayOnce( CString sPath );

	RageSound *PlaySound( RageSound &snd, const RageSoundParams *params );
	void StopPlayingAllCopiesOfSound(RageSound &snd);

	/* Stop all sounds that were started by this thread.  This should be called
	 * before exiting a thread. */
	void StopPlayingSoundsForThisThread();

	void GetCopies( RageSound &snd, vector<RageSound *> &snds, bool bLockSounds=false );

	static void AttenuateBuf( int16_t *buf, int samples, float vol );

private:
	void FlushPosMapQueue();
	RageSound *GetSoundByID( int ID );
};

/* This inputs and outputs 16-bit 44khz stereo input. */
class SoundMixBuffer
{
	int32_t *mixbuf;
	unsigned bufsize; /* actual allocated samples */
	unsigned used; /* used samples */
	int vol; /* vol * 256 */

public:
	void write( const int16_t *buf, unsigned size, float volume = -1, int offset = 0 );
	void read(int16_t *buf);
	void read( float *buf );
	unsigned size() const { return used; }
	void SetVolume(float f);

	SoundMixBuffer();
	~SoundMixBuffer();
};

extern RageSoundManager *SOUNDMAN;

#endif

/*
 * Copyright (c) 2002-2004 Glenn Maynard
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
