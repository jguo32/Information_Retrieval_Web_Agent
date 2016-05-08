#!/usr/bin/perl -w

#
# This program walks through HTML pages, extracting all the links to other
# text/html pages and then walking those links. Basically the robot performs
# a breadth first search through an HTML directory structure.
#
# All other functionality must be implemented
#
# Example:
#
#    robot_base.pl mylogfile.log content.txt http://www.cs.jhu.edu/
#
# Note: you must use a command line argument of http://some.web.address
#       or else the program will fail with error code 404 (document not
#       found).

use strict;
use warnings;
use LWP::Simple;

use Carp;
use HTML::LinkExtor;
use HTTP::Request;
use HTTP::Response;
use HTTP::Status;
use LWP::RobotUA;
use URI::URL;
use LWP::UserAgent;
use URI;

URI::URL::strict( 1 );   # insure that we only traverse well formed URL's

$| = 1;

my $ROBOT_DELAY_IN_MINUTES = 0; # could be dangerous if we set it to 0
my $log_file = shift (@ARGV);
my $content_file = shift (@ARGV);
my $result_file = shift (@ARGV);

if ((!defined ($log_file)) || (!defined ($content_file)) || (!defined ($result_file))) {
    print STDERR "You must specify a log file, a content file and a base_url\n";
    print STDERR "when running the web robot:\n";
    print STDERR "  ./robot_base.pl mylogfile.log content.txt base_url\n";
    exit (1);
}

open LOG, ">$log_file";
open CONTENT, ">$content_file";
# open RESULT, ">$result_file";

my $ROBOT_NAME = 'jguo32Bot/1.0';
my $ROBOT_MAIL = 'jguo32@cs.jhu.edu';

#
# create an instance of LWP::RobotUA. 
#
# Note: you _must_ include a name and email address during construction 
#       (web site administrators often times want to know who to bitch at 
#       for intrusive bugs).
#
# Note: the LWP::RobotUA delays a set amount of time before contacting a
#       server again. The robot will first contact the base server (www.
#       servername.tag) to retrieve the robots.txt file which tells the
#       robot where it can and can't go. It will then delay. The default 
#       delay is 1 minute (which is what I am using). You can change this 
#       with a call of
#
#         $robot->delay( $ROBOT_DELAY_IN_MINUTES );
#

my $robot = new LWP::RobotUA $ROBOT_NAME, $ROBOT_MAIL;
$robot->delay( $ROBOT_DELAY_IN_MINUTES);

my $base_url    = shift(@ARGV);   # the root URL we will start from
my $base_domain = undef;

my @search_urls = ();    # current URL's waiting to be trapsed
my @wanted_urls = ();    # URL's which contain info that we are looking for
my %relevance   = ();    # how relevant is a particular URL to our search
my %pushed      = ();    # URL's which have either been visited or are already
                         #  on the @search_urls array
my @pdf_urls    = ();    # URL for the resume we have found
my $save_path = "./resume/"; # Repository to save the resumes

&main;

