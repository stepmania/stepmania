#!/bin/sh

#Stupid ProjectBuild wouldn't allow perl

perl -w -e '
open F, "<version.tst"||die;
$ver = <F>;
close F;
chomp $ver;
++$ver;
open F,">version.tst"||die;
print F "$ver\n";
close F;
$date = `date`;
chomp $date;
open F, ">ver.cpp"||die;
print F "unsigned long version_num = $ver;\n";print F "const char *version_time = \"$date\";\n";close F;
'
touch ver.cpp