#!/usr/bin/perl


$toc = "";

if ($ARGV[0] eq "") {
	$path = ".";
}
else {
	$path = $ARGV[0];
} 

open (INFILE, "<", "$path/contents.hhc");

while ($line = <INFILE>) {
	$toc .= $line if $line =~ /UL/;
	if ($line =~ /LI/) {
		next if $line =~ /DOCTYPE/;
		chomp $line;
		$line =~ s/<OBJECT.+?>//;
		$toc .= $line;
		while ($line !~ /<\/OBJECT>/) {
			#$line =~ s/name=//;
			#$line =~ s/value=//;
			$line =~ s/<//;
			$line =~ s/>//;
			$line =~ s/"//g;
			@tokens = split "=", $line;
			if ($line =~ /Name/)  {$title = $tokens[2];}
			if ($line =~ /Local/) {$url = $tokens[2];}
			$line = <INFILE>;
		}
		$toc .= "<a href=$url target=\"content\">$title</a>\n";
	}



}


$indexhtml = <<"INDEX";
<html>
<head>
<title>rawproc</title>
<style>
body {
        font-family: sans-serif;
        font-size: small;
}
iframe {    
 border: 0;
}
</style>
</head>
<body>
<h1>rawproc</h1>
<hr>
<div style="width: 30%; float: left;">
$toc
</div>
<div style="width: 70%; height: 90%; float: right;">
<iframe style="width: 100%; height: 100%" name=content src=main.html></iframe>
</body>
</html>
INDEX

print $indexhtml;