sub main {
    &get_base_domain($base_url);
    push @search_urls, $base_url;
    $pushed{$base_url} = 1;


    while (@search_urls) {
        my $url = shift @search_urls;

        my $parsed_url = eval { new URI::URL $url; };
        next if $@; # $@ is set to error message produced by eval{}
        next if $parsed_url->scheme !~/http/i;
      
        # get header information on URL
        # If the status is not okay or the content type is not what we are 
        # looking for skip the URL and move on

        print LOG "[HEAD ] $url\n";
        print "Currently visiting: [HEAD] $url\n";

        my $request  = new HTTP::Request HEAD => $url;
        my $response = $robot->request( $request );
      
        next if $response->code != RC_OK;
        next if ! &wanted_content( $response->content_type );
        
        print LOG "[GET  ] $url\n";

        $request->method( 'GET' );
        $response = $robot->request( $request );

        next if $response->code != RC_OK;
        next if $response->content_type !~ m@text/html@;
        # next if ! &GET($url, $request, $response);


        print LOG "[LINKS] $url\n";
        print "[LINKS] $url\n";

        &extract_content ($response->content, $url);

        my @related_urls  = &grab_urls( $response->content, $url );

        # Possibly the original link is redirected to somewhere else
        # since we have filtered the links at the beginning of the loop,
        # so we process them separately
        if ( scalar @related_urls eq 1 and $related_urls[0] !~/$base_domain/) {
            print "redirected\n";
        }
        else {
            foreach my $link (@related_urls) {
                # We don't even look at the raw link if it is not 
                # in the base domain
                next if ($link !~/$base_domain/);

                my $full_url = eval { (new URI::URL $link, $response->base)->abs; };

                delete $relevance{ $link } and next if $@;


                if (defined $full_url) {
                    # Same page, just skip it
                    if ($full_url =~ /$url#.*/) {
                       next;
                    }
                    if ($full_url !~ /$base_domain/) {
                        &process_redirect($full_url);
                        next;
                    }
                }

                $relevance{ $full_url } = $relevance{ $link };
                delete $relevance{ $link } if $full_url ne $link;

                push @search_urls, $full_url and $pushed{ $full_url } = 1
                    if ! exists $pushed{ $full_url };
            }
        }

        # reorder the urls base upon relevance so that we search
        # areas which seem most relevant to us first.
        @search_urls = 
            sort { $relevance{ $a } <=> $relevance{ $b }; } @search_urls;
        print "\n";


        # Seems useless
        # while (@wanted_urls) {
        #     my $url = shift @wanted_urls;
        #     if (defined($url) && !($url eq "")) {
        #         print "Get pdf file: $url\n";
        #         print RESULT "$url\n";
        #     }
        # }
    }
    close LOG;
    close CONTENT;

    exit (0);


}



# Get the domain of the base url for later use of domain restriction
# May not applicable if you want to use the domain of urls other 
# than base url or multiple domains
# Global variable $base_domain will be set here

sub get_base_domain {
    my $base_url = shift;
    my $domain = undef;

    my $parsed_url = eval { new URI::URL $base_url; };
    die "Error when parsing the base url\n"if $@;

    if ( $base_url =~ /http:\/\/www\.([^\/]*).*/) {
        $domain = $1;
    }
    elsif ($base_url =~ /http:\/\/([^\/]*).*/) {
        $domain = $1;
    }
    if (!defined($domain)) {
        print "Error: unknown domain.\n";
    }
    
    $base_domain = $domain;

}

# Process the link that is redirected to somewhere with a different domain

sub process_redirect {
    my $url = shift;
    print "get url";

}
    
sub process_url {
    my $link = shift;
    my $response = shift;
    my $url = shift;
    my $full_url = eval { (new URI::URL $link, $response->base)->abs; };

    delete $relevance{ $link } and return if $@;

    my $base = $response->base;
    my $domain = undef;

    if ( $base =~ /http:\/\/www\.([^\/]*).*/) {
        $domain = $1;
    }
    elsif ($base =~ /http:\/\/([^\/]*).*/) {
        $domain = $1;
    }

    if (!defined($domain)) {
        print "Error: unknown domain.\n";
    }

    if (defined $full_url) {
        if ($full_url =~ /$url#.*/) {
           next;
        }

        if ($full_url !~ /\.$domain/) {
            next;
        }
    }

    $relevance{ $full_url } = $relevance{ $link };
    delete $relevance{ $link } if $full_url ne $link;

    push @search_urls, $full_url and $pushed{ $full_url } = 1
        if ! exists $pushed{ $full_url };

}

# wanted_content
#
#
#  this function checks to see if the current URL content
#  is something which is either
#
#    a) something we are looking for (e.g. postscript, pdf,
#       plain text, or html). In this case we should save the URL in the
#       @wanted_urls array.
#
#    b) something we can traverse and search for links
#       (this can be just text/html).

sub wanted_content {
    my $content = shift;
    my $url = $_[1];

    # right now we only accept text/html
    # for the pdf files we are looking for, we simply save them and
    # move on

    if ($content =~ m@(text/html|text/plain)@) {
        push @wanted_urls, $url;
        return 1;
    }
    
    return 0;

}

# extract_content
#
# TODO: use this function to extract resume-like information from webpage

