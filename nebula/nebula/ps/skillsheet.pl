#!/usr/bin/perl -w
#
# Skillsheet.pl
# Produces a rolemaster chartacter skillsheet from a .list file
#
# Output is in postscript. 
#
# Copyright 2000 by Brett Nash <nash@ancalagon.org>
#
use strict;

print "Rolemaster character sheet generation program\n";

if (!defined $ARGV[0]){
    print "usage: $0 <filename.list>\n";
    exit 1;
}



open INFILE,$ARGV[0];
my ($outfile) = $ARGV[0] =~ /^(.*)\.list/;
open OUTFILE,">$outfile.ps";

my $profession = <INFILE>;
chomp $profession;
if ($profession =~ /^#/ or $profession =~ /^\s+/){
	print "Producing Generic Character Sheet\n";
	$profession = "(_)";
} else {
	print "Producing a $profession sheet\n";
}
# Options: #FIXME - should handle letter to!
# page sizes for a4
my ($header) = "";
my ($fontsize) = 9;
my ($Fontsize) = 11; # Used on front page
my ($minx) = 12;
my ($maxx) = 582;
my ($midx) = ($minx + $maxx) / 2;
my ($miny) = 15;
my ($maxy) = 825;
my ($midy) = ($miny + $maxy) / 2;
my ($linespace) = $fontsize;
my ($bonuswidth) = 30;
my ($colwidth) = ($midx - $minx);
my ($linesperpage) = 90; # Calc from fontsize

my ($skillwidth) = 110;
my ($progressionwidth) = 80;
my ($smallwidth) = 22;

my ($topsection) = 40;
my ($leftsection) = 200;
my ($rightsection) = $maxx - $minx - $leftsection;

my ($date);
$date = localtime;

# Proccess the input file
my (@cats);
my (@skills);
my ($maxcat) = -1;
my ($skill) = -1;
my ($thisskill) = 0;
my ($multifollow) = 0;
my (%stats);

while (<INFILE>){
        next if (/^\s*\%/);
        next if (/^\s*$/);
        next if (/^\s*!/);
        if (/^Stat:/){
                my ($stat);
                my (@t);
                chomp;
                (undef,$t[0],$stat,$t[1],$t[2],$t[3],$t[4],$t[5],$t[6]) =  split /\s*:\s*/;
                $stats{$stat}{'name'} = $t[0];
                $stats{$stat}{'temp'} = $t[1];
                $stats{$stat}{'pot'} = $t[2];
                $stats{$stat}{'basic'} = $t[3];
                $stats{$stat}{'racial'} = $t[4];
                $stats{$stat}{'spec'} = $t[5];
                $stats{$stat}{'bonus'} = $t[6];
                next;
        }
        if ($multifollow > 0 && $_ !~ /^\s+\#/){
                while($multifollow --){
                        $skills[$maxcat][$skill ++] = "# _"; 
                }
        } 
        if (/^(\w.*)/){
                if ($skill == 0 && $maxcat != -1){
                        print "Empty category - deleting!\n";
	                print "Removed ",$cats[$maxcat --],"\n";
	        }
	        $maxcat ++;
	        $cats[$maxcat] = $1;
	        $skill = 0;
        } elsif (/\s+(.*)/){
	        $thisskill = $1;
                $skills[$maxcat][$skill ++] = $thisskill;
                if ($thisskill =~ /.+#/){
                        $multifollow = 2;
	        } elsif ($thisskill =~ /^#/){
	                $multifollow --;
                }	
        }
}

