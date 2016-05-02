#!/usr/local/bin/perl -w

#use strict;

############################################################
##  hw1b.prl
##
##  hw1b.prl < segment.data.train
## 
##  Suggestion: run in a wide window
############################################################


my @test_lines = ( );    # the current line that we are testing
my @real_lines = ( );    # the correct classification for the current line
my $numb_lines = 0;      # the number of lines in the block that we testing

my $correct   = 0;       # number of correct classifications
my $incorrect = 0;       # number of incorrect classifications
my @rule_correct = ();
my @rule_incorrect = ();
my $line = undef;
my $seg_class = "";

my $name = "";
my $email = "";
my $phone = "";
my $address = "";

my $degree = "";


my $new_file = 1;  # 1 for new file, 0 for not

my $dir = './resume';
foreach my $fp (glob("$dir/*.pdf")){
	printf "%s\n", $fp;
	my $readfile = "./pdftotext ". $fp . " ". $fp . ".txt";
	system($readfile);

	$fp = $fp . ".txt";
	
	open my $fh, "<", $fp;
	
	
	$new_file = 1;
	@real_lines  = ( );   #  these arrays may hold old data which will

	@test_lines  = ( );   # make sure that we clear memory here or else
	$numb_lines  = 0;     #  screw up calculations of their length

	#while (defined ($line = <$fh>)){
	while ( <$fh>){
		$line = $_;	
		chomp $line;
		if ($_ =~/^$/) {
 			next;
		}
#		print $line, "\n";
		
		&fun_main;

		
	}
	close $fh;
}




sub fun_main{
		my ($str) = ($line =~ /^\s*(\S.*)/);
		my $line_len = 0;
		if (defined $str && $str ne ''){
			$line_len = scalar(split /\s+/, $str);
			
		}

		
		my $det_seg = $line_len>0 && $line_len<5;
		
		
		if ($new_file == 1) {
			
			if((lc $line) !~ /curriculum vitae/ && (lc $line) !~ /resume/ && (lc $line) !~/[0-9]/){
				$new_file = 0;
				$seg_class = "contact";
				$name = $line;
				print $name, "\n";
			}
			
		} 
		elsif ($det_seg && (lc $line ) =~ /education/) {
			&process_segment if ($numb_lines && $seg_class ne "stop");
			
			$seg_class = "education";
			@test_lines  = ( );   # make sure that we clear memory here or else
			$numb_lines  = 0;     #  screw up calculations of their length

		} 
		elsif ($det_seg && (lc $line ) =~ /research/){
			&process_segment if ($numb_lines && $seg_class ne "stop");
			
			$seg_class = "research";
			@test_lines  = ( );   # make sure that we clear memory here or else
			$numb_lines  = 0;     #  screw up calculations of their length
		}
		elsif ($det_seg &&  (lc $line ) =~ /appointment/ ){
			&process_segment if ($numb_lines && $seg_class ne "stop");
			
			$seg_class = "appointment";
			@test_lines  = ( );   # make sure that we clear memory here or else
			$numb_lines  = 0;     #  screw up calculations of their length
		}
		
		elsif ($det_seg &&  (lc $line ) =~/[^contact, research, education, appointment]/){
			
			
			&process_segment if ($numb_lines && $seg_class ne "stop");
			
			$seg_class = "stop";
			
			@test_lines  = ( );   # make sure that we clear memory here or else
			$numb_lines  = 0;     #  screw up calculations of their length
		}
		
		
    	else {
    		
    		($test_lines[ $numb_lines ]) = $line;
			#($real_lines[ $numb_lines ]) = ($line =~ m/^([^\t]+)\t/  );
			$numb_lines += 1;
			
    	}
    	
    	if ($seg_class =~/[^contact, research, education, appointment, stop]/){
    		&process_segment if $numb_lines;
    	}
    	
    	
}


