#!/usr/local/bin/perl5 -w
#############################################################################
## Webping: A program that will detect and report whether a web site is up.
## usage: webping [ -a ] [ -i <minutes>] [ -f <filename> ] [-- [ -geometry...]]
##   -a : starts prog in "autoping" mode from beginning.
##   -i : Sets the autoping interval to <int>
##   -f : Uses <filename> instead of .webping_sites as site list
##   -- is necessary to separate webping's options from the Window
##      Manager options.  Allows us to utilize GetOptions instead of parsing
##      them manually (ick).
##   The standard wm specs are allowed after the --, -geometry and -iconic
##   being the most useful of them.
#############################################################################

use Tk;
use LWP::Simple;
use Getopt::Long;

## DEFAULT values -- may be changed by specifing cmd line options.
my $site_file = "$ENV{HOME}/.webping_sites";
$ping_interval = 5;
$auto_mode = 0;
@intervals = (5, 10, 15, 20, 25, 30, 35);

sub numerically { $a <=> $b; }
sub antinumerically { $b <=> $a; }
		  
## Parse our specific command line options first
&GetOptions("i=i" => \$ping_interval,
  "f=s" => \$site_file,
	         "a" => \$auto_mode);

if (! grep /$ping_interval/, @intervals) {
    push (@intervals, $ping_interval);
}
my $mw = MainWindow->new;
$mw->title("Web Ping");
$mw->CmdLine;  ## parse -geometry and etc cmd line options.

$frame1 = $mw->Frame;
$frame1->pack(side => "bottom", -anchor => "n",
	      -expand => "n", -fill => "x");

## Create frame for buttons along the bottom of the window
my $button_f = $frame1->Frame(-borderwidth => 2,
			      -relief => "ridge");
$button_f->pack(-side => "top", -anchor => "n",
		-expand => "n",	-fill => "x");

$update_bttn = $button_f->Button(-text => "Update",
				 -state => 'disabled',
				 -command => sub { &end_automode;
						   &ping_site });
$update_bttn->pack(-side => "left", -anchor => "w", -padx => 5);

$del_bttn = $button_f->Button(-text => "Delete",
			      -state => 'disabled',
			      -command => sub { &delete_site });
$del_bttn->pack(-side => "left",
		-anchor => 'w',
		-padx => 10);

$automode_bttn = $button_f->Button(-text => "Start Automode",
				   -command => \&do_automode);
$automode_bttn->pack(-side => 'left');

$button_f->Label(-text => "Interval: ")->pack(-side => "left");

## Create a psuedo pop-up menu using Menubutton
$interval_mb = $button_f->Menubutton(-indicatoron => 1,
			    -borderwidth => 2,
			    -relief => "raised");
$interval_mb->pack(-side => "left");

$interval_mb->configure(-menu => $interval_mb->Menu(-tearoff => 0),
	       -textvariable => \$ping_interval);
map { $interval_mb->radiobutton(-label => $_,
		       -variable => \$ping_interval,
		       -value => $_,
		       -indicatoron => 0) } sort numerically
@intervals;


$button_f->Button(-text => "Exit",
		  -command => \&exit_program)->pack(-side => "right",
						    -anchor => "e");

my $entry_f = $mw->Frame;
$entry_f->pack(-side => 'top', -anchor => 'n', -fill => 'x');

$entry_f->Label(-text => "URL: ")->pack(-side => 'left',
					-anchor => 'w');
my $entry = $entry_f->Entry(-textvariable => \$url);
$entry->pack(-side => 'left', -anchor => 'w', -expand => 'y', -fill =>
'x');


$entry_f->Button(-text => "Ping",
	       -command => \&add_site)->pack(-side => 'left',
					     -anchor => 'e');
$entry->bind('<Return>', \&add_site);

my $list_f = $mw->Frame;
$list_f->pack(-side => 'top',
	      -anchor => 'n',
	      -expand => 'yes',
	      -fill => 'both');
$history_label = $list_f->Button(-text => "History:",
				 -borderwidth => 2,
				 -relief => "flat");
$history_label->pack(-side => 'top', -anchor => 'n', -fill => 'x');

my $scroll = $list_f->Scrollbar;
my $list = $list_f->Listbox(-selectmode => 'extended',
			    -yscrollcommand => ['set', $scroll]);
