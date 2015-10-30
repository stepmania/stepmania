StepMania
=========

StepMania is an advanced cross-platform rhythm game for home and arcade use.

Advanced cross-platform rhythm game for home and arcade use.

##Installing from Setup/Installer##

For those that do not wish to compile the game on their own and use a binary right away, be aware of the following issues:

* Windows users are expected to have installed the [Microsoft Visual C++ x86 Redistributable for Visual Studio 2015](http://www.microsoft.com/en-us/download/details.aspx?id=48145) prior to running the game. For those on a 64-bit operating system, grab the x64 redistributable as well. Windows 7 is the minimum supported version.
* Mac OS X users need to have Mac OS X 10.6.8 or higher to run StepMania.
* Linux users should receive all they need from the package manager of their choice.

##Installing from Source##

StepMania can be compiled using [CMake](http://www.cmake.org/). More information about using CMake can be found in both the `Build` directory and CMake's documentation.

##Build Status##

We currently have two integration servers in place. Their statuses are listed below.

* Travis (Linux): [![Build Status](https://travis-ci.org/stepmania/stepmania.svg?branch=master)](https://travis-ci.org/stepmania/stepmania)
* AppVeyor (Windows): [![Build status](https://ci.appveyor.com/api/projects/status/e932dk2o3anki27p?svg=true)](https://ci.appveyor.com/project/wolfman2000/stepmania-wm87c)

##Resources##

* Website: http://www.stepmania.com/
* IRC: irc.freenode.net/#stepmania-devs
* Theming Wiki: http://goo.gl/SO7W5
* Lua Documentation for Themes: http://goo.gl/XNiov

##Licensing Terms##

* All of the our source code is under the [MIT license](http://opensource.org/licenses/MIT).
* Any songs that are included within this repository are under the [<abbr title="Creative Commons Non-Commercial">CC-NC</abbr> license](https://creativecommons.org/).
* The [MAD library](http://www.underbit.com/products/mad/) and [FFmpeg codecs](https://www.ffmpeg.org/) when built with our code use the [GPL license](http://www.gnu.org).
