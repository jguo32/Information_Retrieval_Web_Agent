use FileHandle;
use strict;
use LWP::UserAgent;
use URI;
use Net::Domain qw(hostname hostfqdn hostdomain domainname);

&main;

sub main {
	# my $filename = "cv\\ -\\ eisner";
	# my $readfile = "./pdftotext ". $filename . ".pdf " . $filename . ".txt";
	# # my $readfile = "./pdftotext eisner.pdf test.txt";
	# print $readfile;
	# system($readfile);
# 	my $ua = LWP::UserAgent->new;

# 	my $request  = HTTP::Request->new( GET => 'http://www.cs.jhu.edu/~alopez/
# ' );
# 	my $response = $ua->request($request);
# 	if ( $response->is_redirect) {
# 		print "success\n";
# 	}
# 	if ( $response->is_success and $response->previous ) {
# 	    print $request->url, ' redirected to ', $response->request->uri, "\n";
# 	}
	# my $url = "http://www.cs.jhu.edu/~alopez/";
	# my $mech = WWW::Mechanize->new();
	# $mech->requests_redirectable([]);
	# $mech->get($url);
	# print $mech->response->code . " " . $mech->response->header("Location");
# http://www.cs.jhu.edu/~micharu1/michaelrushanan.org/
	my $abs_url = URI->new_abs('CV.pdf', 'http://www.cs.jhu.edu/~andyjhma/index.html');
	print $abs_url, "\n";

	my $url = URI->new( "http://www.cs.jhu.edu/" );
	my $domain = $url->host;
	print $domain;
}