# Split into pages nicely
print "Data loaded: There are $maxcat caetgories\n";
my ($line) = 0;
my ($col) = 0;
my (@cols);
my ($currentcat) = 0;
my ($pad) = 0;
while($currentcat < $maxcat){
    print "Processing column $col\n";
    while ($line < $linesperpage){
	print "\tline $line/$linesperpage Trying ",$#{$skills[$currentcat]} + 1 + 8,"\n";
	if (defined $cats[$currentcat] and ($line + 8 + $#{$skills[$currentcat]}) < $linesperpage){
	    $line += 8 + $#{$skills[$currentcat]} + 1;
	    $currentcat ++;
	} else {
	    $pad = $linesperpage - $line;
            if ($col > 0){
	        addpading($pad,$cols[$col -1],$currentcat);
            } else {
                addpading($pad,0,$currentcat);
            }
            $line = $linesperpage;
	}
    }
    $line = 0;
    $cols[$col ++] = $currentcat;

}
# Produce output 
my ($rightside) = 0;
my ($startcat) = 0;
my ($i,$j);
my ($page) = 1;
&init;
print OUTFILE $header;
print OUTFILE "drawoutline\n";
for ($i = 0 ; $i <= $#cols ; $i ++){

    for ($j = $startcat ; $j < $cols[$i] ; $j ++){
	processcat($j);
    }
    if ($rightside){
	print OUTFILE "showpage\n\n";
	$page = int ($i / 2) + 3;
	if (defined $cols[$i + 1]){
	    print OUTFILE "%%Page: $page $page\n";
	    print OUTFILE "drawoutline\n";
	}
    } else{
	print OUTFILE "col2\n";
    }

    $rightside = !$rightside;
    $startcat = $j;
}

print OUTFILE "showpage\n" if ($rightside);
	

printend();

sub addpading {
    my ($nopads,$begin,$end) = @_;
    my ($i,$j) = (0,0); 
    my ($last) = "";
    my ($tmp);

    while($nopads){
	# First add one to any subskills
	for ($i = $begin ; $i < $end ; $i ++){
            $last = "";
	    for ($j = 0 ; $j <= $#{$skills[$i]} ; $j ++){
                $tmp = $skills[$i][$j];
                print "$tmp and lst is $last\n";
	        if ($last =~ /^#/ and $tmp !~ /^#/){
		    splice @{$skills[$i]},$j,0,'# _';
		    $nopads --;
		}
                $last = $tmp;
                return if ($nopads == 0); 
            }
	}
        # Now try adding to end of each cat
        for ($i = $begin ; $i < $end ; $i ++){
            push @{$skills[$i]},"_"; 
            $nopads --;
            return if ($nopads == 0); 
	}

    }
}   


sub processcat {
        my ($cat) = @_;
        my ($statbonus) = 0;
        my ($catbonus,$profbonus);
        my ($cprogress, $sprogress,$rankbonus);
        
        # $cats $skills
        print "Cat is $cats[$cat]\n";
        my ($catname,$stat,$progression,$specprog, $profbonuses,  $cost,$ranks,$specbonus) = $cats[$cat] =~
	m!(.*\w)\s*                      # Name
	    \{\s*([A-Za-z\+\/]+)\s*     # Stat bonus
	    :\s*(\w+)\s*(?:\(\s*([\d\*]*)\))?\s*   # Progression
	    (?::([^}]*))?               # Cat bonuses
            \s*}\s*                     # end of brace
            (?:[[]([^\]]+)[]]\s*)?      # Rank cost
            (?:<(\d+)(?:/([-+]?\d*))?>)?   # Ranks 
            !x;  # [ Rank costs
        $catname =~ s/\*/\267/;
        if (defined $profbonuses){
                $profbonus = 0;
                $profbonuses =~ s/\+(\d+)/$profbonus += $1/eg; 
        } else {
                $profbonus = "0";
        }
        $cost = "_" if (!defined $cost);
        # Work out the stat bonus...
        my $x;
        foreach $x (split /\+/,$stat){
                if (!defined $stats{$x}){
                        print "Stat $x is not found!!\n";
                } else {
                        $statbonus += $stats{$x}{'bonus'};
                }
        }
                
        if ($progression =~ /C/i){         # Combined
                $cprogress = "0 \267 0 \267 0 \267 0 \267 0";
                $sprogress = "-30 \267 5 \267 3 \267 1.5 \267 0.5";
                $ranks = 'N/A';
                $rankbonus = 'N/A';
        } elsif ($progression =~ /[L1l]/){ # Limited
	        $cprogress = "0 \267 0 \267 0 \267 0 \267 0";
        	$sprogress = "0 \267 1 \267 1 \267 0.5 \267 0.5";
	        $ranks = 'N/A';
        	$rankbonus = 'N/A';
        } elsif ($progression =~ /[RrUu]/){ # racial (BD and PPD only)
                $cprogress = "0  \267 0 \267 0 \267 0 \267 0";
                if (defined $specprog){
                        $sprogress = $specprog;
                        $sprogress =~ s/\*/ \267 /g;
                } else {
	                $sprogress = "_";
                }
	        $ranks = 'N/A';
        	$rankbonus = 'N/A';
        } else {                           # Normal
	        $cprogress = "-15 \267 2 \267 1 \267 0.5 \267 0";
        	$sprogress = "-15 \267 3 \267 2 \267 1 \267 0.5";
                if (!defined $ranks or $ranks == 0){
                        $ranks = 0;
                        $rankbonus = -15;
                } elsif ($ranks < 11){
                        $rankbonus = $ranks * 2;
                } elsif ($ranks < 21){
                        $rankbonus = 10 + $ranks; # = (ranks - 10) + 20
                } elsif ($ranks < 31){
                        $rankbonus = POSIX::ceil(30 + ($ranks - 20) * 0.5);
                } else {
                        $rankbonus = 35;
                }         
        }
        if (!defined $specbonus){
                $specbonus = '0';
        }
        if ($rankbonus eq 'N/A'){
                $catbonus = $specbonus + $profbonus + $statbonus;
        } else {
                $catbonus = $specbonus + $profbonus + $rankbonus + $statbonus;
        }
       
        print OUTFILE "($catbonus) ($sprogress) ($specbonus) ($cprogress) ($rankbonus) ($ranks) ($cost)\n";
        print OUTFILE "        ($statbonus) ($stat) ($profbonus) ($catname) drawcat\n";
        my ($name,$class);
        foreach (@{$skills[$cat]}){
                my ($skillbonus) = 0;
                my ($skillranks,$skillspecbonus) = (0,0);
                if (s!<(\d*)(?:/([-+]?\d+))?>!!) {
                        $skillranks = $1;
                        $skillspecbonus = $2 if (defined $2);
                } else {
                        $skillranks = 0;
                }
                $skillranks = 0 if (!defined $skillranks);
                my @progression = split / \267 /,$sprogress;
                if ($skillranks == 0){
                        $skillbonus = $progression[0];
                } else {
                        my ($index) = 1;
                        my ($rankcount) = $skillranks;
                        while($rankcount > 0 ){
                                if ($rankcount > 10){
                                        $skillbonus += $progression[$index] * 10;
                                } else {
                                        $skillbonus += $progression[$index] * $rankcount;
                                }
                                $rankcount -= 10;
                                $index ++;
                        }
                }
                my ($totalbonus) = $skillbonus + $catbonus + $skillspecbonus;
                $skillspecbonus = '_' if ($skillspecbonus == 0);
                if (/^\s*\#\s*(.*)/){
	                $name = "    ".$1;
                        print OUTFILE "($totalbonus) ($skillspecbonus) ($catbonus) (_) ($skillbonus) ($class) ($skillranks) ($name) skill\n";
                } elsif (/\#\s*(\[.\])?$/){
                        ($name) = /^\s*(.*)\#(?:\s*\[(.)\])?/;
                        ($class) = /\[(.)\]/;
                        $class = '_' if (!defined $class);
                        print OUTFILE "($name) skillgroup\n";
        	} else {
                        ($name) = /\s*([^[]*)/;
                        ($class) = /\[(.)\]/;
        	        $class = '_' if (!defined $class);
	                print OUTFILE "($totalbonus) ($skillspecbonus) ($catbonus) (_) ($skillbonus) ($class) ($skillranks) ($name) skill\n";
	        }
        }
}


