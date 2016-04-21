#!/usr/local/bin/perl5 -w
use strict;

use HTML::FormatText;
use HTML::Parse;
use Tk;

my $query_interval = 30; # in minutes

my $email = "<your email\@address here>";
my $url = "http://www.fedex.com/cgi-bin/track_it";

my $mw = MainWindow->new;
$mw->title("Package Tracker");
$mw->CmdLine;
my @destinations = 
  ("U.S.A.", "ALBANIA", "ALGERIA", "AMERICAN SAMOA ",
   "ANDORRA", "ANGOLA", "ANGUILLA", "ANTIGUA", "ARGENTINA", 
   "ARMENIA", "ARUBA", "AUSTRALIA", "AUSTRIA", "AZERBAIJAN", 
   "BAHAMAS", "BAHRAIN", "BANGLADESH", "BARBADOS", "BELARUS",
   "BELGIUM", "BELIZE", "BENIN", "BERMUDA", "BHUTAN",
   "BOLIVIA", "BOTSWANA", "BRAZIL", "BRITISH VIRGIN IS.", 
   "BRUNEI", "BULGARIA", "BURKINO FASO", "BURUNDI", "CAMBODIA",
   "CAMEROON", "CANADA", "CAPE VERDE", "CAYMAN ISLANDS", 
   "CENTRAL AFRICAN REP.", "CHAD", "CHILE", "CHINA", 
   "COLOMBIA", "CONGO", "COOK ISLANDS",
   "COSTA RICA", "COTE D'IVOIRE", "CROATIA", "CYPRUS", "CZECH
REPUBLIC", 
   "DENMARK", "DJIBOUTI", "DOMINICA", "DOMINICAN REPUBLIC", "ECUADOR",
  "EGYPT", "EL SALVADOR", "EQUATORIAL GUINEA", "ERITREA", "ESTONIA", 
   "ETHIOPIA", "FAEROE ISLANDS", "FIJI",
   "FINLAND", "FRANCE", "FRENCH GUIANA", "FRENCH POLYNESIA", "GABON",
   "GAMBIA", "GEORGIA, REPUBLIC OF", "GERMANY", "GHANA", "GIBRALTAR",
  "GREECE", "GREENLAND", "GRENADA",
   "GUADELOUPE", "GUAM", "GUATEMALA", "GUINEA  ",
   "GUINEA-BISSAU", "GUYANA", "HAITI", "HONDURAS", "HONG KONG",
   "HUNGARY", "ICELAND", "INDIA", "INDONESIA", "IRELAND", "ISRAEL",
"ITALY",
   "JAMAICA", "JAPAN", "JORDAN", "KAZAKHSTAN", "KENYA", "KUWAIT", 
   "KYRGYZSTAN",
   "LATVIA", "LEBANON", "LESOTHO", "LIBERIA", "LIECHTENSTEIN",
"LITHUANIA",
   "LUXEMBOURG", "MACAU", "MACEDONIA", "MADAGASCAR", "MALAWI",
"MALAYSIA",
   "MALDIVES", "MALI", "MALTA", "MARSHALL ISLANDS", "MARTINIQUE",
   "MAURITANIA", "MAURITIUS", "MEXICO", "MICRONESIA", "MOLDOVA",
"MONACO", 
   "MONGOLIA",
   "MONTSERRAT", "MOROCCO", "MOZAMBIQUE", "NAMIBIA", "NEPAL",
   "NETHERLANDS", "NEW CALEDONIA", "NEW ZEALAND", "NICARAGUA",
"NIGER",
   "NIGERIA", "NETHERLANDS ANTILLES", "NORWAY", "OMAN", "PAKISTAN",
"PALAU",
   "PANAMA", "PAPUA NEW GUINEA", "PARAGUAY", "PERU", "PHILIPPINES",
   "POLAND", "PORTUGAL", "QATAR", "REUNION ISLAND", "ROMANIA",
   "RUSSIA", "RWANDA", "SAIPAN", "SAN MARINO", "SAUDI ARABIA",
"SENEGAL",
   "SEYCHELLES", "SIERRA LEONE", "SINGAPORE", "SLOVAK REPUBLIC",
"SLOVENIA",
   "SOUTH AFRICA", "SOUTH KOREA", "SPAIN", "SRI LANKA", "ST. KITTS &
NEVIS",
   "ST. LUCIA", "ST. VINCENT", "SUDAN", "SURINAME",
   "SWEDEN", "SWAZILAND", "SWITZERLAND", "SYRIA", "TAIWAN",
   "TANZANIA", "THAILAND", "TOGO", "TRINIDAD & TOBAGO", "TUNISIA",
"TURKEY",
   "TURKMENISTAN, REPUBLIC OF", "TURKS & CAICOS IS.", "U.A.E.",
"UGANDA", 
   "UKRAINE",
   "UNITED KINGDOM", "URUGUAY", "U.S. VIRGIN ISLANDS", 
   "UZBEKISTAN", "VANUATU", "VATICAN CITY", "VENEZUELA", "VIETNAM",
   "WALLIS & FUTUNA ISLANDS", "YEMEN", "ZAIRE", "ZAMBIA", "ZIMBABWE");

