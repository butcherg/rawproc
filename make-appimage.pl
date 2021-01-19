#!/usr/bin/perl


#make-appimage.pl: constructs an AppImage using the AppImageTooKit
#
#Usage:
#	$ ./make-appimage.pl version BIN:path/to/app [BIN:path/to/nextfileinbin [CONF:path/to/conffile ...]]
#
#Dependencies (must be in $PATH):
#	wget
#	appimagetool-x86_64.AppImage
#	rm, chmod 
#	cp (if appimage is run from a shell) 
#	zenity (if appimage is run from a .desktop or file reference in a GUI
#

use File::Spec;
use File::Copy "cp";

$arch = "x86_64";
$version = shift @ARGV;
my ($dest,$pat) = split ":", $ARGV[0];
$program = $pat;

#get application name, from $program:
($volume,$directory,$file) = File::Spec->splitpath($program);

$rootdir = "$file-$arch.AppDir";

printf "$file...\n";

#make directory structure:
mkdir $rootdir;
mkdir "$rootdir/usr";
mkdir "$rootdir/usr/bin";
mkdir "$rootdir/usr/lib";
mkdir "$rootdir/usr/share";
mkdir "$rootdir/usr/share/metainfo";
mkdir "$rootdir/usr/share/$file";


#copy each arg prepended with BIN: to bin:
printf "BIN:\n";
foreach $arg (@ARGV) {
	my ($dest,$pat) = split ":", $arg;
	next if ($dest ne "BIN");
	printf "\t$pat\n";
	my ($vol,$dir,$fil) = File::Spec->splitpath($pat);
	if (-d $pat) {
		mkdir "$rootdir/usr/bin/$fil";
		for my $dirfile (glob "$pat/*") {
			cp $dirfile, "$rootdir/usr/bin/$fil/";
		}
	}
	else {
		cp  $pat, "$rootdir/usr/bin/";
	}
}

#copy each arg prepended with DATA: to share/$program/:
printf "DATA:\n";
foreach $arg (@ARGV) {
	my ($dest,$pat) = split ":", $arg;
	next if ($dest ne "DATA");
	printf "\t$pat\n";
	my ($vol,$dir,$fil) = File::Spec->splitpath($pat);
	if (-d $pat) {
		mkdir "$rootdir/usr/share/$file/$fil";
		for my $dirfile (glob "$pat/*") {
			cp $dirfile, "$rootdir/usr/share/$file/$fil/";
		}
	}
	else {
		cp  $pat, "$rootdir/usr/share/$file/";
	}
}


