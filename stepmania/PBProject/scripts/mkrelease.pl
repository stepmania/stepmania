#!/usr/bin/perl -w

use strict;
use File::Copy;
use File::Path;
use File::Basename;
use File::Temp qw/tempfile tempdir/;
use Cwd;

my @filelist = qw( Announcers/instructions.txt
		   BackgroundTransitions
		   BackgroundEffects
		   BGAnimations/instructions.txt
		   CDTitles/Instructions.txt
		   Characters/default
		   Characters/Instructions.txt
		   Courses/instructions.txt
		   Courses/Samples
		   Packages/Instructions.txt
		   NoteSkins/instructions.txt
		   NoteSkins/common/common
		   NoteSkins/dance/default
		   RandomMovies/instructions.txt
		   Songs/Instructions.txt
		   Themes/instructions.txt
		   Themes/default
		   Data/Translations.xml
		   Data/AI.ini
		   Data/splash.png
		   StepMania.app );
my @docs = ( "BGAnimation conditionals.txt",
	     "BMA-fmt.txt",
	     "ConditionalBGA Info.txt",
	     "Copying.MAD",
	     "ChangeLog.txt",
	     "Licenses.txt" );

# Passing a date for a CVS release gives StepMania-CVS-date.
# Otherwise you get Stepmania-ver.
die "usage: $0 [date]\n" if @ARGV > 1;
my $root = getcwd;
my $scripts = dirname $0;
my $srcdir = ( $scripts =~ m{^/} ? "$scripts/../.." :
	       "$root/$scripts/../.." );
my $family;
my $id;
my $ver;

open FH, "$srcdir/src/ProductInfo.h" or die "Where am I?\n";
while( <FH> )
{
	if( /^#define\s+PRODUCT_FAMILY_BARE\s+(.*?)\s*$/ ) {
		$family = $1;
	} elsif( /^#define\s+PRODUCT_ID_BARE\s+(.*?)\s*$/ ) {
		$id = $1;
	} elsif( /^#define\s+PRODUCT_VER_BARE\s+(.*?)\s*$/ ) {
		$ver = $1;
	}
}
close FH;

my $destname = @ARGV ? "$id-$ARGV[0]" : "$family-$ver";

$destname =~ s/\s+/-/g;

my $destdir = tempdir;
my $smdir = "$destdir/$id";
my $pkg = "$root/$destname.pkg";
my $zip = "$root/$destname-mac.zip";

for( @filelist )
{
	mkpath "$smdir/" . dirname $_;
	system 'cp', '-r', "$srcdir/$_", "$smdir/$_" and die "cp -r failed: $!\n";
}

# Copy docs
mkdir "$smdir/Docs";
copy "$srcdir/Docs/$_", "$smdir/Docs/$_" for @docs;

#clean up CVS directories
my @cvsdirs = split /\n/, `find "$smdir" -type d -name CVS`;
rmtree \@cvsdirs;
system 'strip', '-x', "$smdir/StepMania.app/Contents/MacOS/StepMania";

my ($ih, $infoname) = tempfile;
my ($dh, $descname) = tempfile;
my $year = 1900+(localtime)[5];

$ver =~ /(\d+)\.(\d+)/;
my ($major, $minor) = ($1, $2);
$major ||= 0;
$minor ||= 0;

print $ih <<EOF;
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0/EN" "http://www.apple.com/DTDs/P
ropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleGetInfoString</key>
	<string>$ver, Copyright ©2001-$year $family</string>
	<key>CFBundleIdentifier</key>
	<string>com.$family.$id</string>
	<key>CFBundleShortVersionString</key>
	<string>$major.$minor</string>
	<key>IFMajorVersion</key>
	<integer>$major</integer>
	<key>IFMinorVersion</key>
	<integer>$minor</integer>
	<key>IFPkgFlagAllowBackRev</key>
	<true/>
	<key>IFPkgFlagAuthorizationAction</key>
	<string>AdminAuthorization</string>
	<key>IFPkgFlagBackgroundAlignment</key>
	<string>center</string>
	<key>IFPkgFlagBackgroundScaling</key>
	<string>none</string>
	<key>IFPkgFlagDefaultLocation</key>
	<string>/Applications</string>
	<key>IFPkgFlagFollowLinks</key>
	<true/>
	<key>IFPkgFlagInstallFat</key>
	<true/>
	<key>IFPkgFlagInstalledSize</key>
	<integer>36080</integer>
	<key>IFPkgFlagIsRequired</key>
	<false/>
	<key>IFPkgFlagOverwritePermissions</key>
	<false/>
	<key>IFPkgFlagRelocatable</key>
	<true/>
	<key>IFPkgFlagRestartAction</key>
	<string>NoRestart</string>
	<key>IFPkgFlagRootVolumeOnly</key>
	<false/>
	<key>IFPkgFlagUpdateInstalledLanguages</key>
	<false/>
	<key>IFPkgFormatVersion</key>
	<real>0.10000000149011612</real>
</dict>
</plist>
EOF
close $ih;
print $dh <<EOF;
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>IFPkgDescriptionDescription</key>
	<string></string>
	<key>IFPkgDescriptionTitle</key>
	<string>$id</string>
</dict>
</plist>
EOF
close $dh;
if( -e $pkg )
{
	print "Removing $pkg\n";
	rmtree "$pkg";
}
my @headerdir = split /\n/, `find "$smdir/StepMania.app" -type d -name Headers`;
rmtree \@headerdir;
my $pm =
'/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker';
system $pm, '-build', '-p', $pkg, '-f', $destdir, '-ds',
	'-i', $infoname, '-d', $descname;
unlink $infoname, $descname;
rmtree $destdir;
print "Created $destname.pkg.\n";
if( -e $zip )
{
	print "Removing $zip\n";
	unlink $zip;
}
chdir $root;
system '/usr/bin/zip', '-9Tqyr', $zip, "$destname.pkg";
print "Created $destname-mac.zip.\n";
