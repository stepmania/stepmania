#ifndef ARCH_HOOKS_H
#define ARCH_HOOKS_H

/*
 * I'm undecided about when it's a good idea to use arch; I'm experimenting
 * a bit.
 *
 * This arch driver is simply called at various important points, to let
 * architectures do things.  Windows can use this to start up the crash
 * handler and to output Windows-specific debug information, for example.
 *
 * This class should have one-way data transfer only: functions are called,
 * they may do something, and no data is returned.  (We don't want to have
 * to deal with error handling and so on when using these functions.)
 *
 * This class is constructed at the very start of the program.
 */
class ArchHooks
{
protected:
    bool MessageIsIgnored( CString ID );
    void IgnoreMessage( CString ID );
	
public:
	/* This is called as soon as SDL is set up, the loading window is shown
	 * and we can safely log and throw. */
	virtual void DumpDebugInfo() { }

	/* Message box stuff.  Not supported on all archs.  The ID can be used
	 * to identify a class of messages, for "don't display this dialog"-type
	 * prompts.  (Don't overload these, overload *Private below.) */
	enum MessageBoxResult { abort, retry, ignore, cancel };
	void MessageBoxError( CString error, CString ID = "" );
	void MessageBoxOK( CString sMessage, CString ID = "" );
	MessageBoxResult MessageBoxAbortRetryIgnore( CString sMessage, CString ID = "" );
	MessageBoxResult MessageBoxRetryCancel( CString sMessage, CString ID = "" );

	virtual ~ArchHooks() { }
	/* This is called once each time through the game loop */
	virtual void Update(float delta) { }

	/* Re-exec the game.  If this is implemented, it doesn't return. */
	virtual void RestartProgram() { }

protected:
	virtual void MessageBoxErrorPrivate( CString sMessage, CString ID ) { printf("Error: %s\n", sMessage.c_str()); }
	virtual void MessageBoxOKPrivate( CString sMessage, CString ID ) {}
	virtual MessageBoxResult MessageBoxAbortRetryIgnorePrivate( CString sMessage, CString ID ) { return ignore; } 
	virtual MessageBoxResult MessageBoxRetryCancelPrivate( CString sMessage, CString ID ) { return cancel; } 

};

#endif

extern ArchHooks *HOOKS;	// global and accessable from anywhere in our program

/*
 * Copyright (c) 2002-2003 by the person(s) listed below.  All rights reserved.
 *
 * Glenn Maynard
 */
