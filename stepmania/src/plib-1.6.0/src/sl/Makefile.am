if BUILD_SL

lib_LIBRARIES = libplibsl.a libplibsm.a

include_HEADERS = sl.h slPortability.h sm.h

libplibsl_a_SOURCES = \
	slDSP.cxx slSample.cxx slEnvelope.cxx \
	slPlayer.cxx slMODPlayer.cxx slSamplePlayer.cxx \
        slScheduler.cxx slMODdacio.cxx slMODfile.cxx \
        slMODinst.cxx slMODnote.cxx slMODPrivate.h slMODfile.h

libplibsm_a_SOURCES = slPortability.h smMixer.cxx

INCLUDES = -I$(top_srcdir)/src/util

endif

EXTRA_DIST = sl.dsp

