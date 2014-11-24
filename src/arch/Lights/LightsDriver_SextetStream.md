`LightsDriver_SextetStream_*`
=============================

Explanation
-----------

This is a set of drivers (currently, just one driver) that encode light
states as text and stream the result to the desired output. By making
the light data available as a readable stream, light control can be
performed by another program. Such a program can be implemented using
any language/platform that supports driving the desired output device
(and reading from an input stream). If C++ isn't your thing, or if
learning the guts of StepMania seems a little much just to implement a
lights driver, you're in the right place.

Quick start
-----------

You'll need a working StepMania build with the driver
`LightsDriver_SextetStreamToFile` enabled.

For this test, you'll run a script, `SextetStreamStdinTest`, to verify
that the driver is set up properly. The script is a simple example of a
*lighting program*, a program that will receive input from StepMania and
produce output using the data. Because the script is for testing and
diagnostics, the is text in a console window. Actual lighting programs
receive input in the same way, but will do different things with the
output, such as sending commands out over a hardware interface
(serial/parallel/USB) to control physical lights.

### Linux

You'll need the perl script
[`SextetStreamStdinTest.pl`](https://gist.github.com/psmay/7f45a867c1ae8f88ec36#file-sextetstreamstdintest-pl).
Set the executable bit (`chmod ug+x SextetStreamStdinTest.pl`).

We assume that `$SM` is the root StepMania directory.

In `Preferences.ini`, set:

    LightsDriver=SextetStreamToFile
    SextetStreamOutputFilename=Data/StepMania-Lights-SextetStream.out

Create the FIFO:

    mkfifo "$SM/Data/StepMania-Lights-SextetStream.out"

Run the test stdin-based lighting program, using the FIFO as input:

    ./SextetStreamStdinTest.pl < "$SM/Data/StepMania-Lights-SextetStream.out"

While the lighting program is running, start StepMania. (When using the
test lighting program, windowed mode may be needed to keep the program
visible.) The lighting program should display the light information.

When StepMania closes, the FIFO connection ends, causing the lighting
program to exit as well.

#### Serial port under Linux

If you have a microcontroller running firmware that understands the
SextetStream protocol, you can use `socat` as the lighting program to
pipe the data directly to the device via a serial port:

    socat "$SM/Data/StepMania-Lights-SextetStream.out" /dev/ttyUSB0,raw,echo=0,b115200

Substitute your actual serial port device for `/dev/ttyUSB0` and the
actual port speed for `115200`. (But note that running at too low a
speed may result in the output blocking, causing StepMania to freeze or
stutter).

TODO: Supply example Arduino/PIC/... firmware.

TODO: Supply example of full-duplex in cooperation with the SextetStream
input driver.

### Mac OS X

FIXME: *This has not been tested at all.* The following is a guess at a
synopsis of how it should work if you're lucky enough for everything to
have fallen into place just so. (Please amend this message if you manage
to get this running consistently on Mac OS X.)

The instructions for Linux should work equally well for Mac OS X.

TODO: Serial examples.

### Windows

FIXME: *This has not been tested at all.* The following is a guess at a
synopsis of how it should work if you're lucky enough for everything to
have fallen into place just so. (Please amend this message if you manage
to get this running consistently on Windows.)

You'll need:

*   The Windows Script Host script
    [`SextetStreamStdinTest.wsf`](https://gist.github.com/psmay/7f45a867c1ae8f88ec36#file-sextetstreamstdintest-wsf),
    the lighting program
*   [`createAndReadPipe`](https://github.com/psmay/windows-named-pipe-utils/releases),
    to work with a named pipe from the console.

In `Preferences.ini`, set:

    LightsDriver=SextetStreamToFile
    SextetStreamOutputFilename=\\.\pipe\StepMania-Lights-SextetStream

Use `createAndReadPipe` to create the named pipe, feeding the output
into the test stdin-based lighting program:

    createAndReadPipe StepMania-Lights-SextetStream | cscript //nologo SextetStreamStdinTest.wsf

While the lighting program is running, start StepMania. (When using the
test lighting program, windowed mode may be needed to keep the program
visible.) The lighting program should display the light information.

When StepMania closes, the pipe connection ends, causing the lighting
program to exit as well.

TODO: Serial examples.

Encoding
--------

### Packing sextets

Values are encoded six bits at a time (hence "sextet").

Data is packed into the low 6 bits of a byte, then the two high bits are
set in such a way that the result is printable, non-whitespace ASCII:

    0x00-0x0F -> 0x40-0x4F
    0x10-0x1F -> 0x50-0x5F
    0x20-0x2F -> 0x60-0x6F
    0x30-0x3F -> 0x30-0x3F

(0x20-0x2F and 0x70-0x7F are avoided since they contain control or
whitespace characters.)

The characters are made printable so that attempting to read the output
or pass it through a text-oriented channel will not cause problems.

In any case, decoding these values is trivial and involves no
lookups—just do `& 0x3F`. (And even that may be optional, depending on
the application.)

### Bit meanings

The driver repeatedly outputs a serialization of the current light state
followed by an LF (0xA, `\n`), at least once each time any light should
change (and currently no more often than that).

Currently, this message is 13 data bytes followed by LF. The following
information is contained:

*   Byte 0
    *   0x01 Marquee upper-left
    *   0x02 Marquee upper-right
    *   0x04 Marquee lower-left
    *   0x08 Marquee lower-right
    *   0x10 Bass left
    *   0x20 Bass right

*   Byte 1
    *   0x01 Player 1 menu left
    *   0x02 Player 1 menu right
    *   0x04 Player 1 menu up
    *   0x08 Player 1 menu down
    *   0x10 Player 1 start
    *   0x20 Player 1 select

*   Byte 2
    *   0x01 Player 1 back
    *   0x02 Player 1 coin
    *   0x04 Player 1 operator
    *   0x08 Player 1 effect up
    *   0x10 Player 1 effect down
    *   0x20 (reserved)

*   Byte 3
    *   0x01 Player 1 #1
    *   0x02 Player 1 #2
    *   0x04 Player 1 #3
    *   0x08 Player 1 #4
    *   0x10 Player 1 #5
    *   0x20 Player 1 #6

*   Byte 4
    *   0x01 Player 1 #7
    *   0x02 Player 1 #8
    *   0x04 Player 1 #9
    *   0x08 Player 1 #10
    *   0x10 Player 1 #11
    *   0x20 Player 1 #12

*   Byte 5
    *   0x01 Player 1 #13
    *   0x02 Player 1 #14
    *   0x04 Player 1 #15
    *   0x08 Player 1 #16
    *   0x10 Player 1 #17
    *   0x20 Player 1 #18

*   Byte 6
    *   0x01 Player 1 #19
    *   0x02 (reserved)
    *   0x04 (reserved)
    *   0x08 (reserved)
    *   0x10 (reserved)
    *   0x20 (reserved)

*   Byte 7
    *   0x01 Player 2 menu left
    *   0x02 Player 2 menu right
    *   0x04 Player 2 menu up
    *   0x08 Player 2 menu down
    *   0x10 Player 2 start
    *   0x20 Player 2 select

*   Byte 8
    *   0x01 Player 2 back
    *   0x02 Player 2 coin
    *   0x04 Player 2 operator
    *   0x08 Player 2 effect up
    *   0x10 Player 2 effect down
    *   0x20 (reserved)

*   Byte 9
    *   0x01 Player 2 #1
    *   0x02 Player 2 #2
    *   0x04 Player 2 #3
    *   0x08 Player 2 #4
    *   0x10 Player 2 #5
    *   0x20 Player 2 #6

*   Byte 10
    *   0x01 Player 2 #7
    *   0x02 Player 2 #8
    *   0x04 Player 2 #9
    *   0x08 Player 2 #10
    *   0x10 Player 2 #11
    *   0x20 Player 2 #12

*   Byte 11
    *   0x01 Player 2 #13
    *   0x02 Player 2 #14
    *   0x04 Player 2 #15
    *   0x08 Player 2 #16
    *   0x10 Player 2 #17
    *   0x20 Player 2 #18

*   Byte 12
    *   0x01 Player 2 #19
    *   0x02 (reserved)
    *   0x04 (reserved)
    *   0x08 (reserved)
    *   0x10 (reserved)
    *   0x20 (reserved)

Above, `Player 1 #n` and `Player 2 #n` refer to StepMania's internal
`GAME_BUTTON_CUSTOM_nn` values, whose meaning depends on the game.
Currently, these mappings are:

*   dance
    *   1 Pad left
    *   2 Pad right
    *   3 Pad up
    *   4 Pad down
    *   5 Pad up-left
    *   6 Pad up-right

*   techno: Same as dance, plus
    *   7 Pad center
    *   8 Pad down-left
    *   9 Pad down-right

*   pump
    *   1 Pad up-left
    *   2 Pad up-right
    *   3 Pad center
    *   4 Pad down-left
    *   5 Pad down-right

*   kb7
    *   1-7 Keys 1-7

*   ez2
    *   1 Foot upper-left
    *   2 Foot upper-right
    *   3 Foot down
    *   4 Hand upper-left
    *   5 Hand upper-right
    *   6 Hand lower-left
    *   7 Hand lower-right

*   para
    *   1 Button left
    *   2 Button up-left
    *   3 Button up
    *   4 Button up-right
    *   5 Button right

*   ds3ddx
    *   1 Hand left
    *   2 Foot down-left
    *   3 Foot up-left
    *   4 Hand up
    *   5 Hand down
    *   6 Foot up-right
    *   7 Foot down-right
    *   8 Hand right

*   beat
    *   1-7 Keys 1-7
    *   8 Scratch up
    *   9 Scratch down

*   maniax
    *   1 Sensor over-left
    *   2 Sensor over-right
    *   3 Sensor under-left
    *   4 Sensor under-right

*   popn
    *   1 Button left white
    *   2 Button left yellow
    *   3 Button left green
    *   4 Button left blue
    *   5 Button center
    *   6 Button right blue
    *   7 Button right green
    *   8 Button right yellow
    *   9 Button right white

Filesystem
----------

The current implementation of this driver uses a `RageFile` for output.
Therefore, any `RageFile`-based filesystem abstractions *are* applied.
Please keep this in mind when specifying the input path.

FIXME: This behavior is probably permanent, but if it is not possible to
open a Windows pipe using `RageFile`, `RageFile` should probably change
so that it is.

Pipes
-----

The output of this driver streams to a file. At face value, this is not
too useful. The actual intent is for the system operator to create a
*named fifo* or *named pipe* that is being listened to by some already
running program, such as a program that sends light data out over a
serial connection. It just happens that opening a named fifo for writing
can be accomplished in the same fashion as opening a file for writing.
Since there's an easy, platform-ignorant way to do that, that's what
we've done.

Note that this uses blocking I/O, so your light processing program must
make sure to empty the fifo as quickly as it gets filled. If this
driver's write blocks, parts of the application stall. If the program is
non-trivial, using a separate thread dedicated to reading may be useful.

How you actually start up the other end of the connection depends on
your platform.

### Linux (and some other unixish systems)

`mkfifo` is used to create a named FIFO. After this is done, two
programs (one reading and one writing) open the FIFO as if it were some
ordinary file and use ordinary file I/O to read or write.

For this driver, the lighting program and StepMania open the file for
read and write, respectively.

Lighting programs accepting input on stdin work trivially; the FIFO is
simply redirected into the program's stdin.

### Windows

The client side of a Windows named pipe is fairly ordinary; the path to
an already-open pipe can be opened and used as if it were an ordinary
file. For this driver, StepMania is the client.

The server side of the pipe is a little trickier. Special Windows API
calls are needed to create the pipe and then wait for the client to
connect. Unlike in Linux, the FIFO goes away when it is closed. For this
driver, the lighting program is the server.

Lighting programs accepting input on stdin do not work trivially because
named pipes are not eligible to be created or redirected on the command
line. A bridging program such as `createAndReadPipe` can be used to
create a pipe and relay all of its input to stdout, which can then be
redirected.

A Windows-specific lighting program might also just create and read a
named pipe by itself.

License
=======

Copyright © 2014 Peter S. May

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