$scroll->configure(-command => ['yview', $list]);
$scroll->pack(-side => 'right', -fill => 'y');
$list->pack(-side => 'left', -expand => 'yes', -fill => 'both');

## Bind Listbox so that the "Update" button is enabled whenever a user
## has an item selected.
$list->bind('<Button-1>', sub { 
    my @selected = $list->curselection;
    if ($#selected >= 0) {
	$update_bttn->configure(-state => 'normal');
	$del_bttn->configure(-state => 'normal');
    } else {
	$update_bttn->configure(-state => 'disabled');
	$del_bttn->configure(-state => 'disabled');
    }
} );

if (open(FH, "$site_file")) {
    while (<FH>) {
	chomp;
	$url = $_;
	&add_site;
    }
    close FH;
}
$url = "";

$entry->focus;

&do_automode if ($auto_mode);

MainLoop;

sub exit_program {
    my @updated = $list->get(0, 'end');
    if (open FH, ">$site_file") {
	map { print FH "$_\n"; } @updated;
	close FH;
    }
    exit;
}
sub ping_site {
  ## get list of indexes in listbox of those selected.
  my $site = "";
  my ($content, @down);
  my @selected = $list->curselection;
  
  $mw->configure(-cursor => 'watch'); 
  $mw->idletasks;

  foreach $index (@selected) {
      my $site = $list->get($index);
      $site =~ s/\s.+$//;         ## Strip off last history record (if any)
      
      $content = head($site);
      if ($content) {
	  $site .= " is UP (" . localtime() .")";
      } else {
	  $site .= " is DOWN (" . localtime() .")";
	  push (@down, $site);
      }
      $list->delete($index);
      $list->insert($index, $site);
  }
  
  ## Since we've deleted and inserted into the box -- the sites prev
  ## selected now aren't. Luckily we know which ones those were.
  map { $list->selection('set', $_) } @selected;
  
  ## Set cursor back to the default value
  $mw->configure(-cursor => 'top_left_arrow'); 
  
  if ($#down >= 0) {
      $mw->deiconify;
      $mw->update;
      
      $old_color = $history_label->cget(-background);
      
      ## Do some stuff to make the user pay attention to us.
      $history_label->configure(-background => "red");
      $history_label->bell;
      $history_label->flash;       $history_label->flash;
      $history_label->configure(-background => $old_color);
  }

}
sub add_site {
    return if ($url eq "");                ## Do nothing, empty string
   
    ## Validate $url contains correct information (ie a server name)
    $url = "http://$url" if ($url !~ /(\w+):\/\//);
    
    ## Insert the new site name into the list, and make sure we can see it.
    $list->insert('end', $url);
    $list->see('end');
    
    ## Select the item so that our function ping_site can do all the work
    $list->selection('clear', 0, 'end');
    $list->selection('set', $list->size - 1);  
    
    $url = "";   ## Clear out string for next site

    &ping_site;
}
sub delete_site {
    my @selected = $list->curselection;

    ## Have to delete the items out of list backwards so that the indexes
    ## we just retrieved remain valid until we're done.
    map { $list->delete($_) } sort antinumerically @selected;

    $update_bttn->configure(-state => 'disabled');
    $del_bttn->configure(-state => 'disabled');
}
sub do_automode {
    ## State if the $automode_bttn will tell us which way we are in.
    my $state = $automode_bttn->cget(-text);

    if ($state =~ /^Start/) {
	$automode_bttn->configure(-text => "End Automode");

	$mw->iconify if ($auto_mode);
	
	$interval_mb->configure(-state => 'disabled');
	
	## If the user started up from scratch -- then select all (doesn't
	## make sense to ping _nothing_.
	@selected = $list->curselection;
	$list->selection('set', 0, 'end') if ($#selected < 0);
	$id = $mw->repeat($ping_interval * 60000, \&ping_site);
    } else {
	&end_automode;
    }
}
## end of do_automode
#########################################################
sub end_automode {	
my $state = $automode_bttn->cget(-text);
   $interval_mb->configure(-state => 'normal');
   if ($state =~ /^End/) {
$automode_bttn->configure(-text => "Start Automode");
	   $mw->after('cancel', $id);
   }
}

