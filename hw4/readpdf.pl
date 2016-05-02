use FileHandle;
use strict;

&main;

sub main {
	my $filename = "cv\\ -\\ eisner";
	my $readfile = "./pdftotext ". $filename . ".pdf " . $filename . ".txt";
	# my $readfile = "./pdftotext eisner.pdf test.txt";
	print $readfile;
	system($readfile);

}
