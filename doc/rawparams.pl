#!/usr/bin/perl

print "<html><head><title>Raw Parameters</title><link rel=\"stylesheet\" type=\"text/css\" href=\"rawprocdoc.css\"></head><body><h3>Raw Parameters</h3>\n";

$frontpiece = <<FRONT;
<p>Use these parameters to tell LibRaw how to process the input raw image. In the input.raw.parameters configuration parameter, build a string of the ones you want to use, separated by semicolons, e.g., "gamma=prophoto;demosaic=prophoto".</p> 
<p>Alternatively (preferred), each libraw dcraw parameter can be specified in individual input.raw.libraw.* parameters, where "*" is the parameter name. You can add a input.raw.libraw parameter to the properties and not specifiy a value; if you do that, it won't be included in the constructed parameter list. This lets you enter placeholder parameters to use when needed. </p>
<p>Each of the LibRaw imgdata.params struct fields has a corresponding raw parameter, and there are selected parameters that have more intuitive abstractions. </p>
<p>in an img command line, you can specify any of these as a parameter to the raw image file name, e.g.,
<pre>
\timg DSC_0001.NEF:cameraprofile=NikonD850.icc,demosaic=ahd,gamma=linear ...
</pre>
<p>Each parameter with a dcraw switch is annotated thusly: 'dcraw: -x'. </p>
FRONT

print "$frontpiece\n<hr><ul>\n";

my @conflines;
my $file = $ARGV[0];
my @lines = `grep \"//raw\" $file`;
foreach $line (@lines) {
	chomp $line;
	$line =~ s/\r//;
	$line =~ s/^\s+\/\/raw //;
	push @conflines, $line;
}


@conf = sort @conflines;
print @conf;
#foreach $line (@conf) {
#	print "$line\n";
#}

print "</ul></body></html>\n";

