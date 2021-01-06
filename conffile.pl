#!/usr/bin/perl

my %properties;
my @files = <$ARGV[0]/*.cpp>;
foreach $file (@files) {
	@lines = `grep -F \"myConfig::getConfig().getValueOrDefault(\" $file`;
	foreach $line (@lines) {
		chomp $line;
		$line =~ s/\r//;
		my @strings = $line =~ /myConfig::getConfig\(\).getValueOrDefault\(.+?\)/g;
		foreach $string (@strings) {
			$string =~ s/myConfig::getConfig\(\).getValueOrDefault\(//;
			$string =~ s/\)//;
			$string =~ s/,/=/;
			my ($name, $value) = split "=", $string, 2;
			$name   =~ s/"//g;
			if (index($value,"\"") != -1) {
				$value =~ s/"//g;
			}
			else {
				$value = "";
			} 
			next if $name eq "make"; 
			next if $name eq "makemodel";
			$properties{$name} = $value;
		}
	}
}

foreach $name (sort keys %properties) {
	print "$name=$properties{$name}\n";
}

#12-22-2020: crop is the new default per the delivered rawproc.conf, new for rawproc 1.0:
print "input.raw.libraw.rawdata=crop\n";

print "\n\n[Templates]\n";

foreach $file (@files) {
	@lines = `grep -F \"//template" $file`;
	foreach $line (@lines) {
		chomp $line;
		$line =~ s/\r//;
		$line =~ s/^.+?\/\/template //;
		
		print "$line\n";
	}
}

