StepMania Scripts For OS X
=========

These scripts can be used to bundle a .dmg installer of StepMania.
It assumes you have already built a StepMania.app using Xcode and
that your *stepmania* directory structure is intact as you cloned it.


Bundling a Release:
-------------------
To bundle a release version of StepMania open a terminal and cd to
your *stepmania* directory, then run the following commands:

```
cd Xcode/scripts  
ruby mkrelease.rb
```

This will create a disk image like *StepMania-v5.0-beta-4-mac.dmg*
in the root of your *stepmania* directory.



Bundling a Nightly:
-------------------
To bundle an intermediate or "nightly" .dmg installer, cd to your
*stepmania* directory, and run the following commands:

```
cd Xcode/scripts  
ruby mkrelease.rb nightly
```

This will create a disk image like *StepMania-v5.0-31-8-2014-mac.dmg*
in the root of your *stepmania* directory.