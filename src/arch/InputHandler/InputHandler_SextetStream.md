`InputHandler_SextetStream_*`
=============================

Explanation
-----------

Each of this set of drivers accepts button inputs encoded as
SextetStream packets (see `src/Sextets/SextetStream.md` for general
info) from some other process over an input stream.

Available drivers
-----------------

There are currently three available SextetStream input drivers:

*   `SextetStreamFromSelectFile` uses POSIX-based non-blocking I/O to
    access a file, which is expected to be a named pipe/FIFO.
    *   This driver is not available on Windows.
    *   This driver uses non-blocking reads.
    *   The filename to use is set using the `SextetStreamInputFilename`
        setting in preferences.
*   `SextetStreamFromFile` uses C stdio (e.g. `fopen()`) to access a
    file, which is expected to be a named pipe/FIFO.
    *   Prefer `SextetStreamFromSelectFile` if it is supported by your
        system.
    *   If you use Windows and you trust the other users on your
        machine, prefer `SextetStreamFromSocket`.
    *   Seriously, a non-blocking named pipe driver for Windows is on
        the to-do list...
    *   This driver uses blocking reads which prevent a reading thread
        from determining whether or not it should continue to run. It
        should be used primarily as a fallback. To prevent hangs/errors
        in StepMania, software providing the packets through this driver
        must output some data (either repeat the most recent packet or
        merely send a blank line) at least once per second or so in
        order to allow the condition to be checked. Alternatively, the
        software providing packets must close the stream at its end
        before the user attempts to exit StepMania.
    *   The filename to use is set using the `SextetStreamInputFilename`
        setting in preferences (same as `SextetStreamFromSelectFile`).
*   `SextetStreamFromSocket` uses a TCP client to open a connection on a
    port where the input program is listening.
    *   This driver uses non-blocking reads.
    *   The host and port to use are set using the
        `SextetStreamInputSocketHost` and `SextetStreamInputSocketPort`,
        respectively.

Quick start
-----------

You'll need a working StepMania build with the driver you want to use
(`InputHandler_SextetStream_*`) enabled.

For this test, you'll run a test input program to verify that the driver
is set up properly. This is a simple example of an *input program*, a
program that will receive input from some arbitrary input source, encode
the button states, and produce output to be read by StepMania. Normally,
an input program reads input from some external source, such as on a
hardware (serial/parallel/USB) interface. Because this program is for
testing and diagnostics, the input will instead come from keypresses on
a GUI window.

### Keypress limitations (that are not part of SextetStream)

Due to the design of some keyboards and/or their drivers, the number of
keys that can be held at one time, which keys can be pressed
simultaneously, and which keys have priority over others may vary. The
SextetStream encoding and drivers impose no such limitation and can
process all buttons independently of each other if the input program is
capable of providing such information.

### Set up `SextetInputTest.jar`

