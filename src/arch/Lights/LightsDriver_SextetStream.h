#ifndef LightsDriver_SextetStream_H
#define LightsDriver_SextetStream_H

/*
 * `LightsDriver_SextetStream` (abstract): Streams the light data (in
 * ASCII-safe sextets) to some output stream.
 *
 * *   `LightsDriver_SextetStreamToFile`: Streams the light data to an
 *     output file.
 *     *   The specified file may be a named pipe (Windows)/named fifo
 *         (Linux, others). This makes it possible to implement an
 *         out-of-process light controller without touching the StepMania
 *         source and without using C++. See the included notes for
 *         details.
 */

#include "LightsDriver.h"
#include "RageFile.h"

class LightsDriver_SextetStream : public LightsDriver
{
public:
	LightsDriver_SextetStream();
	virtual ~LightsDriver_SextetStream();
	virtual void Set(const LightsState *ls);
protected:
	void * _impl;
};

class LightsDriver_SextetStreamToFile : public LightsDriver_SextetStream
{
public:
	LightsDriver_SextetStreamToFile();
	LightsDriver_SextetStreamToFile(const RString& filename);

	// The file object passed here should already be open, and will be
	// flushed, closed, and deleted in the destructor.
	LightsDriver_SextetStreamToFile(RageFile * file);
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
