#!/usr/bin/perl
use warnings;
use strict;
use LWP::Simple;

my $save_path = "./resume/"; 
my $parent_url = "http://cs.jhu.edu/~jason/";
my $link =  '/~jason/cv.pdf';
# my $mod_link;

if ( $link =~ /$parent_url/ ) {
	print "no need to modify this.";
}
else {
	if ( $link =~ /\/(\w+\.pdf)/) {
		my $mod_link = $1;
		if (defined($mod_link)) {
			print "$mod_link\n";
			$link = $mod_link;
		}		
	}
	$link = $parent_url . $link;
}

my $file = get $link;
$link =~ tr/\//\./; 

open( FILE, '>', $save_path . $link ) or die $!;
binmode FILE;
print FILE $file;
close( FILE );