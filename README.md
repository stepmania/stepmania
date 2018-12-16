StepMania
=========

StepMania is an advanced cross-platform rhythm game for home and arcade use.

Advanced cross-platform rhythm game for home and arcade use.

[![Build Status](https://travis-ci.org/stepmania/stepmania.svg?branch=master)](https://travis-ci.org/stepmania/stepmania)
[![Build status](https://ci.appveyor.com/api/projects/status/e932dk2o3anki27p?svg=true)](https://ci.appveyor.com/project/wolfman2000/stepmania-wm87c)

## Installation
### From Packages

For those that do not wish to compile the game on their own and use a binary right away, be aware of the following issues:

* Windows users are expected to have installed the [Microsoft Visual C++ x86 Redistributable for Visual Studio 2015](https://www.microsoft.com/en-us/download/details.aspx?id=48145) prior to running the game. For those on a 64-bit operating system, grab the x64 redistributable as well. [DirectX End-User Runtimes (June 2010)](https://www.microsoft.com/en-us/download/details.aspx?id=8109) is also required. Windows 7 is the minimum supported version.
* Mac OS X users need to have Mac OS X 10.6.8 or higher to run StepMania.
* Linux users should receive all they need from the package manager of their choice.

### From Source

StepMania can be compiled using [CMake](https://cmake.org/). More information about using CMake can be found in both the `Build` directory and CMake's documentation.

### Submodules ###

This repository now uses submodules to attempt to keep the repository size down. Utilize `git submodule init` and `git submodule update` to get the necessary submodules.

## Resources

* Website: https://www.stepmania.com/
* IRC: #stepmania-devs on Freenode (chat.freenode.net), or [webchat client](https://webchat.freenode.net/?channels=%23stepmania-devs&uio=d4)
* Lua for SM5: https://dguzek.github.io/Lua-For-SM5/
* Lua API Documentation can be found in the Docs folder.

## Licensing Terms

In short- you can do anything you like with the game (including sell products made with it), provided you *do not*:

1. Sell the game *with the included songs*
2. Claim to have created the engine yourself or remove the credits
3. Not provide source code for any build which differs from any official release which includes MP3 support.

(It's not required, but we would also appreciate it if you link back to https://www.stepmania.com/)

For specific information/legalese:

* All of the our source code is under the [MIT license](https://opensource.org/licenses/MIT).
* Any songs that are included within this repository are under the [<abbr title="Creative Commons Non-Commercial">CC-NC</abbr> license](https://creativecommons.org/).
* The [MAD library](https://www.underbit.com/products/mad/) and [FFmpeg codecs](https://www.ffmpeg.org/) when built with our code use the [GPL license](https://www.gnu.org).
