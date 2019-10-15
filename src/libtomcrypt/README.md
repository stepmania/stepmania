# libtomcrypt

Previously the git repository contained `doc/crypt.pdf` for detailed documentation.
This was changed and the file is now only available from the tarball of the appropriate version
or from the page https://github.com/libtom/libtomcrypt/releases .

## Project Status

master: [![Build Status](https://api.travis-ci.org/libtom/libtomcrypt.png?branch=master)](https://travis-ci.org/libtom/libtomcrypt) [![Coverage Status](https://coveralls.io/repos/libtom/libtomcrypt/badge.png?branch=master)](https://coveralls.io/r/libtom/libtomcrypt)

develop: [![Build Status](https://api.travis-ci.org/libtom/libtomcrypt.png?branch=develop)](https://travis-ci.org/libtom/libtomcrypt) [![Coverage Status](https://coveralls.io/repos/libtom/libtomcrypt/badge.png?branch=develop)](https://coveralls.io/r/libtom/libtomcrypt)

[![Coverity Scan Build Status](https://scan.coverity.com/projects/487/badge.svg)](https://scan.coverity.com/projects/487)

API/ABI changes: [check here](https://abi-laboratory.pro/tracker/timeline/libtomcrypt/)

## Submitting patches

Please branch off from develop if you want to submit a patch.

Patch integration will be faster if tests and documentation are included.

Please update the makefiles in a separate commit. To update them simply run the `updatemakes.sh` script.

If you have something bigger to submit, feel free to contact us beforehand.
Then we can give you write access to this repo, so you can open your PR based on this repo
and we can easier follow the rebase-before-merge approach we're using (or even do the rebase ourself).

### Reviews

We're using Pull Request reviews to make sure that the code is in line with the existing code base.

Please have a look [here](https://help.github.com/articles/approving-a-pull-request-with-required-reviews/) to get an idea of the approach.

## Branches

Please be aware, that all branches besides _master_ and _develop_ __can__ and __will be__ force-pushed, rebased and/or removed!

If you want to rely on such an _unstable_ branch, create your own fork of this repository to make sure nothing breaks for you.

## Configuration options

By default the library builds its entire feature set (besides `katja`) in a (depending on your needs more or less) optimal way.

There are numerous configuration options available if you want to trim down the functionality of the library.

Please have a look at `src/headers/tomcrypt_custom.h` for all available configuration options.

The following list is a small part of the available, but the most often required, configuration switches.

| Flag | Behavior |
| ---- | -------- |
| `LTC_NO_TEST` | Remove all algorithm self-tests from the library |
| `LTC_NO_FILE` | Remove all API functions requiring a pre-defined `FILE` data-type (mostly useful for embedded targets) |
| `GMP_DESC` | enable [gmp](https://gmplib.org/) as MPI provider *\*1* |
| `LTM_DESC` | enable [libtommath](http://www.libtom.net/) as MPI provider *\*1* |
| `TFM_DESC` | enable [tomsfastmath](http://www.libtom.net/) as MPI provider *\*1* *\*2* |
| `USE_GMP` | use `gmp` as MPI provider when building the binaries *\*3* |
| `USE_LTM` | use `libtommath` as MPI provider when building the binaries *\*3* |
| `USE_TFM` | use `tomsfastmath` as MPI provider when building the binaries *\*3* |

*\*1* It is possible to build the library against all MPI providers in parallel and choose at startup-time which math library should be used.

*\*2* Please be aware that `tomsfastmath` has the limitation of a fixed max size of MPI's.

*\*3* Only one is supported at the time & this is only required when building the binaries, not when building the library itself.

## Building the library

There are several `makefile`s provided. Please choose the one that fits best for you.

| makefile | use-case |
| -------- | -------- |
| `makefile` | builds a static library (GNU Make required) |
| `makefile.shared` | builds a shared (and static) library (GNU Make required) |
| `makefile.unix` | for unusual UNIX platforms, or if you do not have GNU Make |
| `makefile.mingw` | for usage with the mingw compiler on MS Windows |
| `makefile.msvc` | for usage with the MSVC compiler on MS Windows |
| `libtomcrypt_VS2008.sln` | A VisualStudio 2008 project for MS Windows |

### Make targets

The `makefile`s provide several targets to build (VS project excluded).
The following list does not claim to be complete resp. to be available across all `makefile` variants.

| target | application |
| ------ | ----------- |
| *empty target*/none given | c.f. `library`
| `library` | builds only the library |
| `hashsum` | builds the `hashsum` binary, similar to [`shasum`](https://linux.die.net/man/1/shasum), but with support for all hash-algorithms included in the library *\*4* |
| `ltcrypt` | builds the `ltcrypt` binary, implementing something similar to [`crypt`](https://linux.die.net/man/3/crypt) *\*4* |
| `sizes` | builds the `sizes` binary, printing all internal data sizes on invocation *\*4* |
| `constants` | builds the `constants` binary, printing all internal constants on invocation *\*4* |
| `openssl-enc` | builds the `openssl-enc` binary, which is more or less compatible to [`openssl enc`](https://linux.die.net/man/1/enc) *\*4* *\*5* |
| `test` | builds the `test` binary, which runs all algorithm self-tests + some extended tests *\*4* |
| `timing` | builds the `timing` binary, which can be used to measure timings for algorithms and modes *\*4* |
| `bins` | builds `hashsum` *\*4* |
| `all_test` | builds `test`, `hashsum`, `ltcrypt`, `small`, `tv_gen`, `sizes` & `constants` *\*4* |
| `docs` | builds the developer documentation `doc/crypt.pdf` |
| `install` | installs the `library` and header files *\*7* *\*8* |
| `install_bins` | installs the binaries created by the `bins` target *\*7* *\*8* |
| `install_docs` | installs the documentation created by the `docs` target *\*7* *\*8* |
| `install_test` | installs the test-app created by the `test` target *\*7* *\*8* |
| `install_all` | installs everything (i.e. `library`, `bins`, `docs` and `test`) *\*8* |
| `uninstall` | uninstalls the `library` and header files |

*\*4* also builds `library`

*\*5* broken build in some configurations, therefore not built by default

*\*7* also builds the necessary artifact(s) before installing it

*\*8* also have a look at the 'Installation' section of this file

### Examples

You want to build the library as static library

    make

You want to build the library as shared library

    make -f makefile.shared

You have `libtommath` installed on your system and want to build a static library and the `test` binary to run the self-tests.

    make CFLAGS="-DUSE_LTM -DLTM_DESC" EXTRALIBS="-ltommath" test

You have `tomsfastmath` installed on your system and want to build a shared library and all binaries

    make -f makefile.shared CFLAGS="-DUSE_TFM -DTFM_DESC" EXTRALIBS="-ltfm" all demos

You have `gmp`, `libtommath` and `tomsfastmath` installed on your system and want to build a static library and the `timing` binary to measure timings against `gmp`.

    make CFLAGS="-DUSE_GMP -DGMP_DESC -DLTM_DESC -DTFM_DESC" EXTRALIBS="-lgmp" timing

If you have `libtommath` in a non-standard location:

    make CFLAGS="-DUSE_LTM -DLTM_DESC -I/opt/devel/ltm" EXTRALIBS="/opt/devel/ltm/libtommath.a" all

## Installation

There exist several _install_ make-targets which are described in the table above.

These targets support the standard ways (c.f. [[GNU]], [[FreeBSD]])
to modify the installation path via the following set of variables:

    DESTDIR
    PREFIX
    LIBPATH
    INCPATH
    DATAPATH
    BINPATH

The entire set of the variables is only supported in `makefile`, `makefile.shared` and `makefile.unix`.

In case you have to use one of the other makefiles, check in the file which variables are supported.

### Examples

You want to install the static library to the default paths

    make install

You want to install the shared library to a special path and use it from this path

    make -f makefile.shared PREFIX=/opt/special/path

Have a look at the developer documentation, [[GNU]] or [[FreeBSD]] to get a detailed explanation of all the variables.

[GNU]: https://www.gnu.org/prep/standards/html_node/DESTDIR.html

[FreeBSD]: https://www.freebsd.org/doc/en/books/porters-handbook/porting-prefix.html