#set executable permissions for each application in $rootdir/usr/bin/
foreach $fname (<$rootdir/usr/bin/*>) {
	$result=`file $fname |grep -c \"ELF\"`;
	chomp $result;

	if ($result ne "0") {
		chmod 0775, $fname;
	}
}




#fill the usr/lib directory:

#local excludes, if needed:
#$exclude{'libharfbuzz.so.0'} = "foo";

#$result = `wget https://raw.githubusercontent.com/AppImage/AppImages/master/excludelist`;
$result = `wget https://raw.githubusercontent.com/AppImage/pkg2appimage/master/excludelist`;

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

#local 'includes', remove from excludelist:
delete $exclude{'libglib-2.0.so.0'};  #accommodate older rev in some Debian systems, thanks @jade_nl

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

#Establish the unpacked location of the appimage:
HERE="$(dirname "$(readlink -f "${0}")")"

#For first-time use, offer the opportunity to install configuration/data files. Uses host commands, so keep this
#before the path modifications:
if [ ! -d "${HOME}/.rawproc" ]
then
	if [ -t 0 ] #run from a shell
	then
		if command -v whiptail &> /dev/null
		then
			CMD=whiptail
		elif command -v dialog &> /dev/null
		then
			CMD=dialog
		fi
		
		if [ "x$CMD" != "x" ]  #either dialog or whiptail, same syntax for both in this case...
		then
			${CMD} --title "Create confguration/data directory" --backtitle "rawproc AppImage" --yesno "${HOME}/.rawproc doesn't exist.  Shall I create and populate it (y/n)?" 7 60
			response=$?
			clear
			case $response in
				0)
					cp -r ${HERE}/usr/share/rawproc ${HOME}/.rawproc
					${CMD} --title "Create confguration/data directory" --msgbox "${HOME}/.rawproc created and populated." 7 60
					echo "${HOME}/.rawproc populated."
					;;
				1)
					${CMD} --title "Create confguration/data directory" --msgbox "${HOME}/.rawproc NOT created" 7 60
					;;
			esac
			clear
		else
			echo -n "${HOME}/.rawproc doesn't exist.  Shall I create and populate it (y/n)?"
			read answer
			if [ "$answer" != "${answer#[Yy]}" ]
			then
				cp -r ${HERE}/usr/share/rawproc ${HOME}/.rawproc
				echo "${HOME}/.rawproc populated."
			fi
		fi
	elif [ "x$DISPLAY" != "x" ] #running in a GUI... ?
	then
		if command -v zenity &> /dev/null
		then
			zenity --question --text="${HOME}/.rawproc doesn't exist.  Shall I create and populate it?" --no-wrap
			case $? in
				0)
					#cp -r ${HERE}/usr/share/rawproc ${HOME}/.rawproc
					zenity --info --text="${HOME}/.rawproc created and populated." --no-wrap
					;;
				1)
					zenity --info --text="${HOME}/.rawproc NOT created" --no-wrap
					;;
			esac
		fi
	fi
fi

#Alter the paths to first point to the appimage bin and lib:
export PATH="${HERE}"/usr/bin/:"${HERE}"/usr/sbin/:"${HERE}"/usr/games/:"${HERE}"/bin/:"${HERE}"/sbin/:"${PATH}"
export LD_LIBRARY_PATH="${HERE}"/usr/lib/:"${HERE}"/usr/lib/i386-linux-gnu/:"${HERE}"/usr/lib/x86_64-linux-gnu/:"${HERE}"/usr/lib32/:"${HERE}"/usr/lib64/:"${HERE}"/lib/:"${HERE}"/lib/i386-linux-gnu/:"${HERE}"/lib/x86_64-linux-gnu/:"${HERE}"/lib32/:"${HERE}"/lib64/:"${LD_LIBRARY_PATH}"

#Leftover path alterations not needed by rawproc att:
#export PYTHONPATH="${HERE}"/usr/share/pyshared/:"${PYTHONPATH}"
#export XDG_DATA_DIRS="${HERE}"/usr/share/:"${XDG_DATA_DIRS}"
#export PERLLIB="${HERE}"/usr/share/perl5/:"${HERE}"/usr/lib/perl5/:"${PERLLIB}"
#export GSETTINGS_SCHEMA_DIR="${HERE}"/usr/share/glib-2.0/schemas/:"${GSETTINGS_SCHEMA_DIR}"
#export QT_PLUGIN_PATH="${HERE}"/usr/lib/qt4/plugins/:"${HERE}"/usr/lib/i386-linux-gnu/qt4/plugins/:"${HERE}"/usr/lib/x86_64-linux-gnu/qt4/plugins/:"${HERE}"/usr/lib32/qt4/plugins/:"${HERE}"/usr/lib64/qt4/plugins/:"${HERE}"/usr/lib/qt5/plugins/:"${HERE}"/usr/lib/i386-linux-gnu/qt5/plugins/:"${HERE}"/usr/lib/x86_64-linux-gnu/qt5/plugins/:"${HERE}"/usr/lib32/qt5/plugins/:"${HERE}"/usr/lib64/qt5/plugins/:"${QT_PLUGIN_PATH}"

#Original execs, replaced by the following...
#EXEC=$(grep -e '^Exec=.*' "${HERE}"/*.desktop | head -n 1 | cut -d "=" -f 2 | cut -d " " -f 1)
#exec "${EXEC}" "$@"

#exec logic to allow symbolic linking of rawproc, img, and wxcmd names to the appimage.  Default if no link is rawproc.
if [ ! -z $APPIMAGE ] ; then
	BINARY_NAME=$(basename "$ARGV0")
	if [ -e "$HERE/usr/bin/$BINARY_NAME" ] ; then
		exec "$HERE/usr/bin/$BINARY_NAME" "$@"
	else
		exec "$HERE/usr/bin/rawproc" "$@"
	fi
else
	exec "$HERE/usr/bin/rawproc" "$@"
fi
END_APPRUN

open OUTFILE, ">$file-$arch.AppDir/AppRun";
print OUTFILE $APPRUN;
close OUTFILE;

chmod 0775, "$file-$arch.AppDir/AppRun";

$DESKTOP = <<"END_DESKTOP";
[Desktop Entry]
Name=$file
Exec=$file
Icon=icon
Type=Application
Categories=Graphics;
END_DESKTOP

open OUTFILE, ">$file-$arch.AppDir/$file.desktop";
print OUTFILE $DESKTOP;
close OUTFILE;

#hack move files from bin to their appropriate places, replace with META: and ICON: later...
cp "$file-$arch.AppDir/usr/bin/icon.xpm", "$file-$arch.AppDir/.";
cp "$file-$arch.AppDir/usr/bin/rawproc.appdata.xml", "$file-$arch.AppDir/usr/share/metainfo/.";

$result = `appimagetool-x86_64.AppImage --no-appstream $rootdir $file-$version-$arch.AppImage`;
$result = `rm -rf $rootdir`;


