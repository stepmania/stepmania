Warning
==

Using CMake is considered stable, but not every single combination is known to work.
Using the defaults as suggested should cause minimal problems.

Install CMake
==

At first, you have to install CMake.

All OSes
===

The common way of installing CMake is to go to [CMake's download page](http://www.cmake.org/download/). At this time of writing, the latest versions are 3.3.0-rc3 and 3.2.3. Either version will work: the minimum version supported at this time is 2.8.12.

If this approach is used, consider using the binary distributions. Most should also provide a friendly GUI interface.

Windows
===

For those that prefer package manager systems, [Chocolatey](https://chocolatey.org/) has a CMake package. Run `choco install cmake` to get the latest stable version.

Mac OS X Specific
===

For those that prefer package manager systems, both [Homebrew](http://brew.sh/) and [MacPorts](https://www.macports.org/) offer CMake as part of their offerings. Run `brew install cmake` or `port install cmake` respectively to get the latest stable version.

Linux
===

There are many package managers available for Linux. Look at your manual for more details. Either that, or utilize the All OS specific approach.


CMake Installation
==

There are two ways of working with cmake: the command line and the GUI.

CMake Command Line
===

If you are unfamiliar with cmake, first run `cmake --help`. This will present a list of options and generators.
The generators are used for setting up your project.

The following steps will assume you operate from the StepMania project's Build directory.

For the first setup, you will want to run this command:

`cmake -G {YourGeneratorHere} .. && cmake ..`

Replace {YourGeneratorHere} with one of the generator choices from `cmake --help`. As an example, Mac OS X users that want to have Xcode used would run `cmake -G Xcode .. && cmake ..` on their Terminal program.

If you are building on Windows and expecting your final executable to be able to run on Windows XP, append an additional parameter `-T "v140_xp"` (or `-T "v120_xp"`, depending on which version of Visual Studio you have installed) to your command line.

If any cmake project file changes, you can just run `cmake .. && cmake ..` to get up to date.
If this by itself doesn't work, you may have to clean the cmake cache.
Use `rm -rf CMakeCache.txt CMakeScripts/ CMakeFiles/ cmake_install.txt` to do that, and then run the generator command again as specified above.

The reason for running cmake at least twice is to make sure that all of the variables get set up appropriately.

Environment variables can be modified at this stage. If you want to pass `-ggdb` or any other flag that is not set up by default,
utilize `CXXFLAGS` or any appropriate variable.

CMake GUI
===

For those that use the GUI to work with cmake, you need to specify where the source code is and where the binaries will be built.
The first one, counter-intuitively, is actually the parent directory of this one: the main StepMania directory.
The second one for building can be this directory.

Upon setting the source and build directories, you should `Configure` the build.
If no errors show up, you can hit `Generate` until none of the rows on the GUI are red.

If the cmake project file changes, you can just generate the build to get up to date.
If this by itself doesn't work, you may have to clean the cmake cache.
Go to File -> Delete Cache, and then run the `Configure` and `Generate` steps again.

Release vs Debug
==

If you are generating makefiles with cmake, you will also need to specify your build type.
Most users will want to use `RELEASE` while some developers may want to use `DEBUG`.

When generating your cmake files for the first time (or after any cache delete),
pass in `-DCMAKE_BUILD_TYPE=Debug` for a debug build. We have `RelWithDbgInfo` and `MinSizeRel` builds available as well.

It is advised to clean your cmake cache if you switch build types.

Note that if you use an IDE like Visual Studio or Xcode, you do not need to worry about setting the build type.
You can edit the build type directly in the IDE.

Last Words
==

With that, you should be good to go.
If there are still questions, view the resources on the parent directory's README.md file.

