#ifndef INPUT_HANDLER_SEXTETSTREAM
#define INPUT_HANDLER_SEXTETSTREAM

#include "InputHandler.h"

#include <cstdio>
#include <vector>


class InputHandler_SextetStream: public InputHandler
{
public:
	InputHandler_SextetStream();
	~InputHandler_SextetStream();
	void GetDevicesAndDescriptions(std::vector<InputDeviceInfo>& vDevicesOut);

public:
	class Impl;
protected:
	Impl * _impl;
};

// Note: InputHandler_SextetStreamFromFile uses blocking I/O. For the
// handler thread to close in a timely fashion, the producer of data for the
// file (e.g. the program at the other end of the pipe) must either close
// the file or output and flush a line of data no less often than about once
// per second, even if there has been no change. (Repeating the most recent
// state accomplishes this without triggering any new events.) Either of
// these interrupts the blocking read so that the loop can check its
// continue flag.
class InputHandler_SextetStreamFromFile: public InputHandler_SextetStream
{
public:
	// Note: In the current implementation, the filename (either the
	// `filename` parameter or the `SextetStreamInputFilename` setting) is
	// passed to fopen(), not a RageFile ctor, so specify the file to be
	// opened on the actual filesystem instead of the mapped filesystem. (I
	// couldn't get RageFile to work here, possibly because I haven't
	// determined how to disable buffering on an input file.)
	InputHandler_SextetStreamFromFile();
};

#endif

/*
 * Copyright © 2014 Peter S. May
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
