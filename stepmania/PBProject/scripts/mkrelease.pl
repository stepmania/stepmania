#!/usr/bin/perl -w

use strict;
use File::Copy;
use File::Path;
use File::Basename;
use File::Temp qw/tempfile tempdir/;
use Cwd;

my @docs = ( "Licenses.txt" );

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
system "$srcdir/Utils/CreatePackage.pl", $srcdir, "$tmp/StepMania.app/Contents/Resources" and die "mksmdata.pl failed: $!\n";

# Copy docs
mkdir "$tmp/Docs";
copy "$srcdir/Docs/$_", "$tmp/Docs/$_" for @docs;

# Make a dmg
system qw/hdiutil create -ov -format UDZO -imagekey zlib-level=9 -srcfolder/, $tmp, '-volname', $destname, "$root/$destname-mac.dmg";
rmtree $tmp;