sub init {
$header = <<ENDOFHEADER
%!PS-Adobe-
%%DocumentFonts: Times-Roman Times-Bolt Times-Italic
%%Title: RMSS Character Sheet
%%Creator: skillsheet.pl by Brett Nash <nash\@nash.nu>
%%CreationDate: $date
%%Pages: (atend)
%%EndComments


%
% Rolemaster character sheet
%
% Copyright 2000 by Brett Nash
% <nash\@ancalagon.org>
%


% Functions
/str { 20 string } def

% Fonts
/getnorm /Times-Roman findfont $fontsize scalefont def
/getbold /Times-Bold findfont $fontsize scalefont def
/getemph /Times-Italic findfont $fontsize scalefont def
/norm { getnorm setfont } def 
/bold { getbold setfont } def
/emph { getemph setfont } def
/getNorm /Times-Roman findfont $Fontsize scalefont def
/getBold /Times-Bold findfont $Fontsize scalefont def
/getEmph /Times-Italic findfont $Fontsize scalefont def
/Norm { getNorm setfont } def 
/Bold { getBold setfont } def
/Emph { getEmph setfont } def

% Lines
/thickline { 2 setlinewidth } def
/thinline  { 1 setlinewidth } def
/reallythinline  { 0.5 setlinewidth } def



% Movement
/linespace 10 def

/upline { 
    $minx 2 add
    currentpoint
    exch pop
    linespace add 
    moveto 
} def

/downline { 
    $minx 2 add
    currentpoint
    exch pop
    linespace sub
    moveto 
    3 $minx add setx
} def



/topleft {
    $minx $maxy moveto
} def

/col2 {
    $midx $minx sub 0 translate
    topleft
} def

/init {
    newpath 
    0 0 moveto
} def

/setx {
    currentpoint exch pop moveto
} def

/drawoutline {
    newpath
    thickline
    $minx $miny moveto
    $minx $maxy lineto
    $maxx $maxy lineto
    $maxx $miny lineto
    closepath
    stroke
    $midx $miny moveto
    $midx $maxy lineto
    stroke
    topleft
} def

% Justify right
% Draw string right from the current point
% (string) justifyright
/justifyright {
    dup stringwidth currentpoint 4 2 roll
    exch neg 4 3 roll
    add  3 1 roll sub
    moveto show
} def

% Justify center
% (string) width justifycenter
/justifycenter {
    exch dup (_) eq 
       { pop gsave reallythinline 6  sub 3 -1 rmoveto 0 rlineto stroke grestore 
       }
       { exch exch dup 3 1 roll 
         stringwidth pop sub 2 div 0 rmoveto show
       }
    ifelse 
}def





%
% drawcat.  Expects to be at the point to draw the category
% Args are:
% (total bonus) (Skill prog) (special bonus) (cat prog) (rank #) 
%             (rank bonus) (cost) (stat bonus) (stat) (prof bonus) (Cat name) drawcat
/drawcat {
    gsave
    $minx setx thickline
    $midx currentpoint exch pop lineto
    stroke
    grestore 

    % Print the cat name
    downline
    bold show
    % move to the middle
    norm
    $midx $bonuswidth sub currentpoint exch pop moveto
    $bonuswidth justifycenter % the profession bonus
    $midx $bonuswidth sub currentpoint exch pop moveto
    (Profession Bonus:) justifyright

        % Stat Line
        downline 
        $colwidth 2.5 div setx show % the stats involved
        $midx $bonuswidth sub setx
        $bonuswidth justifycenter % the stat bonus
        $midx $bonuswidth sub setx
        (Stat Bonus:) justifyright

    downline
    (Rank Cost: ) show
    $bonuswidth justifycenter
    $colwidth 2.5 div setx (# Ranks: ) show
    % rank progress
     $bonuswidth 	justifycenter
    $midx $bonuswidth sub setx
    % rank progress
     $bonuswidth justifycenter
    $midx $bonuswidth sub setx
    (Rank Bonus:) justifyright

    downline
    (Category Progression: ) show /catpos currentpoint pop def
                                           $progressionwidth justifycenter
    $midx $bonuswidth sub setx
     $bonuswidth justifycenter   % Special bonus
    $midx $bonuswidth sub setx
    (Special Bonus:) justifyright

    % The skill Prorgession and Total lines
    % Skill Progression: 0 * 5 * 3 * 3       Total :  86
    downline
    (Skill Progression: ) show catpos setx $progressionwidth justifycenter
    $midx $bonuswidth sub setx
       $bonuswidth justifycenter  % The Total Bonus
    $midx $bonuswidth sub setx
    (Total Bonus:) justifyright
			    
    downline
    $minx setx
    gsave
    thinline
    0 $fontsize  2 sub 2 div rmoveto
    $colwidth 0 rlineto 
    stroke
    grestore	   

    downline
    bold
    (Skill) show
    $skillwidth setx
    (# Ranks) $bonuswidth justifycenter
    $skillwidth $bonuswidth add setx (Class.)  $bonuswidth justifycenter
    
    (Total) (Spec) (Cat) (Item) (Rank)
    $midx $smallwidth 5 mul sub $smallwidth $midx 1 sub {
        currentpoint exch pop moveto $smallwidth justifycenter
    } for

    downline

} def

% Prints a skill
% (tot) (spec) (cat) (item) (rank) (class) (#ranks) (skill) skill
/skill {
        dup (Other) eq { emph } { norm } ifelse
        dup (_) eq 
        { skillline pop }
        { dup (    _) eq {skillline2 pop} {show} ifelse }
        ifelse
        skillwidth setx
        shortline
        skillwidth bonuswidth add setx shortline
    
        midx smallwidth 5 mul sub smallwidth midx 1 sub {
                currentpoint exch pop moveto  shortline 
        } for
        downline    
} def

% Prints a subgrouped skill
% (skill name) skillgroup
/skillgroup {
    emph
    show
    downline
} def

/skillline {
    reallythinline
    gsave
    $minx 3 add setx
    0 -1 rmoveto 
    $skillwidth 3 sub currentpoint exch pop lineto
    stroke
    grestore
} def

/skillline2 {
    reallythinline
    gsave
    (    ) stringwidth pop $minx 3 add add setx 0 -1 rmoveto
    $skillwidth 3 sub currentpoint exch pop lineto
    stroke
    grestore
} def

% lcolumnprint
% Prints a string, followed by an underline in the left column
% (sting) lcolumnprint
/lcolumnprint {
    10 3 add setx 
    show
    gsave
    2 -1 rmoveto
    reallythinline 
    $leftsection 5 sub currentpoint exch pop lineto
    stroke
    grestore
    downline
} def

% Prints a thin line across the width of the column
/lcolumnline {
    $minx setx
    thinline
    gsave 
    $leftsection $minx sub 0 rlineto 
    stroke
    grestore
    downline
}def
/lcolumnthickline {
    $minx setx
    thickline
    gsave 
    $leftsection $minx sub 0 rlineto 
    stroke
    grestore
    downline
} def
/rcolumnline {
    $minx setx
    gsave
    $maxx $leftsection sub 0 rlineto
    stroke
    grestore
    downline
} def
% Prints the columns for a Stat:
% (Total) (Special) (Racial) (basic) (Pot) (Temp) (Abbr) (Stat) printstat
/printstat {
   show                 % Stat
    80 setx shortline   % Abbr
   120 setx shortline   % Temp
   160 setx shortline   % Pot
                               % Gap
   220 setx shortline   % Basic
   260 setx shortline   % Racial
   300 setx shortline   % Special
   340 setx shortline   % total
   downline
} def

% Print attack
% (Notes) (B#) (Str) (F) (Bonus) (Ranks) (Attack/Weapon)
/printattack {
    dup (_) eq
      { pop gsave reallythinline 1 -1 rmoveto 110 0 rlineto stroke grestore }
      { show }
    ifelse
    130 setx shortline % Ranks
    160 setx shortline % Bonus
    200 setx shortline % F
    230 setx shortline % Str
    260 setx shortline % B#
    dup (_) eq
      { pop gsave reallythinline 5 -1 rmoveto 80 0 rlineto stroke grestore }
      { 80 justifycenter }
    ifelse
    downline
} def
% Printfrontequip
/printfrontequip {
    dup (_) eq 
      { pop gsave reallythinline 1 -1 rmoveto 110 0 rlineto stroke grestore }
      { show }
    ifelse 
    130 setx 
    dup (_) eq 
      { pop gsave reallythinline 1 -1 rmoveto 230 0 rlineto stroke grestore }
      { show }
    ifelse  
    downline
} def
 
%  Draw front page
/drawfrontpage {
    newpath
    thickline
    $minx $miny moveto
    $minx $maxy lineto
    $maxx $maxy lineto
    $maxx $miny lineto
    closepath
    stroke
    thickline
    % Border between name and main body
    $minx $maxy $topsection sub moveto $maxx $minx sub 0 rlineto stroke
    % Divide of left column
    newpath
    $leftsection $maxy $topsection sub moveto $leftsection $miny lineto 
    stroke 

} def

/frontskill {
    dup (_) eq
      { pop gsave reallythinline 1 -1 rmoveto 110 0 rlineto stroke grestore }
      { show }
    ifelse
    130 setx shortline
    160 setx shortline
    downline
} def

/shortline {
    dup (_) eq
      { pop gsave reallythinline 1 -1 rmoveto 18 0 rlineto stroke grestore
                         22 0 rmoveto }
      { 20 justifycenter }
    ifelse
} def
%%EndProlog
%%Page: 1 1

%
% Main Page 
% 

init
save
/linespace 12 def
drawfrontpage

newpath
$minx $maxy moveto downline
Bold (Character:) show downline downline
Norm (Campaign (GM):) show
gsave
$maxx $minx add 2 div 10 sub 0 translate  $minx $maxy moveto downline
(Experience:) show
downline downline (Level:) show
$minx $maxy moveto
thickline 0 0 $topsection sub rlineto stroke
grestore

$minx $maxy $topsection sub moveto downline 
Norm
(Race:) lcolumnprint
($profession) (_) eq 
	{ (Proffession:) lcolumnprint }
	{ 13 setx (Proffession: $profession) show downline }
ifelse 
(Realm:) lcolumnprint
(Training Packages:) lcolumnprint
lcolumnline
(Armor Type:) lcolumnprint
(MMP:) lcolumnprint
(Weight Penalty:)  lcolumnprint
(Missile Penalty:) lcolumnprint
lcolumnline
(Quickness Penalty:) lcolumnprint
(Quickness Bonus:) lcolumnprint
(Adrenal Defense:) lcolumnprint
(Shield Bonus:) lcolumnprint
(Other:) lcolumnprint
Bold
(Total DB:) lcolumnprint
lcolumnthickline
3 $minx add setx (Resistance Rolls) show downline
Norm
(Channeling (3xIn):) lcolumnprint
(Essence (3xEm):) lcolumnprint
(Mentalism (3xPr):) lcolumnprint
(Chan/Ess (In+Em):) lcolumnprint
(Chan/Ment (In+Pr):) lcolumnprint
(Ess/Ment (Em+Pr):) lcolumnprint
(Arcane (Em+In+Pr):) lcolumnprint
(Poison (3xCo):) lcolumnprint
(Disease (3xCo):) lcolumnprint
(Fear (3xSD):) lcolumnprint
lcolumnthickline
Bold
(Race/Stat Fixed Info) show downline
Norm
(Soul Departure: ) show (_) shortline ( rounds.) show downline
(Race Type: ) show (_) shortline downline
(Recovery Multiplier: x) show (_) shortline downline
(Body Development Progression) show downline
(     ) lcolumnprint
(PP Development: Progression) show downline
(     ) lcolumnprint reallythinline
(Recover: ) dup stringwidth pop $minx 3 add add exch show
           (_) shortline ( hits/hour resting) show downline
dup setx   (_) shortline ( hits/sleep cycle)  show downline
dup setx   (_) shortline ( PP/hour resting)   show downline
    setx   (_) shortline ( PP/sleep cycle)    show downline
lcolumnthickline
Bold (Role Traits) show Norm downline
(Skin:) lcolumnprint
(Age/Sex:) lcolumnprint
(Height/Weight:) lcolumnprint
(Eyes/Hair:) lcolumnprint
(Personality:) lcolumnprint
(    ) lcolumnprint
(Motivations:) lcolumnprint
(    ) lcolumnprint
(Alignment:) lcolumnprint
lcolumnthickline
Bold (Background Information) show Norm downline
(Nationality:) lcolumnprint
(Home Town:) lcolumnprint
(Religon:) lcolumnprint
(Lord:) lcolumnprint
(Parents:) lcolumnprint
(Spouse:) lcolumnprint
(Children:) lcolumnprint
(Other:) lcolumnprint 
(    ) lcolumnprint

$leftsection $minx sub 0 translate
3 $maxy $topsection sub moveto downline

Bold
(Total) (Special) (Racial) (Basic) ()      ()      ()     ()        printstat
(Bonus) (Bonus)   (Bonus)  (Bonus) (Pot)   (Temp)  (Abbr) (Stat)    printstat
Norm
($stats{'Ag'}{'bonus'}) ($stats{'Ag'}{'spec'})   
        ($stats{'Ag'}{'racial'})  ($stats{'Ag'}{'basic'}) 
        ($stats{'Ag'}{'pot'}) ($stats{'Ag'}{'temp'}) 
        (Ag)   (Agility) 
        printstat
($stats{'Co'}{'bonus'}) ($stats{'Co'}{'spec'})   
        ($stats{'Co'}{'racial'})  ($stats{'Co'}{'basic'}) 
        ($stats{'Co'}{'pot'}) ($stats{'Co'}{'temp'}) 
        (Co)   (Constitution) printstat
($stats{'Me'}{'bonus'}) ($stats{'Me'}{'spec'})   
        ($stats{'Me'}{'racial'})  ($stats{'Me'}{'basic'}) 
        ($stats{'Me'}{'pot'}) ($stats{'Me'}{'temp'}) 
        (Me)   (Memory) printstat
($stats{'Re'}{'bonus'}) ($stats{'Re'}{'spec'})   
        ($stats{'Re'}{'racial'})  ($stats{'Re'}{'basic'}) 
        ($stats{'Re'}{'pot'}) ($stats{'Re'}{'temp'}) 
        (Re)   (Reasoning) printstat
($stats{'SD'}{'bonus'}) ($stats{'SD'}{'spec'})   
        ($stats{'SD'}{'racial'})  ($stats{'SD'}{'basic'}) 
        ($stats{'SD'}{'pot'}) ($stats{'SD'}{'temp'}) 
        (SD)   (Self Discipline) printstat
thinline rcolumnline
($stats{'Em'}{'bonus'}) ($stats{'Em'}{'spec'})   
        ($stats{'Em'}{'racial'})  ($stats{'Em'}{'basic'}) 
        ($stats{'Em'}{'pot'}) ($stats{'Em'}{'temp'}) 
        (Em)   (Empathy) printstat
($stats{'In'}{'bonus'}) ($stats{'In'}{'spec'})   
        ($stats{'In'}{'racial'})  ($stats{'In'}{'basic'}) 
        ($stats{'In'}{'pot'}) ($stats{'In'}{'temp'}) 
        (In)   (Intuition) printstat
($stats{'Pr'}{'bonus'}) ($stats{'Pr'}{'spec'})   
        ($stats{'Pr'}{'racial'})  ($stats{'Pr'}{'basic'}) 
        ($stats{'Pr'}{'pot'}) ($stats{'Pr'}{'temp'}) 
        (Pr)   (Presence) printstat
($stats{'Qu'}{'bonus'}) ($stats{'Qu'}{'spec'})   
        ($stats{'Qu'}{'racial'})  ($stats{'Qu'}{'basic'}) 
        ($stats{'Qu'}{'pot'}) ($stats{'Qu'}{'temp'}) 
        (Qu)   (Quickness) printstat
($stats{'St'}{'bonus'}) ($stats{'St'}{'spec'})   
        ($stats{'St'}{'racial'})  ($stats{'St'}{'basic'}) 
        ($stats{'St'}{'pot'}) ($stats{'St'}{'temp'}) 
        (St)   (Strength) printstat
thinline rcolumnline
(_) (_)   (_)  (_) (_) (_) (Ap)   (Appearance) printstat
($stats{'Ag'}{'bonus'}) ($stats{'Ag'}{'spec'})   
        ($stats{'Ag'}{'racial'})  ($stats{'Ag'}{'basic'}) 
        ($stats{'Ag'}{'pot'}) ($stats{'Ag'}{'temp'}) 
(_) (_)   (_)  (_) (_) (_) (Wl)   (Willpower) printstat
($stats{'Ag'}{'bonus'}) ($stats{'Ag'}{'spec'})   
        ($stats{'Ag'}{'racial'})  ($stats{'Ag'}{'basic'}) 
        ($stats{'Ag'}{'pot'}) ($stats{'Ag'}{'temp'}) 
(_) (_)   (_)  (_) (_) (_) (Sn)   (Sanity) printstat
thickline rcolumnline


$minx setx
Bold (Commonly Used Skills) $rightsection justifycenter Norm  downline

% Col 1
gsave
$rightsection 2 div 0 translate 
$minx 3 add setx
(Bonus) (Ranks) (Skill) frontskill
0 1 14 { (_) (_) (_) frontskill pop } for
grestore
(Bonus) (Ranks) (Skill) frontskill
0 1 14 { (_) (_) (_) frontskill pop } for

thickline rcolumnline

$minx setx
Bold (Commonly Used Attacks) $rightsection justifycenter Norm  downline
Bold (Notes) (B#) (Str) (F) (Bonus) (Ranks) (Attack/Weapon) printattack
Norm
0 1 5 { (_) (_) (_) (_) (_) (_) (_) printattack  pop } for

thickline rcolumnline
$minx setx
Bold (Talents and Flaws) $rightsection justifycenter Norm  downline
Bold (Notes) (Talent/Flaw) printfrontequip
Norm 
0 1 6 { (_) (_)  printfrontequip  pop } for

thickline rcolumnline
upline
gsave $rightsection 3 div       setx currentpoint pop $miny lineto stroke grestore 
gsave $rightsection 3 div 2 mul setx currentpoint pop $miny lineto stroke grestore
downline
Norm (Hits:) show
$rightsection 3 div 3 add setx (Exhaustion Points:) show
$rightsection 3 div 2 mul 3 add setx (Power Points:) show


showpage
grestore 
%%Page: 2 2
/linespace 9 def
ENDOFHEADER
    ;


}

sub printend {
	print OUTFILE <<ENDOFEND
%%Trailer
%%%Pages: $page

ENDOFEND
	;
	close OUTFILE;
}

