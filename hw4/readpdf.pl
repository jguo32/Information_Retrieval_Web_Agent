# use PDF::API2;
use CAM::PDF;
# use CAM::PDF::PageText;

&main;

sub main {
	$filename = "420distributions2.pdf";
	my $pdf = CAM::PDF->new($filename);
	my $pageone_tree = $pdf->getPageContentTree(1);
	print CAM::PDF::PageText->render($pageone_tree);
}