sub extract_content {
    my $content = shift;
    my $url = shift;

    # my $email;
    # my $phone;
    # my $city;

    # parse out information you want
    # print it in the tuple format to the CONTENT and LOG files, for example:
    
    # while($content =~ s/(\w+@\w+\.\w+(\.\w+){0,1})//){
    #     $email = $1;
    #     print CONTENT "($url; EMAIL; $email)\n";
    #     print LOG "($url; EMAIL; $email)\n";
    # }
    
    # while($content =~ s/\D((\d{3}){0,1}(\(\d{3}\)){0,1}\D\d{3}\D\d{4})\D//){
    #     $phone = $1;
    #     print CONTENT "($url; PHONE; $phone)\n";
    #     print LOG "($url; PHONE; $phone)\n";
    
    # }
    # while($content =~ s/([A-Za-z]+,{0,1}\s[A-Za-z]+,{0,1}\s\d{5}(.\d{4}){0,1})//){
    #     $city = $1;
    #     print CONTENT "($url; CITY; $city)\n";
    #     print LOG "($url; CITY; $city)\n";
    # }



    return;
}

#
# grab_urls
#
#   this function parses through the content of a passed HTML page and
#   picks out all links and any immediately related text.
#
#   Relevancy based on both the link itself and the related text 
#   is calculated and stored in the %relevance hash
#

sub grab_urls {
    my $content = shift;
    my $parent_url = shift;
    my %urls    = ();    

  skip:
    while ($content =~ s/<\s*[aA] ([^>]*)>\s*(?:<[^>]*>)*(?:([^<]*)(?:<[^aA>]*>)*<\/\s*[aA]\s*>)?//) {
      
        my $tag_text = $1;
        my $reg_text = $2;
        my $link = "";

        if ($tag_text =~ /href\s*=\s*(?:["']([^"']*)["']|([^\s])*)/i) {
            $link = $1 || $2;
            if (!defined($link)) {
              next;
            } 

            # Skip those links that are impossible to contain CV or resume
            next if ($link =~/publication|paper|project|slide|image|pub|bibtex/);
            
            # Some publications on computer vision might also have substring "CV" in filename
            if ($link =~ /(CV|cv|resume)+(\w|-)*\.pdf/ and $link !~ /publication/) {

                # Concatenate relative urls
                if ($link !~ /http:\/\//) {
                    if ($parent_url !~ /\/$/) {
                        $parent_url = $parent_url . "/";
                    }
                    $link = URI->new_abs($link, $parent_url);
                }

                print "## Find Resume ##: $link\n";
                my $file = get $link;
                if (defined($file)) {
                    # Replace "/" by "." to make the link a valid filename
                    $link =~ tr/\//\./; 

                    open( FILE, '>', $save_path . $link ) or die $!;
                    binmode FILE;

                    print FILE $file;
                    close( FILE );
                } 
                else {
                    print "error, failed to get remote pdf file!\n";
                }

                push @wanted_urls, $link;
                next;
            }

            $urls{ $link }      = 1;
            $relevance{ $link } = &calc_relevancy($link, $reg_text);
            
        }

        # print $reg_text, "\n" if defined $reg_text;
        # print $link, "\n";
    }

    return keys %urls;   # the keys of the associative array hold all the
                         # links we've found (no repeats).
}

sub calc_relevancy {
    my $link = shift;
    my $reg_text = shift;
    my $rel = 8;

    if (!defined( $reg_text)) {
        return $rel;
    }

    $reg_text =~ s/[\n\r]/ /;
    $reg_text =~ s/\s{2,}/ /;
 
    if ($link =~ /~\w+$/){
        $rel = 1;
    }
    elsif ($link =~ /homepage/ and $reg_text =~ /homepage/){
        $rel = 2;
    }
    elsif ($link =~ /homepage/ or $reg_text =~ /homepage/){
        $rel = 3;
    }
    elsif ($link =~ /faculty/ and $reg_text =~ /faculty/){
        $rel = 4;
    }
    elsif ($link =~ /faculty/ or $reg_text =~ /faculty/){
        $rel = 5;
    }
    elsif ($link =~ /people/ and $reg_text =~ /people/){
        $rel = 6;
    }
    elsif ($link =~ /people/ or $reg_text =~ /people/){
        $rel = 7;
    }

    return $rel;

}
