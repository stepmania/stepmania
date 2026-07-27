#ifndef ARCH_HOOKS_H
#define ARCH_HOOKS_H

#include <cstdint>
#include <ctime>

struct lua_State;
class ArchHooks
{
public:
	static ArchHooks *Create();
	ArchHooks();
	virtual ~ArchHooks() { }
	virtual void Init() { }
	/*
	 * Return the general name of the architecture, eg. "Windows", "macOS", "Unix".
	 */
	virtual RString GetArchName() const { return "generic"; }

	/* This is called as soon as the loading window is shown, and we can
	 * safely log. */
	virtual void DumpDebugInfo() { }

	/**
	 * @brief Re-exec the game.
	 *
	 * If this is implemented, it doesn't return. */
	virtual void RestartProgram() { }

	/*
	 * Get the 2-letter RFC-639 code of the user's preferred language
	 * for localized messages, in lowercase.
	 */
	static RString GetPreferredLanguage();

	/* If this is a second instance, return true.
	 * Optionally, give focus to the existing window. */
	virtual bool CheckForMultipleInstances(int /* argc */, char* [] /* argv[] */) { return false; }

	virtual void SetTime( tm ) { }

	virtual void BoostPriority() { }
	virtual void UnBoostPriority() { }

	/**
	 * @brief Setup the rendering threads for concurrency.
	 *
	 * The priority of the concurrent rendering thread may need to be boosted
	 * on some schedulers.
	 */
	virtual void SetupConcurrentRenderingThread() { }

	/**
	 * @brief Determine if the user wants to quit (eg. ^C, or clicked a "close window" button).
	 * @return true if the user wants to quit, false otherwise. */
	static bool UserQuit() { return g_bQuitting; }
	static void SetUserQuit() { g_bQuitting = true; }

	/*
	 * Returns true if the user wants to toggle windowed mode and atomically clears
	 * the boolean.
	 */
	static bool GetAndClearToggleWindowed();
	static void SetToggleWindowed();

	/*
	 * Return the amount of time since the program started.  (This may actually be
	 * since the initialization of HOOKS.
	 *
	 * Full microsecond accuracy may not be available.
	 *
	 * bAccurate is a hint: it specifies whether to prefer short-term precision
	 * or long-term accuracy.  If false, the implementation may give higher resolution
	 * results, but not be as stable over long periods (eg. may drift depending on
	 * clock speed shifts on laptops).  If true, lower precision results (usually with
	 * no less than a 1ms granularity) are returned, but the results should be stable
	 * over long periods of time.
	 *
	 * Note that bAccurate may change the result significantly; it may use a different
	 * timer, and may have a different concept of when the program "started".
	 *
	 * This is a static function, implemented in whichever ArchHooks source is used,
	 * so it can be used at any time (such as in global constructors), before HOOKS
	 * is initialized.
	 *
	 * RageTimer layers on top of this, and attempts to correct wrapping, as the
	 * underlying timers may be 32-bit, but implementations should try to avoid
	 * wrapping if possible.
	 */
	static std::int64_t GetMicrosecondsSinceStart( bool bAccurate );

	/*
	 * Add file search paths, higher priority first.
	 */
	static void MountInitialFilesystems( const RString &sDirOfExecutable );

	/*
	 * Add file search paths for user-writable directories.
	 */
	static void MountUserFilesystems( const RString &sDirOfExecutable );

	/*
	 * Platform-specific code calls this to indicate focus changes.
	 */
	void SetHasFocus( bool bAppHasFocus );

	/*
	 * Return true if the application has input focus.
	 */
	bool AppHasFocus() const { return m_bHasFocus; }

	/*
	 * Returns true if the application's focus has changed since last called.
	 */
	bool AppFocusChanged();

	/*
	 * Open a URL in the default web browser
	 */
	virtual bool GoToURL( RString sUrl );

	virtual float GetDisplayAspectRatio() = 0;

	/** @brief Fetch the contents of the system clipboard. */
	virtual RString GetClipboard();

	// Lua
	void PushSelf( lua_State *L );
	void RegisterWithLua();

private:
	/* This are helpers for GetMicrosecondsSinceStart on systems with a timer
	 * that may loop or move backwards. */
	static std::int64_t FixupTimeIfLooped( std::int64_t usecs );
	static std::int64_t FixupTimeIfBackwards( std::int64_t usecs );

	static bool g_bQuitting;
	static bool g_bToggleWindowed;
	bool m_bHasFocus;
	bool m_bFocusChanged;
};

#endif

extern ArchHooks *HOOKS;	// global and accessible from anywhere in our program

/**
 * @file
 * @author Glenn Maynard, Chris Danford (c) 2003-2004
 * @section LICENSE
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
