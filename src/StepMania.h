#ifndef STEP_MANIA_H
#define STEP_MANIA_H

struct Game;
class RageTimer;
class VideoModeParams;

int sm_main( int argc, char* argv[] );

/** @brief Utility functions for controlling the whole game. */
namespace StepMania
{
	void ApplyGraphicOptions();
	void ResetPreferences();
	void ResetGame();
	std::string GetInitialScreen();
	std::string GetSelectMusicScreen();
	void InitializeCurrentGame(const Game* g);

	// If successful, return filename of screenshot in sDir, else return ""
	std::string SaveScreenshot( std::string Dir, bool SaveCompressed, bool MakeSignature, std::string NamePrefix, std::string NameSuffix );

	void InsertCoin( int iNum = 1, bool bCountInBookkeeping = true );
	void InsertCredit();
	void ClearCredits();

	void GetPreferredVideoModeParams( VideoModeParams &paramsOut );
	bool GetHighResolutionTextures();
}

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
