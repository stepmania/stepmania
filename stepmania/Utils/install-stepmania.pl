#!/usr/bin/perl
use warnings;
use strict;

sub ReadCommand
{
	while(my $line = <F>)
	{
		chomp $line;
		$line =~ s/[;#].*//;
		$line =~ s/^ *//;
		$line =~ s/ *$//;
		# \foo\bar -> /foo/bar
		$line =~ s-\\-/-g;

		my $cur = 0;
		my $quote = 0; # none
		my @ret;
		for(my $i = 0; $i < length($line); ++$i)
		{
			my $ch = substr($line,$i,1);
			# Check for quotes.  Any quote causes the parameter to
			# be created; eg:
			# Foo       ""          bar
			# is "Foo", "", "bar".
			if( $quote == 0 && $ch eq '\'' )
			{
				$ret[$cur] .= "";
				$quote = 1;
				next;
			}
			elsif( $quote == 0 && $ch eq '"' )
			{
				$ret[$cur] .= "";
				$quote = 2;
				next;
			}
			elsif( $quote == 1 && $ch eq '\'' )
			{
				$ret[$cur] .= "";
				$quote = 0;
				next;
			}
			elsif( $quote == 2 && $ch eq '"' )
			{
				$ret[$cur] .= "";
				$quote = 0;
				next;
			}
			elsif( $quote == 0 && $ch eq ' ' )
			{
				if( defined $ret[$cur] )
				{
					++$cur;
				}
				
				next;
			}
			
			$ret[$cur] .= $ch;
		}
		

		return @ret;
	}
	print "guh\n";
	
	return 0;
}

sub CreateDirectories
{
	my @dirs = split /\//, $_[0];
	my $dir = "";
	for( my $i = 0; $i <= $#dirs; ++$i )
	{
		$dir .= $dirs[$i] . "/";
		if( ! -d $dir )
		{
			mkdir($dir) || die "mkdir($dir): $!";
		}
	}
}


if ($#ARGV == -1)
{
	print "Install where?  (eg. /usr/games/stepmania)\n";
	exit 1;
}

my $instdir = $ARGV[0];
print "Installing to $instdir\n";

# Normally, this script is run after building, which means we probably have a Makefile
# available.  Look for it, and pull out vpath, if any, to see where the real source
# directory is.

my $bin_path;

if( -e "Makefile" )
{
	my $vpath=`grep 'VPATH =' Makefile`;
	chomp $vpath;
	if( $vpath =~ /^VPATH = (.*)/ )
	{
		print "Vpath: $1\n";
		$bin_path = `pwd`; # arr?
		chomp $bin_path;
		$bin_path .= "/src/";
		chdir("$1") || die "chdir($1): $!";
	}
}
if( !defined($bin_path) )
{
	$bin_path="";
}

print "Binary path: $bin_path\n";

open(F, "stepmania.nsi") || die "Couldn't open stepmania.nsi: $!";


# Search for the default installation section.
my $FoundSection = 0;
while(!eof(F))
{
	my @line=ReadCommand();
	$#line == -1 && next;
	
	if( $#line >= 1 && $line[0] eq "Section" && $line[1] eq "" )
	{
		$FoundSection = 1;
		last;
	}

}

$FoundSection || die "stepmania.nsi parse error";
my $FoundEnd = 0;

if( ! -d $instdir )
{
	mkdir($instdir) || die "mkdir($instdir): $!";
}

my $OutPath = "";
while(!eof(F))
{
	my @line=ReadCommand();
	$#line == -1 && next;

	if ( $line[0] eq "SectionEnd" )
	{
		$FoundEnd = 1;
		last;
	}

	my $ignore=0;
	for( my $i = 0; $i <= $#line; ++$i )
	{
		$line[$i] =~ s/\$INSTDIR/$instdir/g;

		# If any arguments contain $SMPROGRAMS, it's related to the start menu; ignore it.
		if( $line[$i] =~ /\$SMPROGRAMS/ )
		{
			$ignore=1;
		}
	}
	$ignore && next;

#	if ( $line[0] eq "CreateDirectory" )
#	{
#		if( ! -d $line[1] )
#		{
#			mkdir($line[1]) || die "mkdir($line[1]): $!";
#		}
#
#		next;
#	}

	if ( $line[0] eq "SetOutPath" )
	{
		$OutPath = $line[1];
		next;
	}
	
	if ( $line[0] eq "File" )
	{
		my $pos = 1;
		my $recurse = 0;
		if( $line[$pos] eq "/r" )
		{
			++$pos;
			$recurse = 1;
		}
		my $fn = $line[$pos];
			
		# Ignore Windows binaries.
		if ( $fn =~ /.*dll/i || $fn =~ /.*exe/i || $fn =~ /.*vdi/i )
		{
			next;
		}
		
		# Only create directories if we're actually installing something to them.
		CreateDirectories( $OutPath );

		my $args="-p";
		if( $recurse )
		{
			$args="$args -r";
		}

		print "$fn -> $OutPath\n";
		system("cp $args \"$fn\" \"$OutPath\"") && die;

		next;
	}

	if ( $line[0] eq "RMDir" )
	{
		my $pos = 1;
		my $recurse = 0;
		if( $line[$pos] eq "/r" )
		{
			++$pos;
			$recurse = 1;
		}
		my $dir = $line[$pos];
			
		my $args="-f";
		if( $recurse )
		{
			$args="$args -r";
		}

		if( -d $dir )
		{
			system("rm $args \"$dir\"");
		}

		next;
	}
	
}

$FoundSection || print "warning: SectionEnd not found\n";

close F;

system("cp -vp \"" . $bin_path . "stepmania\" \"$instdir\"");
if( -e $bin_path . "GtkModule.so" )
{
	system("cp -vp \"" . $bin_path . "GtkModule.so\" \"$instdir\"");
}


