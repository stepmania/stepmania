#ifndef CommandLineActions_H
#define CommandLineActions_H

#include <vector>


class LoadingWindow;

/** @brief The collection of command line actions. */
namespace CommandLineActions
{
	/**
	 * @brief Perform a utility function, then exit.
	 * @param pLW the LoadingWindow that is presently not used? */
	void Handle(LoadingWindow* pLW);

	/** @brief The housing for the command line arguments. */
	class CommandLineArgs
	{
	public:
		/** @brief the arguments in question. */
		std::vector<RString> argv;
	};
	/**
	 * @brief A list of command line arguemnts to process while the game is running.
	 * These args could have come from this process or passed to this process
	 * from another process. */
	extern std::vector<CommandLineArgs> ToProcess;
}

#endif

/**
 * @file
 * @author Chris Danford, Steve Checkoway (c) 2006
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

