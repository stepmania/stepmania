if BUILD_NET

lib_LIBRARIES = libplibnet.a

include_HEADERS = netBuffer.h netChannel.h netChat.h netMessage.h \
	netMonitor.h netSocket.h

libplibnet_a_SOURCES = netBuffer.cxx netChannel.cxx netChat.cxx \
	netMessage.cxx netMonitor.cxx netSocket.cxx

INCLUDES = -I$(top_srcdir)/src/util

endif

EXTRA_DIST = net.dsp

