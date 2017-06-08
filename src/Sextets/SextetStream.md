SextetStream
============

Description
-----------

SextetStream is a stream encoding for finite- but arbitrary-length
vectors of boolean values.

SextetStream-encoded data is made up of characters, not strictly bytes,
and is suited for use by platforms that prefer to treat strings as text
instead of binary data.

Motivation
----------

SextetStream has been developed as a way to make it possible for
hobbyists, scripters, and programmers who can't muster the patience for
C++ to develop their own InputHandler and LightsDriver machinery without
needing to set up the entire build system for StepMania. Instead,
InputHandler and LightsDriver are boiled down to an extremely simple
text stream protocol over stdio, named pipes, and/or TCP sockets.

Refer to `src/arch/InputHandler/InputHandler_SextetStream.md` and
`src/arch/Lights/LightsDriver_SextetStream.md` for more information
about those drivers and how to use them.

Stream syntax
-------------

    SEXTET : 0x30 .. 0x6F               # ASCII '0' through 'o'
    NEWLINE : 0x0D 0x0A | 0x0A | 0x0D   # ASCII CRLF, CR, or LF
    INVALID : ~(SEXTET | NEWLINE)       # Everything except
                                        # SEXTET or NEWLINE

    entireTransmission : (packet | invalidLine)*

    packet : noopPacket | normalPacket

    noopPacket : NEWLINE

    normalPacket : SEXTET+ NEWLINE

    invalidLine : SEXTET* INVALID (SEXTET | INVALID)* NEWLINE

Packets are constructed of sequences of sextet characters followed by a
newline.

### Noop packet

A noop packet (or blank packet, or empty packet; the terminology needs
alignment) is a packet that consists of no sextets followed by a newline
character.

When a noop packet is received, the receiver is not to change its state.

These might be inserted among other data into the stream by the sender
to test the status of the connection, encourage the receiver to recheck
its loop condition, and so forth.

They also appear when the receiver treats a CRLF as two separate
characters. This is intentionally harmless.

### Normal packet

A normal packet is a packet that consists of one or more sextets
followed by a newline character.

When a normal packet is received, the receiver is to update its state to
reflect the new data.

The packet must be at least one sextet long (otherwise, it is read as a
noop packet). Otherwise, the packet data can be as short as desired as
long as all of its true values are expressed; a packet extends
infinitely to the right with false values.

For example, the following packets express the same value:

    @@@@A@B@C@@@@@@@@@@@@
    @@@@A@B@C@@@@@@
    @@@@A@B@C

#### Encoding sextets

A sextet is encoded by starting with a six-bit integer and then
selecting the seventh bit so that the result falls in the range
`0x30 .. 0x6F`:

    encoded = ((raw + 0x10) & 0x3F) + 0x30

(This range was selected in preference to `0x20 .. 0x5F` and
`0x40 .. 0x7F` due to the former containing a whitespace character and
the latter containing a control character.)

#### Decoding sextets

An encoded sextet actually contains the original six bits, so for most
purposes it is enough just to ignore the bits above the low six. If it
is necessary to clear those bits, simply use AND:

    decoded = encoded & 0x3F

### Invalid input

When input that would normally be a packet contains invalid (non-sextet,
non-newline) data, it should be discarded all the way through the next
newline. The processor may resume waiting for valid packets or, at its
discretion, do something else.

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
