This directory is only for Windows.  For other OS's, install it yourself
as usual.

I'm not adding this library source to CVS, because it's still in heavily
active development and there's not yet a stable release, so it's too much
work to keep a copy here, and because it's easier to create diffs to send
upstream if my copy is checked out of the original repository.  Check it
out yourself if you want to recompile it.

Compile with MingW.
Configured with ./configure --enable-shared --enable-mingw32, with the
ffmpeg.patch.
upx packed, to reduce CVS sizes.

Changes:
Implement shared compiling for MingW. (sent upstream)
Fix localtime_r. (sent upstream)
Don't include windows.h, to make the DLLs smaller. (sent upstream)
Use -g1, not -g, to make the DLLs smaller.  (not yet upstreamed)

TODO: Don't compile encoders, audio decoders and unneeded video decoders.

