#!/usr/bin/perl -w 

BEGIN {
        $tab = '';
        $/ = '>';
}

while(<>){
#        print "token '$_'\n";
        
        ($data) = /^([^<]*)/;
        ($tag) = /(<.*>)/;
        if ($data =~ /\w/){
                print "$tab$data\n";
        }
        if (/<\//){
                &tabin();
                print "$tab$tag\n"; 
        } else {
                print "$tab$tag\n"; 
                &tabout();
        }
}

sub tabout {
        $tab .= '.   ';
}
sub tabin {
        $tab =~ s/.   $//;
}
        
