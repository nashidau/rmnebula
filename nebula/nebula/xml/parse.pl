#!/usr/bin/perl -w

use strict;
use XML::Parser;
use Dumpvalue;




my $p1 = new XML::Parser(Style => 'Tree');

#$p1->setHandlers(Init    => \&startdoc,
#		 Final   => \&enddoc,
#		 Default => \&unknown,
#		 Start   => \&starttag,
#		 Char    => \&chardata);
		 
my @parse = $p1->parsefile('ragnor.acg');
my $dumper = new Dumpvalue;
print $dumper->dumpValue(@parse);

sub startdoc {
	print "Doc started\n";
}

sub enddoc {
	print "Doc ended\n";
}

sub unknown {
	print "Got an unknown Elemant $_[1]\n";
}

sub starttag {
	print "Got a tag $_[1]\n";
}
sub endtag {
	print "End tag $_[1]\n";
}

sub chardata {
	my ($expat,$string) = @_;
	return if ($string) =~ /^\s*$/m;
	print "Character data $_[1]\n";
}
