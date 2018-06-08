#!/usr/bin/perl


#make-appimage.pl: constructs an AppImage using the AppImageTooKit
#
#Usage:
#	$ ./make-appimage.pl version path/to/app path/to/nextfileinbin [path/to/nextfileinbin ...]
#
#Dependencies (must be in $PATH):
#	wget
#	appimagetool-x86_64.AppImage
#	rm, chmod

use File::Spec;
use File::Copy "cp";

$arch = "x86_64";
$version = shift @ARGV;
$program = $ARGV[0];

#get application name, from $program:
($volume,$directory,$file) = File::Spec->splitpath($program);

$rootdir = "$file-$arch.AppDir";

#make directory structure:
mkdir $rootdir;
mkdir "$rootdir/usr";
mkdir "$rootdir/usr/bin";
mkdir "$rootdir/usr/lib";


#copy each arg to bin:
foreach $arg (@ARGV) {
	($vol,$dir,$fil) = File::Spec->splitpath($arg);
	cp $arg, "$rootdir/usr/bin/";
}
chmod 0775, "$rootdir/usr/bin/$file";



#fill the usr/lib directory:

#local excludes, if needed:
#$exclude{'libharfbuzz.so.0'} = "foo";

$result = `wget https://raw.githubusercontent.com/AppImage/AppImages/master/excludelist`;

open INFILE, "excludelist";
while ($line = <INFILE>) {
	chomp $line;
	@file = split /\#/, $line;
	if ($file[0] ne "") {
		$exclude{$file[0]} = "foo";
	}
}
close INFILE; 

$result = `rm excludelist`;

print "\n";

@lines = `ldd $program`;

foreach $line (@lines) {
	@items = split /\s/, $line;
	
	if (exists $exclude{$items[1]}) {
		print "Excluded: $items[1]\n";
	}
	else {
		if ($items[3] ne "") {
			cp $items[3], "$rootdir/usr/lib/";
		}
	} 
}

$APPRUN = <<'END_APPRUN';
#!/bin/sh
HERE="$(dirname "$(readlink -f "${0}")")"
export PATH="${HERE}"/usr/bin/:"${HERE}"/usr/sbin/:"${HERE}"/usr/games/:"${HERE}"/bin/:"${HERE}"/sbin/:"${PATH}"
export LD_LIBRARY_PATH="${HERE}"/usr/lib/:"${HERE}"/usr/lib/i386-linux-gnu/:"${HERE}"/usr/lib/x86_64-linux-gnu/:"${HERE}"/usr/lib32/:"${HERE}"/usr/lib64/:"${HERE}"/lib/:"${HERE}"/lib/i386-linux-gnu/:"${HERE}"/lib/x86_64-linux-gnu/:"${HERE}"/lib32/:"${HERE}"/lib64/:"${LD_LIBRARY_PATH}"
export PYTHONPATH="${HERE}"/usr/share/pyshared/:"${PYTHONPATH}"
export XDG_DATA_DIRS="${HERE}"/usr/share/:"${XDG_DATA_DIRS}"
export PERLLIB="${HERE}"/usr/share/perl5/:"${HERE}"/usr/lib/perl5/:"${PERLLIB}"
export GSETTINGS_SCHEMA_DIR="${HERE}"/usr/share/glib-2.0/schemas/:"${GSETTINGS_SCHEMA_DIR}"
export QT_PLUGIN_PATH="${HERE}"/usr/lib/qt4/plugins/:"${HERE}"/usr/lib/i386-linux-gnu/qt4/plugins/:"${HERE}"/usr/lib/x86_64-linux-gnu/qt4/plugins/:"${HERE}"/usr/lib32/qt4/plugins/:"${HERE}"/usr/lib64/qt4/plugins/:"${HERE}"/usr/lib/qt5/plugins/:"${HERE}"/usr/lib/i386-linux-gnu/qt5/plugins/:"${HERE}"/usr/lib/x86_64-linux-gnu/qt5/plugins/:"${HERE}"/usr/lib32/qt5/plugins/:"${HERE}"/usr/lib64/qt5/plugins/:"${QT_PLUGIN_PATH}"
EXEC=$(grep -e '^Exec=.*' "${HERE}"/*.desktop | head -n 1 | cut -d "=" -f 2 | cut -d " " -f 1)
exec "${EXEC}" "$@"
END_APPRUN

open OUTFILE, ">$file-$arch.AppDir/AppRun";
print OUTFILE $APPRUN;
close OUTFILE;

chmod 0775, "$file-$arch.AppDir/AppRun";

$DESKTOP = <<"END_DESKTOP";
[Desktop Entry]
Name=$file
Exec=$file
Icon=$file
Type=Application
Categories=Graphics;
END_DESKTOP

open OUTFILE, ">$file-$arch.AppDir/$file.desktop";
print OUTFILE $DESKTOP;
close OUTFILE;


$ICON = '<svg xmlns="http://www.w3.org/2000/svg"/>';
open OUTFILE, ">$file-$arch.AppDir/$file.svg";
print OUTFILE $ICON;
close OUTFILE;


$result = `appimagetool-x86_64.AppImage $rootdir $file$version-$arch.AppImage`;
$result = `rm -rf $rootdir`;


