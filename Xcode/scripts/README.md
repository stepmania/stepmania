StepMania Scripts For OS X
=========

This script can be used to bundle a .dmg installer of StepMania.
It assumes you have already built a StepMania.app using Xcode and
that your stepmania directory structure is intact as you cloned it.

To use this script, open a terminal and cd to your stepmania directory,
then run the following commands:

cd Xcode/scripts  
ruby mkrelease.rb

You'll see output as each stepmania sub-directory is copied into a tmp directory.
If all goes well, you'll be notified that the dmg was created in your stepmania directory.