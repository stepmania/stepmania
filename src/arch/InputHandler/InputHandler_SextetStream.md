`InputHandler_SextetStream_*`
=============================

Explanation
-----------

This is a set of drivers (currently, just one driver) that accept button
inputs encoded in a stream of character. By accepting this data from a
stream, input can be produced by a separate program. Such a program can
be implemented using any language/platform that supports reading from
the desired input device (and writing to an output stream). If C++ isn't
your thing, or if learning the guts of StepMania seems a little much
just to implement an input driver, you're in the right place.

Quick start
-----------

You'll need a working StepMania build with the driver
`InputHandler_SextetStreamFromFile` enabled.

For this test, you'll run a test input program to verify that the driver
is set up properly. This is a simple example of an *input program*, a
program that will receive input from some arbitrary input source, encode
the button states, and produce output to be read by StepMania. Because
this script is for testing and diagnostics, input is accepted as
keypresses on a GUI window. Actual input programs produce output in the
same way, but will read input from different sources, such as data from
a hardware interface (serial/parallel/USB).

### Set up `SextetStreamStdoutTest.jar`

You'll need the test input program,
[`SextetStreamStdoutTest.jar`](https://github.com/psmay/SextetStreamStdoutTest/releases),
as well as a working Java VM to run it. To ensure that the current
environment is correct, simply run the program:

    java -jar SextetStreamStdoutTest.jar

A GUI window should open. Click on this window and then try pressing
some keys. When one of these keys is pressed, its code (number) appears;
when released, the code disappears.

The output of this program on your console is the encoded sextets
representing the state of the pressed keys. A new output line is
produced immediately when a button is either pressed or released. An
output line is also produced periodically (about once per second) if
there have been no recent changes so that the driver is never left
blocking for too long an interval.

Each output line is only as long as necessary to express the current
state. While nothing is pressed, the output should be this line, about
once per second:

    @

Press and hold the up arrow. The output should be this line, again about
once per second:

    @@@@@@D

If instead you get a rapidly scrolling alternation like

    @@@@@@D
    @
    @@@@@@D
    @
    @@@@@@D
    @
    @@@@@@D
    @

you should use your OS or window system settings to disable key repeat,
at least for the duration of the test, to avoid an undesired rapid-fire
effect.

### Linux

We assume that `SextetStreamStdoutTest.jar` is working (as outlined
above) and that `$SM` is the root StepMania directory.

In `Preferences.ini`, set:

    InputDrivers=X11,SextetStreamFromFile
    SextetStreamInputFilename=Data/StepMania-Input-SextetStream.in

(This also keeps X11-based keyboard input enabled. The X11 part can be
removed later after setting up input mappings, if desired.)

Create the FIFO:

    mkfifo "$SM/Data/StepMania-Input-SextetStream.in"

Run the test input program using the FIFO as output:

    java -jar SextetStreamStdoutTest.jar > "$SM/Data/StepMania-Input-SextetStream.in"

While the input program is running, start StepMania. (While using the
test program, use windowed mode to keep both StepMania and the input
program visible.) Using the keyboard directly on the StepMania window,
go to Options, Test Input. Switch to the test input program and try
pressing some keys. If StepMania displays corresponding messages, the
driver is working properly.

#### Serial port under Linux

FIXME: *This has not been tested at all.* The following is a guess at a
synopsis of how it should work if you're lucky enough for everything to
have fallen into place just so. (Please amend this message if you manage
to get this running consistently.)

If you have a microcontroller running firmware that produces output
compatible with the SextetStream protocol, you can use `socat` as the
input program to pipe the data directly from the device via a serial
port:

    socat /dev/ttyUSB0,raw,echo=0,b115200 "$SM/Data/StepMania-Input-SextetStream.in"

Substitute your actual serial port device for `/dev/ttyUSB0` and the
actual port speed for `115200`. (A low speed will not freeze StepMania,
but may introduce an unacceptable input latency.)

TODO: Supply example Arduino/PIC/... firmware.

TODO: Supply example of full-duplex in cooperation with the SextetStream
lights driver.

### Mac OS X

FIXME: *This has not been tested at all.* The following is a guess at a
synopsis of how it should work if you're lucky enough for everything to
have fallen into place just so. (Please amend this message if you manage
to get this running consistently on Mac OS X.)

Follow the instructions for Linux, but change the `InputDrivers` setting
to:

    InputDrivers=HID,SextetStreamFromFile

(This also keeps the default HID-based input enabled. The HID part can
be removed later after setting up input mappings, if desired.)

TODO: Serial examples.

### Windows

FIXME: *This has not been tested at all.* The following is a guess at a
synopsis of how it should work if you're lucky enough for everything to
have fallen into place just so. (Please amend this message if you manage
to get this running consistently on Windows.)

We assume that `SextetStreamStdoutTest.jar` is working (as outlined
above). You will also need
[`createAndWritePipe`](https://github.com/psmay/windows-named-pipe-utils/releases)
to be able to work with a named pipe from the console.

In `Preferences.ini`, set:

    InputDrivers=DirectInput,SextetStreamFromFile
    SextetStreamInputFilename=\\.\pipe\StepMania-Input-SextetStream

(This also keeps the default DirectInput-based input enabled. The
DirectInput part can be removed later after setting up input mappings,
if desired.)

Run the input program and use `createAndWritePipe` to redirect the
output into the named pipe:

    java -jar SextetStreamStdoutTest.jar | createAndWritePipe StepMania-Input-SextetStream

While the input program is running, start StepMania. (While using the
test program, use windowed mode to keep both StepMania and the input
program visible.) Using the keyboard directly on the StepMania window,
go to Options, Test Input. Switch to the test input program and try
pressing some keys. If StepMania displays corresponding messages, the
driver is working properly.

TODO: Serial examples.

Keyboard-based demo limitations
-------------------------------

Due to the design of some keyboards and/or their drivers, the number of
keys that can be held at one time, which keys can be pressed
simultaneously, and which keys have priority over others may vary. The
SextetStream driver itself has no such limitation and can process all
buttons independently of each other if the input program is capable of
providing such information.

Encoding
--------

### Packing sextets

Values are encoded six bits at a time (hence "sextet"). The characters
are made printable, non-whitespace ASCII so that attempting to read the
data or pass it through a text-oriented channel will not cause problems.
The encoding is similar in concept to base64 or uuencode, but in this
scheme the low 6 bits remain unchanged.

Data is packed into the low 6 bits of a byte, then the two high bits are
set in such a way that the result is printable, non-whitespace ASCII:

    0x00-0x0F -> 0x40-0x4F
    0x10-0x1F -> 0x50-0x5F
    0x20-0x2F -> 0x60-0x6F
    0x30-0x3F -> 0x30-0x3F

(0x20-0x2F and 0x70-0x7F are avoided since they contain control or
whitespace characters.)

To encode a 6-bit value `s` in this way, this transform may be used:

    ((s + 0x10) & 0x3F) + 0x30

### Bit meanings

This driver produces events for a virtual game controller with no axes
and an arbitrary number of buttons. As with an actual gamepad, the
in-game meaning of each button can be configured in StepMania.

Note that the maximum number of gamepad buttons supported by StepMania
is currently 32. Higher codes are coded as keypresses for unknown keys
(which is unusual for a game controller, but works here).

*   Byte 0
    *   0x01 button B1
    *   0x02 button B2
    *   0x04 button B3
    *   0x08 button B4
    *   0x10 button B5
    *   0x20 button B6

*   …

*   Byte `n` for `n < 5`
    *   0x01 button B`6n+1`
    *   0x02 button B`6n+2`
    *   0x04 button B`6n+3`
    *   0x08 button B`6n+4`
    *   0x10 button B`6n+5`
    *   0x20 button B`6n+6`

*   …

*   Byte 5
    *   0x01 button B31
    *   0x02 button B32
    *   0x04 key unk 0
    *   0x08 key unk 1
    *   0x10 key unk 2
    *   0x20 key unk 3

*   …

*   Byte `n` for `n > 5`
    *   0x01 key unk `6(n-6)+4`
    *   0x02 key unk `6(n-6)+5`
    *   0x04 key unk `6(n-6)+6`
    *   0x08 key unk `6(n-6)+7`
    *   0x10 key unk `6(n-6)+8`
    *   0x20 key unk `6(n-6)+9`

A message is ended by terminating it with LF (0x0A) or CR LF (0x0D
0x0A). Data bytes outside 0x30-0x6F are discarded. The message can be as
large or as short as necessary, to encode all buttons. Any buttons not
encoded in a message are understood to have the value 0.

These messages both indicate that buttons 4 and 6 are pressed, and all
others are not:

    # Note: 0x68 & 0x3F == 0x28; 0x40 & 0x3F == 0x00
    0x68 0x40 0x40 0x0A
    0x68 0x0A

The number of buttons supported by the implementation is currently the
number of supported joy buttons plus the number of supported unknown
keys (as of this writing: 32 + 321 = 353; you certainly shouldn't need
this many). Overlong input lines are allowed and truncated. Because the
number of bytes needed in an input line is proportional to the index of
the highest-indexed bit currently in an on state, it's probably a good
idea to keep that number low whenever possible, especially when using a
low-speed input (such as a serial or network connection).

Filesystem
----------

The current implementation of this driver uses C standard I/O (i.e.,
`fopen()`, `fread()`, etc.) instead of the `RageFile` abstraction
Therefore, any `RageFile`-based filesystem abstractions *are not*
applied. Please keep this in mind when specifying the input path.

FIXME: This behavior is definitely subject to change should the
following problem ever get worked out: I was unable to get `RageFile` to
work in this context; I believe the cause is that I couldn't get a
`RageFile` object to do unbuffered input. It may have been something
else.

Pipes
-----

The input of this driver is streamed from a file. At face value, this is
not too useful. The actual intent is for the system operator to create a
*named fifo* or *named pipe* that is being written to by some already
running program, such as a program that reads button state data in over
a serial connection. It just happens that opening a named fifo for
reading can be accomplished in the same fashion as opening a file for
reading, as long as buffering can be disabled. Since there's an easy,
platform-ignorant way to do that, that's what we've done.

### Blocking behavior

This driver uses blocking I/O, but does so in a separate thread so that
StepMania will read lines about as fast as they are written and there is
no technical minimum data rate. However, the blocking read temporarily

particular, as StepMania exits, it may hang waiting for the input thread
to finish until one of the following happens:

*   The input program closes the stream
*   The input program outputs a line, allowing StepMania to end the
    input thread

So, for the purposes of this driver, it is good manners for an input
program to periodically (e.g. once per second) repeat its current state
so that the input thread is never left blocking for too long at a time.
(Alternatively, if the input program has some way to determine that
StepMania is exiting, it may just close the stream instead.)

### Linux (and some other unixish systems)

`mkfifo` is used to create a named FIFO. After this is done, two
programs (one reading and one writing) open the FIFO as if it were some
ordinary file and use ordinary file I/O to read or write.

For this driver, the input program and StepMania open the file for write
and read, respectively.

Input programs producing output on stdout work trivially; the program's
output is simply redirected to the FIFO.

### Windows

The client side of a Windows named pipe is fairly ordinary; the path to
an already-open pipe can be opened and used as if it were an ordinary
file. For this driver, StepMania is the client.

The server side of the pipe is a little trickier. Special Windows API
calls are needed to create the pipe and then wait for the client to
connect. Unlike in Linux, the FIFO goes away when it is closed. For this
driver, the input program is the server.

Input programs producing output on stdout do not work trivially because
named pipes are not eligible to be created or redirected to on the
command line. Such a program can be redirected to a bridging program
such as `createAndWritePipe`, which creates a pipe and then relays
whatever is received on stdin to the pipe.

A Windows-specific input program might also just create and write a
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
