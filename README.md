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

### Prerequisites
* A `C/C++` compiler
* [Git](https://git-scm.com/)
* [CMake](https://cmake.org/)

Install those with a package manager of your choice or manually.

### Dependencies
* [FFmpeg](https://ffmpeg.org/)
  * specifically `libavformat`, `libavutil` and `libswscale`
  * optional, disable by setting the CMake option `USE_FFMPEG=OFF`

Install those with a package/library manager of your choice.

#### Windows

On Windows we recommend [vcpkg](https://github.com/Microsoft/vcpkg). Note that we don't support `x86-64` on Windows yet (see [#1382](https://github.com/stepmania/stepmania/issues/1382)). See [target triplet](https://github.com/microsoft/vcpkg/blob/master/docs/users/triplets.md) to change the target environment.

You also need to install the [DirectX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812) (see [#1874](https://github.com/stepmania/stepmania/issues/1874))

### [Generate a Project Buildsystem](https://cmake.org/cmake/help/latest/manual/cmake.1.html#generate-a-project-buildsystem)
```bash
$ mkdir build
$ cd build
$ cmake .. -G"$CMAKE_MAKE_PROGRAM"
```

With `$CMAKE_MAKE_PROGRAM` being the [CMake generator](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html) of your choice. On Windows we don't support `Visual Studio 15 2017` or higher yet (see [#1875](https://github.com/stepmania/stepmania/issues/1875)).

On single-configuration generators specify the build type (e.g. `Release`, `Debug`, see [`CMAKE_BUILD_TYPE`](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html)) using:
```bash
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE
```

When using vcpkg specify its toolchain with:
```bash
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
```

You can change the target environment (see [vcpkg - triplet selection](https://github.com/microsoft/vcpkg/blob/master/docs/users/integration.md#with-cmake)) for this project like this:
```bash
    -DVCPKG_TARGET_TRIPLET=$VCPKG_TARGET_TRIPLET
```

On systems with multiple available compilers you can use a non-default one with:
```bash
    -DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX
```

If you want to install the project, you can specify a custom prefix using:
```bash
    -DCMAKE_INSTALL_PREFIX="$PREFIX"
```

### [Build the project](https://cmake.org/cmake/help/latest/manual/cmake.1.html#build-a-project)
```bash
$ cmake --build .
```

When using a multi-configuration tool, specify the configuration (e.g. `Release`, `Debug`) with:
```bash
    --config $CONFIG
```

You can override the native build tool's default maximum number of concurrent processes to use when building with:
```bash
    --parallel $JOBS
```

### Install the project
```bash
$ cmake --build build --target install
```

Set `$DESTDIR` for overriding the destination directory.

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
