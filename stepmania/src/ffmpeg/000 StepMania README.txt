This directory is only for Windows.  For other OS's, install it yourself
as usual.

I'm not adding this library source to CVS, because it's still in heavily
active development and there's not yet a stable release, so it's too much
work to keep a copy here, and because it's easier to create diffs to send
upstream if my copy is checked out of the original repository.  Check it
out yourself if you want to recompile it.

Compile with MingW.
Configured with ./configure --enable-shared --enable-mingw32 --enable-small --disable-debug

TODO: Don't compile encoders, audio decoders and unneeded video decoders.

