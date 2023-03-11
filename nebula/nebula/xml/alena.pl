#!/usr/bin/perl -w 
# Alena
#
# Produces a B5W control sheet from an XML source
#
use strict;
use XML::Parser;
 
my $debug =1;
 
my $p1 = new XML::Parser(Style => 'Tree');
my $parse = $p1->parsefile('Avioki.xml');

open IN,"<","shell.ps";
open OUT,">","avioki.ps";
 
my @in = <IN>;
print OUT @in;
 
 
my $ship = &getitem('b5wship',$parse);
my $datacard = &getitem('datacard',$ship);
my $structure = &getitem('controlcard',$ship);
my @items;
push @items,&getitems('item',&getitem('forward',$structure));
push @items,&getitems('item',&getitem('port',$structure));
push @items,&getitems('item',&getitem('starboard',$structure));
push @items,&getitems('item',&getitem('aft',$structure));
push @items,&getitems('item',&getitem('primary',$structure));
 
# Print the Ship name/title
my $race = &getitem('race',$datacard);
my $hull = &getitem('hull',$datacard);
my $variant = &getitem('variant',$datacard);
# Fixme - this just gets the first version!
my $availability = &getitem('rarity',&getitem('availability',$datacard));

my $psname = "";
my $psvariant = "";
if (${$variant}[2] =~ /[a-z]/){
        $psname = "${$race}[2] ${$variant}[2]";
        $psvariant = "${$availability}[2] ${$hull}[2] variant";
} else {
        $psname = "${$race}[2] ${$hull}[2]";
        $psvariant = "${$availability}[2] base hull";
}

print "Printing title\n";
print OUT "($psname) ($psvariant) title\n";
 
 
###
# Spec windows
###
my $specs = &getitem('specs',$datacard);
my $maneuvering = &getitem('maneuvering',$datacard);
my $combatstats = &getitem('combatstats',$datacard);
my $specialnotes = &getitem('specialNotes',$datacard);
print OUT "startshipinfo\n";
&shipinfo($specs);
&shipinfo($maneuvering);
&shipinfo($combatstats);
&shipinfo($specialnotes);

###
# To Hit chart 
###
my $forwardhits = &getitem('hits',&getitem('forward',$structure));
my $sidehits = &getitem('hits',&getitem('port',$structure));
my $afthits = &getitem('hits',&getitem('aft',$structure));
my $primaryhits = &getitem('hits',&getitem('primary',$structure));
print OUT "starthitboxes\n";
&printhitchart("Forward",$forwardhits);
&printhitchart("Side",$sidehits);
&printhitchart("Aft",$afthits);
&printhitchart("Primary",$primaryhits);


###
# Ship Control sheet itself - icons et al
###

my $item;
my $count;
# Print the ships items
foreach $item (@items){
	printpsitem($item);
}

sub printpsitem {
	my ($item) = @_;
	# Need X and Y
	my $x = ${&getitem('x',&getitem('position',$item))}[2];
	my $y = ${&getitem('y',&getitem('position',$item))}[2];
 	my $direction = &getitem('direction',&getitem('position',$item));
	if (defined $direction){
		$direction = ${$direction}[2];
	}
	my $type = ${&getitem('type',$item)}[2];
	my $structure = ${&getitem('structure',$item)}[2];
	my $armor = ${&getitem('armor',$item)}[2];
        my $arc = &getitem('arc',$item);
        my ($arcbegin,$arcend);
        if (defined $arc){
                ($arcbegin,$arcend) = split /-/,$$arc[2];
        }
	my $power = &getitem('power',$item);
	if (defined $power){
		$power = $$power[2];
		$armor = "$armor $power armorpower";
	} else {
		$armor = "$armor armor";
	}
	my $rating = &getitem('rating',$item);
	if (defined $rating){
		$rating = $$rating[2];
	} else {
		$rating = 0;
	}
	my @structs = split /\+/,$structure;
	my ($maxX,$maxY) = (0,0);
	foreach (@structs){
		my ($ac,$up) = split /\*/;
		$maxX = $ac if (defined $ac and $ac > $maxX);
		$up = 1 if (!defined $up);
		$maxY += $up;
	}
	
	debug("printing $type\n");
 	print OUT "\n% $type\n";	
	print OUT "$x mm $y mm topleft moveto\n";
	print OUT "$direction " if (defined $direction);
	print OUT "$armor\n";
	if ($type !~ /Structure/){
		$type =~ s/ //g;
		$type =~ s/(Port|Aft|Main|Starboard|Primary|Retro)//;
		print OUT "$rating $maxX $maxY $type\n";
	}
	foreach (@structs){
		my ($ac,$up) = split /\*/;
		$up = 1 if (!defined $up);
		$ac = 1 if (!defined $ac);
		print OUT "$ac $up boxes\n";
	}
        if (defined $arc){
                # move ot the correct position for the arc
                print OUT "rmoveto\n";
                print OUT "$arcbegin $arcend hex-group\n";
        }
	print OUT " grestore\n" if (defined $direction);
}
		
	
		

sub getitem {
	my ($item,$tree) = @_;
	my $i;
	for ($i = 0 ; $i <= $#$tree ; $i ++){
		if ($$tree[$i] eq $item){
			return $$tree[$i + 1];
		}
	}
	return undef;
}
sub getitems {
	my ($item,$tree) = @_;
	my $i;
	my @results;
	for ($i = 0 ; $i <= $#$tree ; $i ++){
		if ($$tree[$i] eq $item){
			push @results, $$tree[$i + 1]
		}
	}
	return @results;
}
 
sub debug {
	print @_ if ($debug);
}
sub toenglish {
        my $x = $_[0];
        $x =~ s/([A-Z])/ $1/g;
        $x =~ s/^(.)/\u$1/;
        return $x;
}
sub shipinfo {        
        my ($specs) = @_;
        my $spec;
        my $i;
        print OUT "resetshipinfo\n";
        for ($i = 3 ; $i < $#$specs ; $i += 4){
                $spec = $$specs[$i];
                $spec = &toenglish($spec);
                print OUT "($spec) (${$$specs[$i + 1]}[2]) shipinfo\n";
        } 
        print OUT "shipinfobox\n";
}
sub printhitchart {
        my ($side, $stats) = @_;
        my $i;
        my ($range,$item);

        print OUT "($side) starthitbox\n";
        for ($i = 4 ; $i < $#$stats ; $i += 4){
                $range = &getitem('range',$$stats[$i]);
                $item = &getitem('item'.$$stats[$i]);
                # FIXME: Hardcoded
                print OUT  "(${$range}[2]) ($$stats[$i][8][2]) hititem\n";
        }
                
                
        print OUT "endhitbox\n";
}
