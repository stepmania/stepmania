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
