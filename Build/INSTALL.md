Warning
==
Make sure you read README.md first if you have not.

Compiling StepMania
==
To use StepMania on your computer, it is first assumed that cmake is run (see README.md for more information).
Then, follow the guide based on your operating system.

Windows
===
Using Visual Studio, simply build and it will place the .exe file in the correct directory.

Mac OS X
===
Using Xcode, simply build in Xcode and it will place the .app file in the correct directory.

Linux
===
Using the command line, simply type make and it will place stepmania and GtkModule.so
(if requested) in the root StepMania directory. There is no more need to symlink the files.

Installing StepMania
==
Installing in this context refers to placing the folders and generated binary in a standard location based on your operating system.
This guide assumes default install locations.
If you want to change the initial location, pass in `-DCMAKE_INSTALL_PREFIX=/new/path/here` when configuring your local setup.

Windows
===
The default installation directory is `C:\Program Files (x86)\Stepmania 5`.

Mac OS X
===
The `StepMania.app` package can be copied to `/Applications` and it will work as expected.

Linux
===
After installing, run `sudo make install`. The files will be placed in the location specified:
by default, that is now `/usr/local/stepmania-5.1`.

