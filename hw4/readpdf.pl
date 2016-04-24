use PDF::API2;
se CAM::PDF;
use CAM::PDF::PageText;

&main;

sub main {
	my $filename = "cv - eisner.pdf";
	# open my $fh, $filename or die $@;
	# undef $/;
	# my $pdf_string = <$fh>;
	# my $line;
	# while (defined( $line = <$fh> )) {
	# 	print $line, "\n";
	# }
	# print $pdf_string;
	# my $pdf = PDF::API2->open($filename);
	# my $page = $pdf->openpage(1);

	my $pdf = CAM::PDF->new($filename);
	my $pageone_tree = $pdf->getPageContentTree(1);
	print CAM::PDF::PageText->render($pageone_tree);
}
