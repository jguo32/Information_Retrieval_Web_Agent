
# Given a full URL, return the scheme, hostname, port, and path
# into ($scheme, $hostname, $port, $path).  We'll only deal with
# HTTP URLs.

sub parse_URL {

  # put URL into variable
  my ($URL) = @_;

  # attempt to parse.  Return undef if it didn't parse.
  (my @parsed =$URL =~ m@(\w+)://([^/:]+)(:\d*)?([^#]*)@) || return undef;

  # remove colon from port number, even if it wasn't specified in the URL
  if (defined $parsed[2]) {
    $parsed[2]=~ s/^://;
  }

  # the path is "/" if one wasn't specified
  $parsed[3]='/' if ($parsed[0]=~/http/i && (length $parsed[3])==0);

  # if port number was specified, we're done
  return @parsed if (defined $parsed[2]);

  # otherwise, assume port 80, and then we're done.
  $parsed[2] = 80;

  @parsed;
}

# grab_urls($html_content, %tags) returns an array of links that are
# referenced from within html. 


sub grab_urls {

  my($data, %tags) = @_;
  my @urls;

  # while there are HTML tags
  skip_others: while ($data =~ s/<([^>]*)>//)  {

    my $in_brackets=$1;
    my $key;

    foreach $key (keys %tags) {

      if ($in_brackets =~ /^\s*$key\s+/i) {     # if tag matches, try parms
        if ($in_brackets =~ /\s+$tags{$key}\s*=\s*"([^"]*)"/i) {
          my $link=$1;
          $link =~ s/[\n\r]//g;  # kill newlines,returns anywhere in url
          push (@urls, $link);
          next skip_others;
        } 
        # handle case when url isn't in quotes (ie: <a href=thing>)
        elsif ($in_brackets =~ /\s+$tags{$key}\s*=\s*([^\s]+)/i) {
          my $link=$1;
          $link =~ s/[\n\r]//g;  # kill newlines,returns anywhere in url
          push (@urls, $link);
          next skip_others;
        }    
      }       # if tag matches
    }         # foreach tag
  }           # while there are brackets
  @urls;
}


1;

