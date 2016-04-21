#!/usr/local/bin/perl5

use Tk;
require LWP::UserAgent;
use HTML::Parse;

%html_action =
  (
   "</TITLE>", \&end_title,
   "<H1>", \&start_heading,
   "</H1>", \&end_heading,
   "<H2>", \&start_heading,
   "</H2>", \&end_heading,
   "<H3>", \&start_heading,
   "</H3>", \&end_heading,
   "<H4>", \&start_heading,
   "</H4>", \&end_heading,
   "<H5>", \&start_heading,
   "</H5>", \&end_heading,
   "<H6>", \&start_heading,
   "</H6>", \&end_heading,
   "<P>", \&paragraph,
   "<BR>", \&line_break,
   "<HR>", \&draw_line,
   "<A>", \&flush_text,
   "</A>", \&end_link,
   "</BODY>", \&line_break,
  );

$ua = new LWP::UserAgent;
$dictionary_url = "http://work.ucsd.edu:5141/cgi-bin/http_webster";

$mw = MainWindow->new;
$mw->title("xword");
$mw->CmdLine;

$frame1 = $mw->Frame(-borderwidth => 2,
		     -relief => 'ridge');
$frame1->pack(-side => 'top',
	      -expand => 'n',
	      -fill => "x");
$frame2 = $mw->Frame;
$frame2->pack(-side => 'top', -expand => 'yes', -fill => 'both');
$frame3 = $mw->Frame;
$frame3->pack(-side => 'top', -expand => 'no', -fill => 'x');

$frame1->Label(-text => "Enter Word: ")->pack(-side => "left",
					      -anchor => "w");
$entry = $frame1->Entry(-textvariable => \$word,
			-width => 40);
$entry->pack(-side => "left",
	     -anchor => "w",
	     -fill => "x",
	     -expand => "y");

$bttn = $frame1->Button(-text => "Lookup",
			-command => sub { &do_search(); });
$bttn->pack(-side => "left",
	    -anchor => "w");

$entry->bind('<Return>', sub { &do_search(); } );

$scroll = $frame2->Scrollbar;
$text = $frame2->Text(-yscrollcommand => ['set', $scroll],
		      -wrap => 'word',
		      -font => 'lucidasans-12',
		      -state => 'disabled');
$scroll->configure(-command => ['yview', $text]);
$scroll->pack(-side => 'right', -expand => 'no', -fill => 'y');
$text->pack(-side => 'left', -anchor => 'w',
	    -expand => 'yes', -fill => 'both');

$frame3->Label(-textvariable => \$INFORMATION,
	       -justify => 'left')->pack(-side => 'left',
					 -expand => 'no',
					 -fill => 'x');
$frame3->Button(-text => "Exit",
	    -command => sub{exit} )->pack(-side => 'right',
				       -anchor => 'e');
$text->tag('configure', '</H1>', -font => 'lucidasans-bold-24');
$text->tag('configure', '</H2>', -font => 'lucidasans-bold-18');
$text->tag('configure', '</H3>', -font => 'lucidasans-bold-14');
$text->tag('configure', '</H4>', -font => 'lucidasans-bold-12');
$text->tag('configure', '</H5>', -font => 'lucidasans-bold-12');
$text->tag('configure', '</H6>', -font => 'lucidasans-bold-12');
$entry->focus;
MainLoop;
sub do_search {
    my ($url) = @_;
    
    return if ($word =~ /^\s*$/);
    
    $url = "$dictionary_url?$word" if (! defined $url);

$INFORMATION = "Connect: $url";

    $text->configure(-cursor=> 'watch');
    $mw->idletasks;
    my $request = new HTTP::Request('GET', $url);
    
    my $response = $ua->request($request);
    if ($response->is_error) {
	$INFORMATION = "ERROR: Could not retrieve $url";
    } elsif ($response->is_success) {
	my $html = parse_html($response->content);

	## Clear out text item
	$text->configure(-state => "normal");
	
	$text->delete('1.0', 'end');
	$html->traverse(\&display_html);
	$text->configure(-state => "disabled");
	$html_text = "";
	$INFORMATION = "Done";
    }
    
    $text->configure(-cursor => 'top_left_arrow'); 
}
sub display_html {
    my ($node, $startflag, $depth) = @_;
    my ($tag, $type, $coderef);  ## This tag is the HTML tag...
    
    if (!ref $node) {
$html_text .= $node;
    } else {
if ($startflag) {
$tag = $node->starttag;
} else {
	    	$tag = $node->endtag;
}

## Gets rid of any 'extra' stuff in the tag, and saves it
if ($tag =~ /^(<\w+)\s(.*)>/) {
	    	$tag = "$1>";
	    	$extra = $2;
}
	
if (exists $html_action{$tag}) {
$html_text =~ s/\s+/ /g;
	    	&{ $html_action{$tag} }($tag, $html_text);
	   	$html_text = "";
}
    }
    1;
}

sub end_title {
    $mw->title("xword: ". $_[1]);
}
sub start_heading {
    &flush_text(@_);
    $text->insert('end', "\n\n");
}
sub end_heading {
    $text->insert('end', $_[1], $_[0]);
    $text->insert('end', "\n");
}
sub paragraph {
    &flush_text(@_);
    $text->insert('end', "\n\n");
}
sub line_break {
    &flush_text(@_);
    $text->insert('end', "\n");
}

sub draw_line {
    &flush_text(@_);
    $text->insert('end',
"\n---------------------------------------------\n");
}

sub flush_text {
    $text->insert('end', $_[1]);
}

sub end_link {
    ## Don't want to add links to mailto refs.
    if ($extra =~ /HREF\s*=\s*"(.+)"/ && $extra !~ /mailto/) {
	my $site = $1;

	## The tags must have unique names to allow for a different
	## binding to it. (Otherwise we'd just be changing that same 
	## tag binding over and over again)
	
	my $newtag = "LINK". $cnt++;
	
	$text->tag('configure', $newtag, -underline => 'true',
		   -foreground => 'blue');
	$text->tag('bind', $newtag, '<Enter>', 
		   sub { $text->configure(-cursor => 'hand2');
			 $INFORMATION = $site; });
	$text->tag('bind', $newtag, '<Leave>', 
		   sub { $text->configure(-cursor =>
'top_left_arrow');
			 $INFORMATION = "";});
	
	$text->tag('bind', $newtag, '<ButtonPress>', 
		   sub { &do_search($site); });
	
	$text->insert('end', $_[1], $newtag);
    } else {
	&flush_text(@_);
    }

}