my $entry_f = $mw->Frame;
$entry_f->pack(-expand => 'n', -fill => 'x');
$entry_f->Label(-text => "Airbill #: ")->pack(-side => 'left',
					    -anchor => 'w',
					    -expand => 'n',
					    -fill => 'none');
my $airbill = "";
my $airbill_entry = $entry_f->Entry(-textvariable => \$airbill,
				    -width => 10);
$airbill_entry->pack(-side => 'left',
		     -anchor => 'w',
		     -expand => 'y',
		     -fill => 'x');
$entry_f->Label(-text => "Date Shipped: ")->pack(-side => 'left',
					       -anchor => 'w',
					       -expand => 'n',
					       -fill => 'none');

my %months;

my $i = 1;
foreach (qw(Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec)) {
  $months{$_} = $i++;
}

my $fulltime = localtime;

my ($month, $day, $year) = $fulltime =~
  /\w+\s(\w+)\s(\d+)\s..:..:..\s..(\d\d)$/;

$month = $months{$month};
$month = "0$month" if (length($month) < 2);
$day = "0$day" if (length($day) < 2);

my $date = "$month$day$year";

$entry_f->Entry(-textvariable => \$date,
		-width => 6)->pack(-side => 'left',
				   -anchor => 'w',
				   -expand => 'n',
				   -fill => 'none');
my $lb_f = $mw->Frame;
$lb_f->pack(-anchor => 'n',
	    -expand => 'n',
	    -fill => 'x');
$lb_f->Label(-text => "Shipped To:")->pack(-side => 'left',
					   -anchor => 'w');

my $scroll = $lb_f->Scrollbar;
my $listbox = $lb_f->Listbox(-selectmode => 'single',
			  -height => 1,
			  -yscrollcommand => ['set', $scroll],
			  -exportselection => 0);
$scroll->configure(-command => ['yview', $listbox]);
$scroll->pack(-side => 'right', -fill => 'y');
$listbox->pack(-side => 'left', -expand => 'yes', -fill => 'both');

$listbox->insert('end', @destinations); 
$listbox->selection('set',0);

my $response_f = $mw->Frame;
$response_f->pack(-expand => 'y', -fill => 'both');

$response_f->Label(-text => "Response:")->pack(-anchor => 'w',
					       -side => 'left');

my $response_txt = "";
$response_f->Label(-justify => 'left', -borderwidth => 2, -relief =>
'sunken',
		   -textvariable => \$response_txt)->pack(-anchor =>
'w',
							  -side =>
'left',
							  -expand =>
'y',
							  -fill =>
'x');

my $bttn_f = $mw->Frame;
$bttn_f->pack;

$bttn_f->Button(-text => "Exit", -command => sub{exit})->pack(-side =>
'right',
							   -anchor =>
'e');

my $loop_bttn = $bttn_f->Button(-text => "Loop", -command =>
\&loop_query);
$loop_bttn->pack(-side => 'left', -anchor => 'w');

$bttn_f->Button(-text => "Query", -command => \&do_query)->
  pack(-side => 'left',
       -anchor => 'w');


my $pkg_tracker = new FedEx $url, $email;
my $loop_id;

$airbill_entry->focus;

MainLoop;


