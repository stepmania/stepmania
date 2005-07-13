#ifndef STEPMANIA_H
#define STEPMANIA_H

class Game;
class RageTimer;

#ifdef _XBOX
void __cdecl main();
#else
int main( int argc, char* argv[] );
#endif
void ApplyGraphicOptions();
void NORETURN HandleException( CString error );
void ExitGame();
void ResetGame();
void SaveGamePrefsToDisk();
void ChangeCurrentGame( const Game* g );
void FocusChanged( bool bHasFocus );
bool AppHasFocus();

// If successful, return filename of screenshot in sDir, else return ""
CString SaveScreenshot( CString sDir, bool bSaveCompressed, bool bMakeSignature, int iIndex = -1 );

void InsertCoin( int iNum = 1, const RageTimer *pTime = NULL );
void InsertCredit();

extern int g_argc;
extern char **g_argv;
bool GetCommandlineArgument( const CString &option, CString *argument=NULL, int iIndex=0 );

#endif

/*
 * (c) 2001-2004 Chris Danford
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
