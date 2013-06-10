#!/usr/bin/perl -w
use File::Copy;

$ver = 0;
if( open FH, '<ver.cpp' )
{
	<FH> =~ /version_num = (\d+);/ and $ver = $1 + 1;
	close FH;
}
chop( $date = `date` );
open FH, '>.ver.tmp' or die $!;
print FH <<"EOF";
extern const unsigned long version_num = $ver;
extern const char *const version_time = "$date";
EOF
close FH;
move '.ver.tmp', 'ver.cpp';
