#!/usr/bin/perl

use strict;

my @filelist = qw(Announcers/instructions.txt
				  BackgroundTransitions
				  BackgroundEffects
				  BGAnimations/instructions.txt
				  CDTitles/instructions.txt
				  Characters/default
				  Courses/instructions.txt
				  Courses/Samples
				  Packages/instructions.txt
				  NoteSkins/instructions.txt
				  NoteSkins/common/default
				  NoteSkins/dance/default
				  NoteSkins/dance/flat
				  NoteSkins/dance/note
				  NoteSkins/dance/solo
				  NoteSkins/pump/Classic
				  NoteSkins/pump/default
				  RandomMovies/instructions.txt
				  Songs/instructions.txt
				  Themes/instructions.txt
				  Themes/default
				  Data/Translation.dat
				  Data/AI.ini
				  Data/splash.png
				  README-FIRST.html
				  NEWS
				  StepMania.app);
my @docs = qw(Copying.txt ChangeLog.txt);

die "usage: $0 path version\n" if @ARGV < 2;
my $srcdir = $ARGV[0];
my $destdir = "StepMania-$ARGV[1]";
my $smdir = "$destdir/StepMania";
my $scripts = `dirname $0`;

chomp $scripts;

if (-e $destdir)
{
	print "Removing $destdir.\n";
	system(("rm", "-rf", "$destdir")) == 0 or die "$0: rm -rf failed: $!\n";
}

mkdir $destdir;
mkdir $smdir;
foreach (@filelist)
{
	my $file = "$srcdir/$_";
	my @parts = split /\//;

	if (@parts > 1)
	{
		pop @parts;
		my $dir = "$smdir/" . join '/', @parts;
		my @args = ("mkdir", "-p", $dir);

		system(@args) == 0 or die "$0: mkdir -p failed: $!\n";
	}

	my @args = ("cp", "-r", $file, "$smdir/$_");

	system(@args) == 0 or die "$0: cp -r failed: $!\n";
}
foreach (@docs)
{
	my @args = ("cp", "$srcdir/Docs/$_", "$smdir/$_");
	
	system(@args) == 0 or die "$0: cp -v failed: $!\n";
}
#clean up CVS directories
my @args = ('find', $smdir, '-name', 'CVS', '-prune', '-exec', 'rm',
			'-rf', '{}', ';');
system(@args) == 0 or die "$0: failed to clean up CVS directories: $!\n";
system(("strip", "-x", "$smdir/StepMania.app/Contents/MacOS/StepMania"));

my $rscdir = "Install_Resources";

if (-e $rscdir)
{
	print "Removing $rscdir.\n";
	system(("rm", "-rf", "$rscdir")) == 0 or die "$0: rm -rf failed: $!\n";
}

# Make the resources directory now
mkdir $rscdir;
system(("cp", "$srcdir/README-FIRST.html", "$rscdir/ReadMe.html")) == 0
  or die "$0: cp failed: $!\n";
system(("cp", "$srcdir/Docs/Copying.txt", "$rscdir/License.txt")) == 0
  or die "$0: cp failed $!\n";
system(("cp", "$scripts/postinstall", $rscdir)) == 0
  or die "$0: cp failed $!\n";

# build the package
# my $tool = "/Developer/Applications/Utilities/PackageMaker.app/"
#   . "Contents/MacOS/PackageMaker";
# @args = ($tool, "-build", "-p", "$destdir.pkg", "-f", $destdir,
# 		 "-r", $rscdir);
# system(@args) == 0 or die "$0: system @args failed: $!\n";
print "Success.\n";
