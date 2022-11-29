#include "global.h"
#include "LightsDriver_SextetStream.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "RageUtil.h"
#include "SextetUtils.h"

#include <cstring>

using namespace std;


// Private members/methods are kept out of the header using an opaque pointer `_impl`.
// Google "pimpl idiom" for an explanation of what's going on and why it is (or might be) useful.


// Implementation class

namespace
{
	class Impl
	{
	protected:
		uint8_t lastOutput[FULL_SEXTET_COUNT];
		RageFile * out;

	public:
		Impl(RageFile * file) {
			out = file;

			// Ensure a non-match the first time
			lastOutput[0] = 0;
		}

		virtual ~Impl() {
			if(out != nullptr)
			{
				out->Flush();
				out->Close();
				SAFE_DELETE(out);
			}
		}

		void Set(const LightsState * ls)
		{
			uint8_t buffer[FULL_SEXTET_COUNT];

			packLine(buffer, ls);

			// Only write if the message has changed since the last write.
			if(memcmp(buffer, lastOutput, FULL_SEXTET_COUNT) != 0)
			{
				if(out != nullptr)
				{
					out->Write(buffer, FULL_SEXTET_COUNT);
					out->Flush();
				}

				// Remember last message
				memcpy(lastOutput, buffer, FULL_SEXTET_COUNT);
			}
		}
	};
}


// LightsDriver_SextetStream interface
// (Wrapper for Impl)

#define IMPL ((Impl*)_impl)

LightsDriver_SextetStream::LightsDriver_SextetStream()
{
	_impl = nullptr;
}

LightsDriver_SextetStream::~LightsDriver_SextetStream()
{
	if(IMPL != nullptr)
	{
		delete IMPL;
	}
}

void LightsDriver_SextetStream::Set(const LightsState *ls)
{
	if(IMPL != nullptr)
	{
		IMPL->Set(ls);
	}
}


// LightsDriver_SextetStreamToFile implementation

REGISTER_LIGHTS_DRIVER_CLASS(SextetStreamToFile);

#if defined(_WINDOWS)
	#define DEFAULT_OUTPUT_FILENAME "\\\\.\\pipe\\StepMania-Lights-SextetStream"
#else
	#define DEFAULT_OUTPUT_FILENAME "Data/StepMania-Lights-SextetStream.out"
#endif
static Preference<RString> g_sSextetStreamOutputFilename("SextetStreamOutputFilename", DEFAULT_OUTPUT_FILENAME);

inline RageFile * openOutputStream(const RString& filename)
{
	RageFile * file = new RageFile;

	if(!file->Open(filename, RageFile::WRITE|RageFile::STREAMED))
	{
		LOG->Warn("Error opening file '%s' for output: %s", filename.c_str(), file->GetError().c_str());
		SAFE_DELETE(file);
		file = nullptr;
	}

	return file;
}

LightsDriver_SextetStreamToFile::LightsDriver_SextetStreamToFile(RageFile * file)
{
	_impl = new Impl(file);
}

LightsDriver_SextetStreamToFile::LightsDriver_SextetStreamToFile(const RString& filename)
{
	_impl = new Impl(openOutputStream(filename));
}

LightsDriver_SextetStreamToFile::LightsDriver_SextetStreamToFile()
{
	_impl = new Impl(openOutputStream(g_sSextetStreamOutputFilename));
}

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
