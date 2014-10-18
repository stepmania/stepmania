Warning
==
This is considered unstable. This may work on Windows and Mac. This is not guaranteed to work on Linux right now.

Use this at your own risk.

CMake Installation
==
There are two ways of working with cmake: the command line and the GUI.

CMake Command Line
===

If you are unfamiliar with cmake, first run `cmake --help`. This will present a list of options and generators. The generators are used for setting up your project.

For the first setup, you will want to run this command:

`cmake -G {YourGeneratorHere} .. && cmake ..`

Replace {YourGeneratorHere} with one of the generator choices from `cmake --help`.

If any cmake project file changes, you can just run `cmake .. && cmake ..` to get up to date. If this by itself doesn't work, you may have to clean the cmake cache. Use `rm -rf CMakeCache.txt CMakeScripts/ CMakeFiles/ cmake_install.txt` to do that, and then run the generator command again.

The reason for running cmake at least twice is to make sure that all of the variables get set up appropriately.

CMake GUI
===

For those that use the GUI to work with cmake, make sure your source directory is the parent directory and the build files go in this one. Keep generating until none of the lines are red.

If the cache has to be deleted, go to File -> Delete Cache, and then start again.

With that, you should be good to go.

