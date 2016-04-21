#!/usr/local/bin/perl5 -w
use Tk;
my $mw = MainWindow->new;
$mw->Button(-text => "Hello World!", -command => sub { exit} )->pack;
MainLoop;
