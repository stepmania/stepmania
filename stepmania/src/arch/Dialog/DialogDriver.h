#ifndef DIALOG_BOX_DRIVER_H
#define DIALOG_BOX_DRIVER_H

#include "Dialog.h"
#include "RageUtil.h"

class DialogDriver
{
public:
	static DialogDriver *Create();

	virtual void Error( RString sMessage, RString sID ) { printf("Error: %s\n", sMessage.c_str()); }
	virtual void OK( RString sMessage, RString sID ) {}
	virtual Dialog::Result AbortRetryIgnore( RString sMessage, RString sID ) { return Dialog::ignore; } 
	virtual Dialog::Result AbortRetry( RString sMessage, RString sID ) { return Dialog::abort; } 

	virtual RString Init() { return RString(); }
	virtual ~DialogDriver() { }
};
class DialogDriver_Null : public DialogDriver { };

typedef DialogDriver *(*CreateDialogDriverFn)();
struct RegisterDialogDriver
{
	static map<istring, CreateDialogDriverFn> *g_pRegistrees;
	RegisterDialogDriver( const istring &sName, CreateDialogDriverFn pfn );
};
#define REGISTER_DIALOG_DRIVER_CLASS( name ) \
	static RegisterDialogDriver register_##name( #name, CreateClass<DialogDriver_##name, DialogDriver> )


#endif

/*
 * (c) 2003-2004 Glenn Maynard, Chris Danford
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
