# StepMania

[![Website](https://img.shields.io/website/https/www.stepmania.com?down_color=lightgrey&down_message=offline&up_color=blue&up_message=online)](https://www.stepmania.com)
[![Travis (.org)](https://img.shields.io/travis/stepmania/stepmania?label=Travis%20CI&logo=travis)](https://travis-ci.org/stepmania/stepmania)
[![AppVeyor](https://img.shields.io/appveyor/ci/Nickito12/stepmania?label=AppVeyor&logo=appveyor)](https://ci.appveyor.com/project/Nickito12/stepmania)

StepMania is an advanced cross-platform rhythm game for home and arcade use.

Advanced cross-platform rhythm game for home and arcade use.

## Installing binaries

For those that do not wish to compile the game on their own and use a binary right away, be aware of the following issues:

* Windows users are expected to have installed the [Microsoft Visual C++ x86 Redistributable for Visual Studio 2015](https://www.microsoft.com/en-us/download/details.aspx?id=48145) prior to running the game. For those on a 64-bit operating system, grab the x64 redistributable as well. [DirectX End-User Runtimes (June 2010)](https://www.microsoft.com/en-us/download/details.aspx?id=8109) is also required. Windows 7 is the minimum supported version.
* Mac OS X users need to have Mac OS X 10.6.8 or higher to run StepMania.
* Linux users should receive all they need from the package manager of their choice.

## Building

StepMania can be compiled using [CMake](https://www.cmake.org/). More information about using CMake can be found in both the `Build` directory and CMake's documentation.

## Resources

* [www.stepmania.com](https://www.stepmania.com/)
* irc.freenode.net/#stepmania-devs
* [dguzek/Lua-For-SM5](https://dguzek.github.io/Lua-For-SM5/)
* [Lua API Documentation](Docs/Luadoc)

## Licensing Terms

In short- you can do anything you like with the game (including sell products made with it), provided you *do not*:

1. Sell the game *with the included songs*
2. Claim to have created the engine yourself or remove the credits
3. Not provide source code for any build which differs from any official release which includes MP3 support.

(It's not required, but we would also appreciate it if you link back to https://www.stepmania.com/)

For specific information/legalese:

* All of our source code is under the [MIT license](https://opensource.org/licenses/MIT).
* Any songs that are included within this repository are under the [<abbr title="Creative Commons Non-Commercial">CC-NC</abbr> license](https://creativecommons.org/).
* The [MAD library](https://www.underbit.com/products/mad/) and [FFmpeg codecs](https://www.ffmpeg.org/) when built with our code use the [GPL license](https://www.gnu.org).