Get a copy of the test input program,
[`SextetInputTest.jar`](https://github.com/psmay/SextetInputTest/releases),
which can be used to test the pipe-based and socket-based drivers. A
working Java VM (version 7 or later) is also needed.

To ensure that the current environment is correct, simply run the
program:

    java -jar SextetInputTest.jar

A GUI window should open. Click on this window and then try pressing
some keys. When any of these keys is pressed or released, the displayed
"State" packet changes on the window.

The output of this program on your console is sextet packets
representing the state of the pressed keys. A new output line is
produced immediately when a button is either pressed or released. A noop
packet (blank output line) is also produced periodically (about once per
second) if there have been no recent changes. (The noop packets serve to
ensure the stream is still open and allow the driver, if it uses
blocking reads, to check its loop variables.)

Press and hold the up arrow. The output should be the following line, or
something similar, followed by noop packets if held long enough:

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

When you release the key, the output should be exactly the following
line, followed by noop packets if left alone long enough:

    @

### Linux (and, in principle, Mac OS X and anything unixish)

We assume that `SextetInputTest.jar` is working (as outlined above) and
that `$SM` is the root StepMania directory.

#### Using a FIFO

In `Preferences.ini`, set:

    InputDrivers=X11,SextetStreamFromSelectFile
    SextetStreamInputFilename=Data/StepMania-Input-SextetStream.in

(This also keeps the default keyboard input enabled. The `X11` part can
be removed later after setting up input mappings, if desired. For Mac OS
X, replace `X11` with `HID`.)

Create the FIFO:

    mkfifo "$SM/Data/StepMania-Input-SextetStream.in"

Run the test input program using the FIFO as output:

    java -jar SextetInputTest.jar > "$SM/Data/StepMania-Input-SextetStream.in"

While the input program is running, start StepMania. (While using the
test program, use windowed mode to keep both StepMania and the input
program visible.) Using the keyboard directly on the StepMania window,
go to Options, Test Input. Switch to the test input program and try
pressing some keys. If StepMania displays corresponding messages, the
driver is working properly.

#### Using a TCP socket

In `Preferences.ini`, set:

    InputDrivers=X11,SextetStreamFromSocket
    SextetStreamInputSocketHost=localhost
    SextetStreamInputSocketPort=6761

(This also keeps X11-based keyboard input enabled. The X11 part can be
removed later after setting up input mappings, if desired.)

Run the test input program as a TCP server:

    java -jar SextetInputTest.jar host=localhost port=6761

Continue as above by starting StepMania.

### Windows

**This has not been tested.** It's just a guess of how it should work
once everything is in place.

We assume that `SextetInputTest.jar` is working (as outlined above).

#### Using a named pipe

You will need
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

    java -jar SextetInputTest.jar | createAndWritePipe StepMania-Input-SextetStream

While the input program is running, start StepMania. (While using the
test program, use windowed mode to keep both StepMania and the input
program visible.) Using the keyboard directly on the StepMania window,
go to Options, Test Input. Switch to the test input program and try
pressing some keys. If StepMania displays corresponding messages, the
driver is working properly.

#### Using a TCP socket

In `Preferences.ini`, set:

    InputDrivers=DirectInput,SextetStreamFromSocket
    SextetStreamInputSocketHost=localhost
    SextetStreamInputSocketPort=6761

(This also keeps the default DirectInput-based input enabled. The
DirectInput part can be removed later after setting up input mappings,
if desired.)

Run the test input program as a TCP server:

    java -jar SextetInputTest.jar host=localhost port=6761

Continue as above by starting StepMania.

Data map
--------

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

The number of buttons supported by the implementation is currently the
number of supported joy buttons plus the number of supported unknown
keys (as of this writing: 32 + 321 = 353; you certainly shouldn't need
this many). Overlong input lines are allowed and truncated. Because the
number of bytes needed in an input line is proportional to the index of
the highest-indexed bit currently in an on state, it's probably a good
idea to keep that number low whenever possible, especially when using a
low-speed input (such as a serial or network connection).

Platform notes
--------------

### Blocking behavior

The `SextetStreamFromFile` driver uses blocking I/O. It does so in a
separate thread so that StepMania will read lines about as fast as they
are written and there is no technical minimum data rate; however, the
blocking read cannot be interrupted except by the other side of the
connection either sending new data or closing. If neither happens
promptly as StepMania closes, it will hang.

For the purposes of such blocking-read-based, it is good manners for an
input program to periodically (e.g. once per second) send a noop packet
so that the input thread is never left blocking for too long at a time.
(Alternatively, if the input program has some way to determine that
StepMania is exiting, it may just close the stream instead.)

The `SextetStreamFromSelectFile` driver is a non-blocking drop-in
replacement for `SextetStreamFromFile` for non-Windows systems, even
using the same filename setting. On Windows, `SextetStreamFromSocket`
provides non-blocking functionality. (Non-blocking named pipe read on
Windows is not impossible, but not yet a priority. Maybe you'd like to
take it on?)

### Linux (and other unixish systems) FIFOs

`mkfifo` is used to create a named FIFO. After this is done, two
programs (one reading and one writing) open the FIFO as if it were some
ordinary file and use ordinary file I/O to read or write.

For this driver, the input program and StepMania open the file for write
and read, respectively.

Input programs producing output on stdout work trivially; the program's
output is simply redirected to the FIFO.

### Windows named pipes

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

### Sockets

Common sockets functionality is found in the EzSockets library. The
socket-based driver is tested in Linux and should work in Windows. For
this driver, the input program is the server and StepMania is the
client. The input program will need to be waiting for a client before
starting StepMania.

To-do
-----

*   Put the connection lifetime code in a large loop so that a
    disconnect is followed at some point by a retry.
*   Get this driver into non-Linux builds.
*   Implement a workalike for `SextetStreamFromSelectFile` in Windows.
*   Make more example input programs
    *   e.g. Bidirectional bridge to Arduino-based controller *and*
        lights

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
