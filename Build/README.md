Warning
==
Using CMake is considered stable, but not every single combination is known to work.
Using the defaults as suggested should cause minimal problems.

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

Replace {YourGeneratorHere} with one of the generator choices from `cmake --help`.

If any cmake project file changes, you can just run `cmake .. && cmake ..` to get up to date.
If this by itself doesn't work, you may have to clean the cmake cache.
Use `rm -rf CMakeCache.txt CMakeScripts/ CMakeFiles/ cmake_install.txt` to do that, and then run the generator command again.

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
pass in `-DCMAKE_BUILD_TYPE=Debug` for a debug build. We have `RelWithDbgInfo` and `MinSizeRel` available as well.

It is advised to clean your cmake cache if you switch builds.

Note that if you use an IDE like Visual Studio or Xcode, you do not need to worry about setting the build type.
You can edit the build type directly in the IDE.

Last Words
==

With that, you should be good to go.
If there are still questions, view the resources on the parent directory's README.md file.

