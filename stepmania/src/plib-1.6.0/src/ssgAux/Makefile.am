if BUILD_SSGAUX

lib_LIBRARIES = libplibssgaux.a

include_HEADERS = ssgAux.h             \
                  ssgaShapes.h         \
                  ssgaLensFlare.h      \
                  ssgaParticleSystem.h \
                  ssgaWaveSystem.h     \
                  ssgaFire.h

libplibssgaux_a_SOURCES = ssgAux.cxx               \
                          ssgaShapes.cxx           \
                          ssgaPatch.cxx            \
                          ssgaParticleSystem.cxx   \
                          ssgaWaveSystem.cxx       \
                          ssgaLensFlare.cxx        \
                          ssgaLensFlareTexture.cxx \
                          ssgaFire.cxx             \
                          ssgaTeapot.cxx

INCLUDES = -I$(top_srcdir)/src/sg -I$(top_srcdir)/src/ssg
INCLUDES += -I$(top_srcdir)/src/util

endif

EXTRA_DIST = ssgAux.dsp


