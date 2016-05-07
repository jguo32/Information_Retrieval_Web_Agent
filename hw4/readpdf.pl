use FileHandle;
use strict;
use LWP::UserAgent;

&main;

sub main {
	# my $filename = "cv\\ -\\ eisner";
	# my $readfile = "./pdftotext ". $filename . ".pdf " . $filename . ".txt";
	# # my $readfile = "./pdftotext eisner.pdf test.txt";
	# print $readfile;
	# system($readfile);
	my $ua = LWP::UserAgent->new;

	my $request  = HTTP::Request->new( GET => 'http://google.com/' );
	my $response = $ua->request($request);
	if ( $response->is_success and $response->previous ) {
	    print $request->url, ' redirected to ', $response->request->uri, "\n";
	}

}
