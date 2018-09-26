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
			$string =~ s/","/=/;
			$string =~ s/"//g;
			my ($name, $value) = split "=", $string;
			$properties{$name} = $value;
		}
	}
}

foreach $name (sort keys %properties) {
	print "$name=$properties{$name}\n";
}

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