# 
# 
# while (defined ($line = <STDIN>)) {
#     chomp $line;
# 
#     if ($line =~ /^#BLANK#/) {
# 		&process_segment if $numb_lines;     # if we are at a blank line then
# 												 # process the previous block
# 		@test_lines  = ( );   # make sure that we clear memory here or else
# 		@real_lines  = ( );   #  these arrays may hold old data which will
# 		$numb_lines  = 0;     #  screw up calculations of their length
#     }
#     else {
# 		($test_lines[ $numb_lines ]) = ($line =~ m/^[^\t]+\t(.*)/);
# 		($real_lines[ $numb_lines ]) = ($line =~ m/^([^\t]+)\t/  );
# 		$numb_lines += 1;
#     }
# }
# 
# my $util_sum = $correct + $incorrect;
# 
# for ($i = 1; $i < 10; $i++) {
# 	if (!$rule_correct[$i]) {
# 		$rule_correct[$i] = 0;
# 	}
# 	if (!$rule_incorrect[$i]) {
# 		$rule_incorrect[$i] = 0;
# 	}
# 	$sum = $rule_correct[$i] + $rule_incorrect[$i];
# 	if ($sum == 0) { $sum++;} # In case the rule is never used
# 	print "Rule $i utilization: ", $rule_correct[$i] + $rule_incorrect[$i], " = ", ($rule_correct[$i] + $rule_incorrect[$i])*100/$util_sum,  "%  correct: ", $rule_correct[$i], " = ", $rule_correct[$i]*100/$sum, "% incorrect: ", $rule_incorrect[$i], " = ", $rule_incorrect[$i]*100/$sum, "%\n";
# }
# 
# print "### HW1B jguo32 - OVERALL CORRECT: ", $correct, " = ", $correct*100/($correct+$incorrect), "%  INCORRECT: ", $incorrect, " = ", $incorrect*100/($correct + $incorrect),"%.\n";


############################################################
## Process segments by applying specialized detectors/
## classifiers to the array of the text lines in the segment 
############################################################

sub process_segment {

	# We detect each type of text from the most frequently appeared to least appeared
	# E.g. "ADDRESS" only appears 58 times so we detect this type at the end
    if    ( $seg_class eq "contact" ) {
    	&extract_content_contact;
    }
    elsif	 ( $seg_class eq "education" ) {
		&extract_content_education;
	}
	elsif	 ( $seg_class eq "research" ) {
		&extract_content_research;
	}
	elsif	 ( $seg_class eq "appointment" ) {
		&extract_content_appointment;
	}

	else{
		
	}
}




# extract_content
#
#  this function should read through the context of all the text/html
#  documents retrieved by the web robot and extract three types of
#  contact information described in the assignment

sub extract_content_contact {
	
	
	for($cnt = 0; $cnt < scalar @test_lines ; $cnt++) {
		# parse out information you want
    	# print it in the tuple format to the CONTENT and LOG files, for example:
    	
    
    	while($test_lines[$cnt] =~ s/(\w+@\w+\.\w+(\.\w+){0,1})//){
        	$email = $1;
        	print "EMAIL: ", $email, "\n";
    	}
    
    	while($test_lines[$cnt] =~ s/\D((\d{3}){0,1}(\(\d{3}\)){0,1}\D\d{3}\D\d{4})\D//){
    	
        	$phone = $1;
        	print "PHONE: ", $phone, "\n";
    	}
    	while($test_lines[$cnt] =~ s/([0-9]*,{0,5}\s[A-Za-z]+,{0,1}\s[A-Za-z]+,{0,1}\s\d{5}(.\d{4}){0,1})//){
        	$address = $1;
        	print "Address: ", $address, "\n";
    	}
			
	}
     
    return;
}





sub extract_content_education {
	
	
	for($cnt = 0; $cnt < scalar @test_lines ; $cnt++) {
		# parse out information you want
    	# print it in the tuple format to the CONTENT and LOG files, for example:
    
    	
    	if($test_lines[$cnt] =~ /Ph.*D/){
        	$degree = "Ph.D.";
        	print "Degree; ", $degree, "\n";
    	}
    	elsif($test_lines[$cnt] =~ /M.*S/){
    		$degree = "M.S.";
        	print "Degree; ", $degree, "\n";
    	}
    
	}
     
    return;
}




sub extract_content_research {
	for($cnt = 0; $cnt < scalar @test_lines ; $cnt++) {
		# parse out information you want
    	# print it in the tuple format to the CONTENT and LOG files, for example:
    	
    	#print $test_lines[$cnt], "\n";
    
	}

	
	
     
    return;
}

sub extract_content_appointment {
	for($cnt = 0; $cnt < scalar @test_lines ; $cnt++) {
		# parse out information you want
    	# print it in the tuple format to the CONTENT and LOG files, for example:
    	
    	print $test_lines[$cnt], "\n";
    
	}

	
	
     
    return;
}




sub ret {

    my $rule       = shift;   # which rule did I use here
    my $trueclass  = shift;   # what is the true sentence class ('NEOS','EOS')
    my $givenclass = shift;   # what is the sentence class my rule gives
    my $line       = shift;   # the actual line tested

    if ($trueclass eq $givenclass) {

	#printf(".. (%2d) %s\t%s\t%s\n",$rule,$givenclass,$trueclass,$line);
	$correct++;
	$rule_correct[$rule]++;
    }
    else {
	
	printf("XX (%2d) %s\t%s\t%s\n",$rule,$givenclass,$trueclass,$line);
	$incorrect++;
	$rule_incorrect[$rule]++;
    }
}