sub loop_query {
    my $bttn_text = $loop_bttn->cget(-text);
    if ($bttn_text =~ /^Loop/) {
	&do_query;
	$loop_bttn->configure(-text => "Stop");
	$loop_id = $mw->repeat($query_interval * 60000, \&do_query);
    } else {
	$loop_bttn->configure(-text => "Loop");
	$mw->after('cancel', $loop_id);
    }
}
sub do_query { 
    $mw->configure(-cursor => 'watch');
    $mw->idletasks;
    
    my $dest = $listbox->get($listbox->curselection);
    
    $pkg_tracker->check($airbill, $dest, $date);
    
    if ($pkg_tracker->retrieve_okay) {
	
	if ($pkg_tracker->delivered) {
	    $response_txt = "Tracking number $airbill was delivered
to: " . 
	      $pkg_tracker->who_got_it;
	} else {
	    $response_txt = "Package not yet delivered";
	}
    } else {
	my $parsed = parse_html($pkg_tracker->error_info);
	my $converter = new HTML::FormatText;
	$response_txt = $converter->format($parsed);
	chomp($response_txt);
    }
    
    $response_txt .= "\n[As of " . localtime() . "]";
    $mw->configure(-cursor => 'top_left_arrow');
    $mw->deiconify;
 $mw->bell;
    $mw->update;
}
## Package FedEx Written by Clinton Wong
package FedEx;

use HTTP::Request;
use HTTP::Response;
use LWP::RobotUA;
use HTTP::Status;

sub new {

  my($class, $cgi_url, $email, $proxy) = @_;
  my $user_agent_name = 'ORA-Check-FedEx/1.0';

  my $self  = {};
  bless $self, $class;

  $self->{'url'} = new URI::URL $cgi_url;

  $self->{'robot'} = new LWP::RobotUA $user_agent_name, $email; 
  $self->{'robot'}->delay(0);    # we'll delay requests by hand

  if ($proxy) {
    $self->{'robot'}->proxy('http', $proxy);
  }

  $self;
}

sub check {
    
    my ($self, $track_num, $country, $date) = @_;
    
    $self->{'url'}->query("trk_num=$track_num&dest_cntry=" .
			  "$country&ship_date=$date");
    my $request = new HTTP::Request 'GET', $self->{'url'};
    
    my $response = $self->{'robot'}->request($request);
    $self->{'status'} = $response->code();
    
    if ($response->code == RC_OK) {
	
	if ($response->content =~ /Delivered To : (\w.*)/) {
	    
	    # package delivered
	    $self->{'who_got_it'} = $1;
	    $self->{'delivered'} = 1;
	} 
	
	# Odd cases when package is delivered but "Delivered To" is blank.
	# Check for delivery time instead.
	
	elsif ($response->content =~ /Delivery Time : \w.*/) {
	    
	    # package delivered
	    $self->{'who_got_it'} = 'left blank by FedEx computer';
	    $self->{'delivered'} = 1;
	}
	else {
	    
	    # package wasn't delivered
	    $self->{'delivered'} = 0;
	    
	    # if there isn't a "Delivered To : " field, something's wrong.
	    # error messages seen between HTML comments
	    
	    if ($response->content !~ /Delivered To : /) {
		$self->{'status'} = RC_BAD_REQUEST;
		
		# get explanation from HTML response
		my $START = '<!-- BEGIN TRACKING INFORMATION -->';
		my $END = '<!-- END TRACKING INFORMATION -->';
		if ($response->content =~ /$START(.*?)$END/s) {
		    $self->{'error_as_HTML'} = $1;
		} 
		else {
		    # couldn't get explanation, use generic one
		    $self->{'error_as_HTML'} = 'Unexpected HTML
response from FedEx'; 
		    
		}    # couldn't get error explanation
	    }      # unexpected reply
	}        # not delivered yet
    }          # if HTTP response of RC_OK (200)
    else {
	$self->{'error_as_HTML'} = $response->error_as_HTML;
    }
    
}

sub retrieve_okay {
    my $self = shift;
    return 0 if ($self->{'status'} != RC_OK);
    1;
}

sub delivered {
    my $self = shift;
    $self->{'delivered'};
}

sub who_got_it {
    my $self = shift;
    $self->{'who_got_it'};
}

sub error_info {
    my $self = shift;
    $self->{'error_as_HTML'};
}


