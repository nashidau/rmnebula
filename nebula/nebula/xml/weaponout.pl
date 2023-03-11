#!/usr/bin/perl -w

while(<>){
	chomp;
	print <<EOS
<!ELEMENT $_  %turnattrs; >
<!ATTLIST $_  %weaponattrs; >

EOS
;
	}
