#!/usr/bin/perl -w

use strict;
use File::Copy;
use File::Path;
use File::Basename;
use File::Temp qw/tempfile tempdir/;
use Cwd;

my @docs = (
	"Licenses.txt",
	"Changelog_sm5.txt",
	"Changelog_sm-ssc.txt",
	"Changelog_SSCformat.txt",
	"CommandLineArgs.txt",
	"credits.txt"
);

my @themerdocs = (
	"actordef.txt",
	"conditional_music.txt",
	"fontini.txt",
	"gamecommands.txt",
	"included_scripts.txt",
	"moremsg.txt",
	"Noteskin elements Reference.txt",
	"recommended_practices.txt",
	"ScreenMessages.txt",
	"ScreenTextEntry.txt",
	"sm-ssc_themeguide.txt",
	"ThemePrefs.txt",
	"ThemePrefsRows.txt"
);

# XXX
my @songs = ( "Instructions.txt" );
my @song_mechatribe = (
	"Mecha-Tribe Assault.ssc",
	"Mecha-Tribe Assault.ogg",
	"mechatribeassaultbg.png",
	"mechatribeassaultbn.png",
	"wyde cd-tital.png"
);

my @song_springtime = (
	"Springtime.ssc",
	"Kommisar - Springtime.mp3",
	"spring.png",
	"springbn.png"
);

# Passing a date for a CVS release gives StepMania-CVS-date.
# Otherwise you get StepMania-ver.
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

my $tmp = tempdir;

# Copy StepMania and make smzip
system 'cp', '-r', "$srcdir/StepMania.app", $tmp and die "cp -r failed: $!\n";
system 'strip', '-x', "$tmp/StepMania.app/Contents/MacOS/StepMania";

mkdir "$tmp/Packages";
system "$srcdir/Utils/CreatePackage.pl", $srcdir, "$tmp/Packages" and die "mksmdata.pl failed: $!\n";

# Copy docs
mkdir "$tmp/Docs";
copy "$srcdir/Docs/$_", "$tmp/Docs/$_" for @docs;

# Copy themer docs
mkdir "$tmp/Docs/Themerdocs";
copy "$srcdir/Docs/Themerdocs/$_", "$tmp/Docs/Themerdocs/$_" for @themerdocs;

# Copy songs
mkdir "$tmp/Songs";
copy "$srcdir/Songs/$_", "$tmp/Songs/$_" for @songs;

# oh man, is this ugly or what
mkdir "$tmp/Songs/StepMania 5";

mkdir "$tmp/Songs/StepMania 5/MechaTribe Assault";
copy "$srcdir/Songs/StepMania 5/MechaTribe Assault/$_", "$tmp/Songs/StepMania 5/MechaTribe Assault/$_" for @song_mechatribe;

mkdir "$tmp/Songs/StepMania 5/Springtime";
copy "$srcdir/Songs/StepMania 5/Springtime/$_", "$tmp/Songs/StepMania 5/Springtime/$_" for @song_springtime;

# Make a dmg
system qw/hdiutil create -ov -format UDZO -imagekey zlib-level=9 -srcfolder/, $tmp, '-volname', $destname, "$root/$destname-mac.dmg";
rmtree $tmp;
