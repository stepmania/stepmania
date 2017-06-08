`LightsDriver_SextetStream_*`
=============================

Explanation
-----------

Each of this set of drivers produces light outputs encoded as
SextetStream packets (see `src/Sextets/SextetStream.md` for general
info) to some other process over an output stream.

Available drivers
-----------------

There is currently only one available SextetStream light driver:

*   `SextetStreamToFile` uses a `RageFile` opened for output to access a
    file, which is expected to be a named pipe/FIFO.
    *   This driver opens a `RageFile` with the `RageFile::WRITE` and
        `RageFile::STREAMED` flags.
    *   Writes by this driver may block if the lighting program fails to
        read at a sufficient rate.
        *   If the lighting program leaves a pending packet unread as
            StepMania closes down, it may hang until the lighting
            program completes the read, closes the stream, or exits.
        *   Unless the processing of a packet is seriously trivial
            timewise, it may be a good idea to implement the lighting
            program in terms of a thread that waits for a packet, reads
            a packet, quickly hands off the packet for processing by
            another thread, then repeats without waiting for the packet
            to process.

Quick start
-----------

You'll need a working StepMania build with the driver
`LightsDriver_SextetStreamToFile` enabled.

For this test, you'll run a test lighting program to verify that the
driver is set up properly. This is a simple example of a *lighting
program*, a program that will receive input from StepMania and produce
output via some arbitrary output device using the data. Normally, a
lighting program produces output on some external display, such as on a
hardware (serial/parallel/USB) interface. Because this program is for
testing and diagnostics, the output will instead be displayed as a
simulated lighting display on a GUI window.

### Set up `SextetOutputTest.jar`

Get a copy of the test lighting program,
[`SextetOutputTest.jar`](https://github.com/psmay/SextetOutputTest/releases),
which can be used to test the pipe-based driver. A working Java VM
(version 7 or later) is also needed. (Note that, as of this writing,
this test program only supports the `dance` and `techno` game modes.)

To ensure that the current environment is correct, simply run the
program:

    java -jar SextetOutputTest.jar

A GUI window should open with the simulated display, with all lights
off. Return to the console window and enter a row of `?` characters (13
or more for this example), then press Enter.

    ?????????????

If this succeeds, all of the lights should now be on. To turn them back
off, type a row of `@`, then press Enter.

    @@@@@@@@@@@@@

The input to this program on your console is sextet packets representing
the states of the output lights. (A `?` character lights all of the bits
in a given sextet; a `@` turns them back off.) The lighting program
should be prepared to accept an empty line as a noop packet.

### Linux (and, in principle, Mac OS X and anything unixish)

We assume that `SextetOutputTest.jar` is working (as outlined above) and
that `$SM` is the root StepMania directory.

#### Using a FIFO

In `Preferences.ini`, set:

    LightsDriver=SextetStreamToFile
    SextetStreamOutputFilename=Data/StepMania-Lights-SextetStream.out

Create the FIFO:

    mkfifo "$SM/Data/StepMania-Lights-SextetStream.out"

Run the test stdin-based lighting program, using the FIFO as input:

    java -jar SextetOutputTest.jar < "$SM/Data/StepMania-Lights-SextetStream.out"

While the lighting program is running, start StepMania. (While using the
test program, use windowed mode to keep both StepMania and the lighting
program visible.) When successfully configured, the test program
displays animated light patterns as StepMania runs, and while under
Options, Test Input, presses and releases on the dance pad buttons are
echoed as lights.

When StepMania closes, the FIFO connection's closing may or may not
cause the lighting program to exit. If not, the lighting program can be
exited manually.

### Windows

FIXME: *This has not been tested at all.* It's just a guess of how it
should work once everything is implemented.

We assume that `SextetOutputTest.jar` is working (as outlined above).

#### Using a named pipe

You will need
[`createAndReadPipe`](https://github.com/psmay/windows-named-pipe-utils/releases),
to be able to work with a named pipe from the console.

In `Preferences.ini`, set:

    LightsDriver=SextetStreamToFile
    SextetStreamOutputFilename=\\.\pipe\StepMania-Lights-SextetStream

Use `createAndReadPipe` to create the named pipe, feeding the output
into the test stdin-based lighting program:

    createAndReadPipe StepMania-Lights-SextetStream | java -jar SextetOutputTest.jar

While the lighting program is running, start StepMania. (While using the
test program, use windowed mode to keep both StepMania and the lighting
program visible.) When successfully configured, the test program
displays animated light patterns as StepMania runs, and while under
Options, Test Input, presses and releases on the dance pad buttons are
echoed as lights.

When StepMania closes, the named pipe connection's closing may or may
not cause the lighting program to exit. If not, the lighting program can
be exited manually.

Data map
--------

The driver outputs a packet at least once each time any light changes.
Each packet represents the entire current light state.

The following information is encoded in each packet:

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
*   techno: Same as dance, and also
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

Platform notes
--------------

### RageFile

The current implementation of this driver uses a `RageFile` for output.
Therefore, any `RageFile`-based filesystem abstractions *are* applied.
Please keep this in mind when specifying the input path.

FIXME: This behavior is probably permanent, but if it is not possible to
open a Windows pipe using `RageFile`, `RageFile` should probably change
so that it is.

### Linux (and some other unixish systems) FIFOs

`mkfifo` is used to create a named FIFO. After this is done, two
programs (one reading and one writing) open the FIFO as if it were some
ordinary file and use ordinary file I/O to read or write.

For this driver, the lighting program and StepMania open the file for
read and write, respectively.

Lighting programs accepting input on stdin work trivially; the FIFO is
simply redirected into the program's stdin.

### Windows named pipes

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

To-do
-----

*   Put the connection lifetime code in a large loop so that a
    disconnect is followed at some point by a retry.
*   Get this driver into non-Linux builds.
*   `SextetStreamToFile` driver should be implemented to discard any
    packets it is not able to write.
*   Provide more example lighting programs
    *   e.g. Bidirectional bridge to Arduino-based controller *and*
        lights
*   Create additional drivers
    *   A TCP driver (`SextetStreamToSocket`) is needed.
        *   An initial implementation based on EzSockets was withdrawn
            because packets were being buffered, negating any utility,
            and I wasn't able to convince the socket to flush instead of
            buffering.
    *   A POSIX select()-based non-blocking I/O driver
        (`SextetStreamToSelectFile`) is needed.
        *   A workalike to this is also needed for Windows.

License
=======

Copyright © 2014-2016 Peter S. May

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
