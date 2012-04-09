#!/bin/sh

appbin="${0%/*}"
appbase=$(dirname $(dirname $(dirname $appbin)))

#on getPath()
#	set myPath to path to me as string
#	set macPath to (myPath & "Contents:Resources:")
#	set binaryPath to POSIX path of macPath
#end getPath

cd $appbase

cp -f $appbin/../Resources/tbt-typewriter.js $appbase

osascript <<EOF
tell application "Terminal"
	activate
	tell window 1
		do script "$appbin/tbt -r -s html $appbase/TBT-record.html; exit"
		repeat while busy
			delay 1
		end repeat
		close
	end tell
end tell
display dialog "Time Based Text recording saved in $appbase/TBT-record.html" with icon alias ((path to me) & "Contents:Resources:tbt.icns" as string)
EOF


