#include "global.h"
#include "RageSoundReader_Resample.h"
#include "RageSoundReader_Resample_Fast.h"
#include "RageSoundReader_Resample_Good.h"

RageSoundReader_Resample *RageSoundReader_Resample::MakeResampler( ResampleQuality q )
{
	switch( q )
	{
	case RESAMP_FAST:
		return new RageSoundReader_Resample_Fast;
	case RESAMP_NORMAL:
	case RESAMP_HIGHQUALITY:
	{
		RageSoundReader_Resample_Good *ret = new RageSoundReader_Resample_Good;
		ret->SetHighQuality( q == RESAMP_HIGHQUALITY );
		return ret;
	}
	default:
		ASSERT(0);
		return NULL;
	}
}

/*
 * Copyright (c) 2003 Glenn Maynard
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
