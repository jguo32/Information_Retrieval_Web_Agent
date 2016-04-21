#!/usr/local/bin/perl -w

#
# This program extracts all the image links from the web page given on 
# the command line and prints them to STDOUT. This program is the same
# as the lwp_parser.pl program except that it use the package HTML::LinkExtor
# to extract links. This class gives you more control in parsing links
#
# Example:
#
#    lwp_client.pl http://www.cs.jhu.edu/~jkloss/index.shtml
#
# Note: you must use a command line argument of http://some.web.address
#       or else the program will fail with error code 404 (document not
#       found).

use strict;

use HTML::LinkExtor;
use HTTP::Request;
use HTTP::Response;
use LWP::UserAgent;
use URI::URL;

my $ROBOT_NAME = 'KlossBot/1.0';
my $ROBOT_MAIL = 'jkloss@cs.jhu.edu';


my $ua = new LWP::UserAgent;  # create an new LWP::UserAgent
$ua->agent( $ROBOT_NAME );    # identify who we are
$ua->from ( $ROBOT_MAIL );    # and give an email address in case anyone would
                              # like to complain

my @images = ( );   # array to store found image links

#
# call back routine used by the HTML::LinkExtor object to parse passed
# input lines
#

sub call_back {
    my ( $tag    ,  # the <"TAG" ... > value parsed out 
	 %attribs,  # the attribute which follow $tag
	) = @_;
    
    #
    # currently we only retrieve and parse "img" tags
    # 
    # a more fine grained approach to parsing could be implemented
    # here.
    #

    push @images, $attribs{ 'src' } if $tag eq "img";
}
	
my $extract = new HTML::LinkExtor( \&call_back );


#
# create a request object associating the 'GET' method with the URL we
# want to connect to. Then have the UserAgent contact the web server and
# return the server's response.
#
# Note: during the parsing of $response the sub reference will call 
#       $extract->parse( ) for each buffered line read. $extract will
#       then call $call_back (from &parse) and extact image links.
#

my $request  = new HTTP::Request 'GET' => "$ARGV[0]"; 
my $response = $ua->request( $request, sub{ $extract->parse( $_[0] ) });
my $base     = $response->base;

# translate each link to a fully qualified URL

@images = map { $_ = url($_, $base)->abs; } @images;

print join "\n", @images, "\n";
