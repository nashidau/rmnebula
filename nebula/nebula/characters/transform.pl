#!/usr/bin/perl -n

if (s/^!//) {
        $hidden = 1;
} else {
        $hidden = 0;
}
        

if (/^(.*){ (.*) : (.*) : (.*) }\s*\[.*\]\s*<\d*\/\d*>/){
        # Skill
        $cat = $1;
        $stats = $2;
        $prog = $3;
        $profbon = $4;
        $cost = $5;
        $ranks = $6;
        $specbon = $7;
        
        if ($group == 1){
                print "\t\t</skillgroup>\n";
                $group = 0;
        }

        $cat =~ s/\*/&#xb7;/;
        $cat =~ s/\s+$//;
        if ($prog eq 'S'){
                $progress = 'Standard';
        } elsif ($prog eq 'C'){
                $progress = 'Combined';
        } elsif ($prog eq 'L'){
                $progress = 'Limited';
        } else {
                $progress = 'FIXME - Unknown!!\n';
        }
        
        print "\t</category>\n";
        
        print "\t<category>\n";
        
        print "\t\t<name>$cat</name>\n";
        print "\t\t<statbonuses>$stats</statbonuses>\n";

        print "\t\t<progression>$progress</progression>\n";
        print "\t\t<cost>$cost</cost>\n";
        print "\t\t<professionbonus>$profbonus</professionbonus>\n";
        print "\t\t<statbonus>0</statbonus>\n";
        print "\t\t<specialbonus>$specbon</specialbonus>\n";
        print "\t\t<ranks>$ranks</ranks>\n";
        print "\t\t<rankbonus>0</rankbonus>\n";
        print "\t\t<total>111</total>\n";

        if ($hidden){
                print "\t\t<hidden />\n";
        } 
        
        
} elsif (/^\t(.*)#\s*$/){
        # Skill groups
        if ($group == 1){
                print "\t\t</skillgroup>\n";
                $group = 0;
        }
        $skill = $1;
        $skill =~ s/\s*<(\d+)(?:\/(\+?\d+))?>//;
        $ranks = $1;
        $specbonus = $2;
        $skill =~ s/\s*\[(.)\]//;
        $prog = $1;
        if ($prog eq 'E'){
                $prog = 'Everyman';
        } elsif ($prog eq 'O'){
                $prog = 'Occupational';
        } elsif ($prog eq 'R'){
                $prog = 'Restricted';
        } else {
                $prog = '';
        }

        print "\t\t<skillgroup>\n";
        print "\t\t\t<name>$skill</name>\n";
        print "\t\t\t<class>$prog</class>\n";
        print "\t\t\t<specialbonus>$specbonus</specialbonus>\n";        
        
        if ($hidden){
                print "\t\t\t<hidden />\n";
        } 
        $group = 1;
} elsif (/^\t(.*)/){
        # Skills
        $skill = $1;
        $skill =~ s/\s*$//;
        $skill =~ s/\s*<(\d+)(?:\/(\+?\d+))?>//;
        $ranks = $1;
        $specbonus = $2;
        $skill =~ s/\s*\[(.)\]//;
        $prog = $1;
        if ($prog eq 'E'){
                $prog = 'Everyman';
        } elsif ($prog eq 'O'){
                $prog = 'Occupational';
        } elsif ($prog eq 'R'){
                $prog = 'Restricted';
        } else {
                $prog = '';
        }
        if ($skill =~ s/^#\s*//){
                $t = "\t";
        } else {
                if ($group == 1){
                        print "\t\t</skillgroup>\n";
                        $group = 0;
                }
                $t = '';
        }
                
        
        print "$t\t\t<skill>\n";
        print "$t\t\t\t<name>$skill</name>\n";
        print "$t\t\t\t<ranks>$ranks</ranks>\n";
        print "$t\t\t\t<class>$prog</class>\n";
        print "$t\t\t\t<rankbonus>0</rankbonus>\n";
        print "$t\t\t\t<itembonus>0</itembonus>\n";
        print "$t\t\t\t<categorybonus>0</categorybonus>\n";
        print "$t\t\t\t<specialbonus>$specbonus</specialbonus>\n";
        print "$t\t\t\t<total>222</total>\n";
        if ($hidden){
                print "$t\t\t\t<hidden />\n";
        } 
        print "$t\t\t</skill>\n";
        
}

