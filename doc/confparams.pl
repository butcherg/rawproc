#!/usr/bin/perl

print "<html><head><title>Configuration Parameters</title></head><body><h3>Configuration Parameters</h3><ul>\n";

my @conflines;
my @files = <$ARGV[0]/*.cpp>;
foreach $file (@files) {
	@lines = `grep \"//parm\" $file`;
	foreach $line (@lines) {
		chomp $line;
		$line =~ s/\r//;
		$line =~ s/^\s+\/\/parm //;
		push @conflines, $line;
	}
}

@conf = sort @conflines;
foreach $line (@conf) {
	#print "<li>$line</li><br>\n";
	my ($parm,$stuff) = split /:/, $line;
	print "<li><b>$parm</b>: $stuff</li><br>\n";
}

print "</ul>\n";

$outputparams = <<OUTPUTPARAMS;
<h3>Output Parameters</h3>
<p>For the output.*.parameters properties, the following are available.  Specify them
as name=value pairs, separated by semicolons.</p>
<ul>
OUTPUTPARAMS

my @parmlines;
foreach $file (@files) {
	@lines = `grep -F \"//\$\" $file`;
	foreach $line (@lines) {
		chomp $line;
		$line =~ s/\r//;
		$line =~ s/^\s+\/\/\$ //;
		push @parmlines, $line;
	}
}
print $outputparams;
print @parmlines;

print "</ul></body></html>\n";
