if !SYSTEM_FFMPEG
all-local:
	cd ffmpeg && $(MAKE) 
if WINDOWS
	cp ffmpeg/*/*.dll ..
endif

del-dlls:
if WINDOWS
	rm ../*.dll
endif

clean-local: del-dlls
	cd ffmpeg && $(MAKE) clean

distclean-local: del-dlls
	cd ffmpeg && $(MAKE) distclean
endif

