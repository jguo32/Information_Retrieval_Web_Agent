#!/usr/local/bin/perl5 -w

use strict;
use HTTP::Status;
use HTTP::Response;
use LWP::UserAgent;
use URI::URL;
use HTML::Parse;
use vars qw($opt_h $opt_i $opt_l $opt_p);
use Getopt::Std;

my $url;

getopts('hilp:');
my $all = !($opt_i || $opt_l);       # $all=1 when -i -l not set

if ($opt_h || $#ARGV==-1) {  # print help text when -h or no args
  print_help();
  exit(0);
}


while ($url = shift @ARGV) {

  my ($code, $type, $data)  = get_html($url, $opt_p, $opt_i, $opt_l);
  if (not_good($code, $type)) { next; }
  if ($opt_i || $all) { print_images($data, $url); }
  if ($opt_l || $all) { print_hyperlinks($data, $url); }

} # while there are URLs on the command line

sub print_help {
  print << "HELP";
usage: $0 [-hilp] [proxy URL] URLs

 -h help
 -i grep out images references only
 -l grep out hyperlink references only
 -p use this proxy server

Example:  $0 -p http://proxy:8080/ http://www.ora.com

HELP
}

sub get_html() {
  my($url, $proxy, $want_image, $want_link) = @_;

# Create a User Agent object
  my $ua = new LWP::UserAgent;
  $ua->agent("hgrepurl/1.0");

# If proxy server specified, define it in the User Agent object
  if (defined $proxy) {
    my $proxy_url = new URI::URL $url;
    $ua->proxy($proxy_url->scheme, $proxy);
  }

# Ask the User Agent object to request a URL.
# Results go into the response object (HTTP::Reponse).

  my $request = new HTTP::Request('GET', $url);
  my $response = $ua->request($request);

  return ($response->code, $response->content_type, $response->content);
}



# returns 1 if the request was not OK or HTML, else 0

sub not_good {
  my ($code, $type) = @_;

  if ($code != RC_OK) {
    warn("$url had response code of $code");
    return 1;
  }

  if ($type !~ m@text/html@) {
    warn("$url is not HTML.");
    return 1;
  }
  return 0;
}

sub print_images {

  my ($data, $model) = @_;

  my $parsed_html=HTML::Parse::parse_html($data);
  for (@{ $parsed_html->extract_links(qw (body img)) }) {
    my ($link) = @$_;
    my ($absolute_link) = globalize_url($link, $model);
    print "$absolute_link\n";
  }
  $parsed_html->delete(); # manually do garbage collection
}

sub print_hyperlinks {

  my ($data, $model) = @_;

  my $parsed_html=HTML::Parse::parse_html($data);
  for (@{ $parsed_html->extract_links(qw (a)) }) {
    my ($link) = @$_;
    my ($absolute_link) = globalize_url($link, $model);
    print "$absolute_link\n";
  }
  $parsed_html->delete(); # manually do garbage collection
}


sub globalize_url() {

  my ($partial, $model) = @_;
  my $url = new URI::URL($partial, $model);
  my $globalized = $url->abs->as_string;

  return $globalized;
}


