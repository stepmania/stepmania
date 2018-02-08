/* RageLog - Manages logging. */

#ifndef RAGE_LOG_H
#define RAGE_LOG_H

#include "enum_flags.h"
#include "fmt/format.h"

ENUM_FLAGS(WriteDest);
/** @brief The possible destinations/options for writing the logs. */
enum class WriteDest
{
	NoDest = 0x00, /**< No destination is set for writing the log. */
	Info = 0x01, /**< If this is set, the message will also be written to info.txt. (info and warnings) */
	UserLog = 0x02, /**< If this is set, the message will also be written to userlog.txt. (user warnings only) */
	Loud = 0x04, /**< Whether this line should be loud when written to log.txt (warnings). */
	Time = 0x08 /**< If this is set, we write the time. */
};

class RageLog
{
public:
	RageLog();
	~RageLog();

	template<typename... Args>
	void Trace(std::string const &msg, Args const & ...args)
	{
		Write(WriteDest::NoDest, fmt::sprintf(msg, args...));
	}

	template<typename... Args>
	void Warn(std::string const &msg, Args const & ...args)
	{
		Write(WriteDest::Info | WriteDest::Loud, fmt::sprintf(msg, args...));
	}

	/** @brief Log data with a severity of info.
	 *
	 * Use this for more important information; it'll always be included
	 * in crash dumps. */
	template<typename... Args>
	void Info(std::string const &msg, Args const & ...args)
	{
		Write(WriteDest::Info, fmt::sprintf(msg, args...));
	}

	// Time is purely for writing profiling time data to the time log. -Kyz
	template<typename... Args>
	void Time(std::string const &msg, Args const & ...args)
	{
		Write(WriteDest::Time, fmt::sprintf(msg, args...));
	}

	template<typename... Args>
	void UserLog(std::string const &sType, std::string const &sElement, std::string const &msg, Args const & ...args)
	{
		std::string result = fmt::sprintf(msg, args...);
		if (!sType.empty())
		{
			result = fmt::sprintf("%s \"%s\" %s", sType.c_str(), sElement.c_str(), result.c_str());
		}

		Write(WriteDest::UserLog, result);
	}

	void Flush();

	template<typename... Args>
	void MapLog(std::string const &key, std::string const &msg, Args const & ...args)
	{
		std::string result = fmt::sprintf(msg, args...);
		StoreMapLog(key, result);
	}

	void UnmapLog( const std::string &key );

	static const char *GetAdditionalLog();
	static const char *GetInfo();
	/* Returns nullptr if past the last recent log. */
	static const char *GetRecentLog( int n );

	void SetShowLogOutput( bool show ); // enable or disable logging to stdout
	void SetLogToDisk( bool b );	// enable or disable logging to file
	void SetInfoToDisk( bool b );	// enable or disable logging info.txt to file
	void SetUserLogToDisk( bool b);	// enable or disable logging user.txt to file
	void SetFlushing( bool b );	// enable or disable flushing

private:
	bool m_bLogToDisk;
	bool m_bInfoToDisk;
	bool m_bUserLogToDisk;
	bool m_bFlush;
	bool m_bShowLogOutput;
	void Write(WriteDest dest, std::string const &line);
	void StoreMapLog(std::string const &key, std::string const &result);
	void UpdateMappedLog();
	void AddToInfo( const std::string &buf );
	void AddToRecentLogs( const std::string &buf );
};

extern RageLog*	LOG;	// global and accessible from anywhere in our program
#endif

/*
 * Copyright (c) 2001-2004 Chris Danford, Glenn Maynard
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
