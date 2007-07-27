#!/usr/bin/perl -w

use strict;
use File::Copy;
use File::Path;
use File::Basename;
use File::Temp qw/tempdir/;
use Cwd qw/getcwd realpath/;

my @data = qw(  BackgroundTransitions
		BackgroundEffects
		Characters/default
		Courses/Samples
		NoteSkins/common/common
		NoteSkins/dance/default
		NoteSkins/dance/bold
		NoteSkins/dance/flat
		Themes/default
		Data );

die "usage: $0 [input_dir output_dir]\n" if @ARGV > 2;

my $srcdir = @ARGV > 0? $ARGV[0]:getcwd;
my $destdir = @ARGV == 2? $ARGV[1]:$srcdir;

$srcdir = realpath $srcdir;
$destdir = realpath $destdir;

my $tmp = tempdir;

for( @data )
{
	mkpath "$tmp/" . dirname $_;
	system 'cp', '-r', "$srcdir/$_", "$tmp/$_" and die "cp -r failed: $!\n";
}
my @cvsdirs = split /\n/, `find "$tmp" -type d -name CVS`;
rmtree \@cvsdirs;
chdir $tmp;
system qq(zip -qyrn .ogg:.mp3:.png "$destdir"/SMData.smzip *) and die "zip failed: $!";
rmtree $tmp
