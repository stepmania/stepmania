#!/usr/bin/perl -w

$ver = 0;
if( open FH, '<ver.cpp' )
{
	$_ = <FH>;
	/version_num = (\d+);/ and $ver = $1+1;
	close FH;
}
chop( $date = `date` );
open FH, '>ver.cpp' or die $!;
# Cannot use const here since gcc will strip it out...
print FH <<"EOF";
unsigned long version_num = $ver;
const char *version_time = "$date";
EOF
close FH;
