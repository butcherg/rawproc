#!/bin/sh

FILENAME=$1
SCRIPTNAME=$2
INNO_BIN="Inno Setup 6/ISCC.exe"

# Check if variable is set
[ -z "$SCRIPTNAME" ] && { echo "Usage: $0 <SCRIPT_NAME>"; echo; exit 1; }

# Check if filename exist
[ ! -f "$SCRIPTNAME" ] && { echo "File not found. Aborting."; echo $SCRIPTNAME; echo; exit 1; }

# Check if wine is present
command -v wine >/dev/null 2>&1 || { echo >&2 "I require wine but it's not installed. Aborting."; echo; exit 1; }

# Get Program Files path via wine command prompt
PROGRAMFILES=$(wine cmd /c 'echo %PROGRAMFILES%' 2>/dev/null)

# Translate windows path to absolute unix path
PROGFILES_PATH=$(winepath -u "${PROGRAMFILES}" 2>/dev/null)

# Get inno setup path
INNO_PATH="${PROGFILES_PATH%?}/${INNO_BIN}"

# Translate unix script path to windows path 
SCRIPTNAME=$(winepath -w "$SCRIPTNAME" 2> /dev/null)

# Check if Inno Setup is installed into wine
[ ! -f "$INNO_PATH" ] && { echo "Install Inno Setup 6 Quickstart before running this script."; echo; exit 1; }

# Compile!
#wine "$INNO_PATH" /F"$FILENAME" "$SCRIPTNAME"
wine "$INNO_PATH" "$SCRIPTNAME"

