#ifndef RAGELOG_H
#define RAGELOG_H

/*
-----------------------------------------------------------------------------
 Class: RageLog

 Desc: Manages logging

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

class RageLog
{
public:
	RageLog();
	~RageLog();

	void Trace( const char *fmt, ...) PRINTF(2,3);
	void Warn( const char *fmt, ...) PRINTF(2,3);
	void Info( const char *fmt, ...) PRINTF(2,3);
	void Flush();

	void MapLog( const CString &key, const char *fmt, ... ) PRINTF(3,4);
	void UnmapLog( const CString &key );

	static const char *GetAdditionalLog();
	static const char *GetInfo();
	/* Returns NULL if past the last recent log. */
	static const char *GetRecentLog( int n );

	void SetShowLogOutput( bool show ); // enable or disable logging to stdout
	void SetLogToDisk( bool b );	// enable or disable logging to file
	void SetFlushing( bool b );	// enable or disable flushing
	void SetTimestamping( bool b );	// enable or disable timestamping

private:
	bool m_bLogToDisk;
	bool m_bFlush;
	bool m_bTimestamping;
	bool m_bShowLogOutput;
	void Write( int, const CString &str );
	void UpdateMappedLog();
	void AddToInfo( const CString &buf );
	void AddToRecentLogs( const CString &buf );
};

extern RageLog*	LOG;	// global and accessable from anywhere in our program
#endif
